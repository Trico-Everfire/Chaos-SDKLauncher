//
// Created by trico on 14-12-22.
//

#include "modmanager.h"

#ifdef _WIN32
#define OS_BIN_SUFFIX ".bat"
#else
#define OS_BIN_SUFFIX ".sh"
#endif

#include "editconfig.h"
#include "filedownloader.h"
#include "mz_compat.h"
#include "zip_handler.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>

CModManager::CModManager( CMainView *parent ) :
	QDialog( parent )
{
	setWindowTitle( "New P2:CE Source Mod" );

	auto pManagerGridLayout = new QGridLayout( this );
	auto pModNameLabel = new QLabel( this );
	pModNameLabel->setText( "Mod Name:" );
	pManagerGridLayout->addWidget( pModNameLabel, 0, 0 );
	m_pModNameLineEdit = new QLineEdit( this );
	pManagerGridLayout->addWidget( m_pModNameLineEdit, 0, 1, 1, 3 );

	auto pModPathLabel = new QLabel( this );
	pModPathLabel->setText( "Mod Path:" );
	pManagerGridLayout->addWidget( pModPathLabel, 1, 0 );
	auto pDirectoryPushButton = new QPushButton( this );
	pDirectoryPushButton->setIcon( QIcon( ":/resource/edit.png" ) );
	pDirectoryPushButton->setFixedWidth( pDirectoryPushButton->height() );
	pManagerGridLayout->addWidget( pDirectoryPushButton, 1, 1 );
	m_pModPathLineEdit = new QLineEdit( this );
	m_pModPathLineEdit->setText( parent->m_pInstallDir );
	m_pModPathLineEdit->setDisabled( true );
	pManagerGridLayout->addWidget( m_pModPathLineEdit, 1, 2 );
	auto pCreateCancelButtonBox = new QDialogButtonBox( this );
	m_pCreateButton = pCreateCancelButtonBox->addButton( "Create", QDialogButtonBox::AcceptRole );
	m_pCreateButton->setDisabled( true );
	m_pCreateButton->setToolTip( "Mods MUST have a name." );
	pCreateCancelButtonBox->addButton( "Cancel", QDialogButtonBox::RejectRole );
	connect( pDirectoryPushButton, &QPushButton::pressed, [this]
			 {
				 QString directoryResult = QFileDialog::getExistingDirectory( this );
				 if ( !directoryResult.isEmpty() )
					 m_pModPathLineEdit->setText( directoryResult + "/" );
			 } );
	connect( pCreateCancelButtonBox, &QDialogButtonBox::clicked, this, [this]( QAbstractButton *pCreateOrCancelButton )
			 {
				 if ( pCreateOrCancelButton->text() == "Create" )
					 this->modCreationHandler();
				 else
					 this->close();
			 } );
	connect( m_pModNameLineEdit, &QLineEdit::textChanged, this, [this]( const QString &text )
			 {
				 bool isNotEmpty = text.count( " " ) != text.count();
				 m_pCreateButton->setEnabled( isNotEmpty );
				 m_pCreateButton->setToolTip( isNotEmpty ? "" : "Mods MUST have a name." );
			 } );
	connect( m_pModPathLineEdit, &QLineEdit::textChanged, this, [this, pDirectoryPushButton]( const QString &text )
			 {
				 QFileInfo currentSelectedDirectory = QFileInfo( text );
				 bool fileExistsAndWritable = currentSelectedDirectory.exists() && currentSelectedDirectory.isDir() && currentSelectedDirectory.isWritable();
				 m_pCreateButton->setEnabled( fileExistsAndWritable );
				 m_pCreateButton->setToolTip( fileExistsAndWritable ? "" : "The folder MUST exist and application must have write access." );
				 pDirectoryPushButton->setIcon( QIcon( fileExistsAndWritable ? ":/resource/edit.png" : ":/resource/edit_error.png" ) );
			 } );
	pManagerGridLayout->addWidget( pCreateCancelButtonBox, 2, 0, 1, 3, Qt::AlignLeft );
}

bool CModManager::modZipHandler( const QFileInfo &zFile, QSplashScreen *sps, const QString &modName, const QString &modPath )
{
	unzFile uzfile;
	char *zfilename;
	unsigned char *buf;
	size_t buflen;
	zipper_result_t zipper_ret;
	uint64_t len;

	uzfile = unzOpen64( zFile.absoluteFilePath().toStdString().c_str() );
	if ( uzfile == nullptr )
	{
		printf( "Could not open %s for unzipping\n", zFile.fileName().toStdString().c_str() );
		return false;
	}
	QString firstDir = QString();
	do
	{
		zipper_ret = ZIPPER_RESULT_SUCCESS;
		zfilename = zipper_filename( uzfile, nullptr );
		if ( zfilename == nullptr )
			return true;

		if ( zipper_isdir( uzfile ) )
		{
			if ( firstDir.isEmpty() )
				firstDir.append( zfilename );
			QString newDir = QString( zfilename ).replace( firstDir, modName + "/" );
			QDir( modPath ).mkpath( newDir );
			sps->showMessage( QString( "Creating dir: " ) + newDir );
			unzGoToNextFile( uzfile );
			free( zfilename );
			continue;
		}

		QString fixedFilePath = QString( zfilename ).replace( firstDir, modName + "/" );

		len = zipper_filesize( uzfile );
		sps->showMessage( QString( "reading file (" ) + QString::number( len ) + " bytes): " + fixedFilePath );
		zipper_ret = zipper_read_buf( uzfile, &buf, &buflen );
		if ( zipper_ret == ZIPPER_RESULT_ERROR )
		{
			free( zfilename );
			break;
		}

		QFile file = QFile( modPath + fixedFilePath );
		if ( !file.open( QFile::WriteOnly ) )
		{
			free( zfilename );
			return false;
		}
		file.write( reinterpret_cast<const char *>( buf ), buflen );
		file.close();
		free( buf );
		free( zfilename );
	} while ( zipper_ret == ZIPPER_RESULT_SUCCESS );

	if ( zipper_ret == ZIPPER_RESULT_ERROR )
	{
		printf( "failed to read file\n" );
		return false;
	}

	unzClose( uzfile );
	return true;
}

void CModManager::modCreationHandler()
{
	const QString modName = m_pModNameLineEdit->text();
	const QString modPath = m_pModPathLineEdit->text();

	const QFileInfo modPathInfo = QFileInfo( modPath );
	const QFileInfo modPathFullInfo = QFileInfo( modPath + modName );

	if ( modPathFullInfo.exists() )
	{
		QMessageBox::warning( this, "Already Exists!", "This folder already exists!" );
		return;
	}

	if ( !modPathInfo.isWritable() )
	{
		QMessageBox::critical( this, "ACCESS ERROR!!", "Cannot write to " + modPathInfo.absoluteFilePath() );
		return;
	}

	if( !QFileInfo( QDir::tempPath() ).isWritable() ){
		QMessageBox::critical( this, "ACCESS ERROR!!", "Cannot write to " + QDir::tempPath() );
		return;
	}
	auto pTemporaryFile = new QFile( QDir::tempPath() + "/p2ce-mod-template.zip", this );
	QPixmap splashPixMap( ":/resource/splash.png" );
	auto *pDownloadSplashScreen = new QSplashScreen( splashPixMap );
	pDownloadSplashScreen->setParent( this );
	pDownloadSplashScreen->show();

	auto pFileDownloader = new CFileDownloader( QUrl( "https://codeload.github.com/ChaosInitiative/p2ce-mod-template/zip/refs/heads/main" ), this );

	connect( pFileDownloader, &CFileDownloader::downloaded, this, [pFileDownloader, pTemporaryFile, pDownloadSplashScreen]
			 {
				 pDownloadSplashScreen->showMessage( "Downloading latest ZIP from https://github.com/ChaosInitiative/p2ce-mod-template" );
				 pTemporaryFile->open( QFile::ReadWrite );
				 pTemporaryFile->write( pFileDownloader->downloadedData() );
				 pTemporaryFile->close();
			 } );

	connect( pTemporaryFile, &QFile::aboutToClose, [pDownloadSplashScreen, this]
			 {
				 qInfo() << "UNZIPPING!";
				 pDownloadSplashScreen->showMessage( "ZUP Downloaded, Extracting..." );
				 if(!modZipHandler( QFileInfo( QDir::tempPath() + "/p2ce-mod-template.zip" ), pDownloadSplashScreen, m_pModNameLineEdit->text(), m_pModPathLineEdit->text() )){
					 QMessageBox::critical( this, "UNZIP ERROR", "Cannot unzip mod template." );
					 pDownloadSplashScreen->close();
					 return;
				 }
				 pDownloadSplashScreen->showMessage( "Extraction complete. Closing process." );
				 QFile::remove( QDir::tempPath() + "/p2ce-mod-template.zip" );
				 pDownloadSplashScreen->close();
				 this->hide();
				 auto mainParent = dynamic_cast<CMainView *>( this->parentWidget() );
				 auto editCFG = new CEditConfig( mainParent );

				 auto pCurrentItem = editCFG->m_pEditList->currentItem();
				 auto pEditConfigPopup = new CEditConfigPopup( editCFG );
				 const QString modName = m_pModNameLineEdit->text();
				 const QString modPath = m_pModPathLineEdit->text();
				 pEditConfigPopup->setWindowTitle( modName + " Config Entry" );
				 pEditConfigPopup->m_pTypeComboBox->setCurrentIndex( 0 );
				 pEditConfigPopup->m_pNameLineEdit->setText( modName );
				 pEditConfigPopup->m_pUrlLineEdit->setText( mainParent->m_pInstallDir + "p2ce" + OS_BIN_SUFFIX );
				 pEditConfigPopup->m_pArgumentsListTextEdit->append( "-game" );
				 pEditConfigPopup->m_pArgumentsListTextEdit->append( "'" + modPath + modName + "'" );
				 pEditConfigPopup->exec();
				 if ( !pEditConfigPopup->shouldApplyChanges() )
					 return;

				 // We then apply the visuals of the item.
				 // depending on the type, it'll receive JSON data.
				 // it's either only the name and type,
				 // name, type, icon and url, or name, type, url, icon and arguments.
				 auto pNewListItem = new QListWidgetItem();
				 pNewListItem->setText( pEditConfigPopup->m_pNameLineEdit->text() );
				 pNewListItem->setIcon( QIcon() );
				 if ( QFileInfo::exists( pEditConfigPopup->m_pIconPathLineEdit->text() ) )
					 pNewListItem->setIcon( QIcon( pEditConfigPopup->m_pIconPathLineEdit->text() ) );

				 auto listItemJSONContents = QJsonObject();
				 listItemJSONContents["name"] = pEditConfigPopup->m_pNameLineEdit->text();
				 if ( pEditConfigPopup->m_pTypeComboBox->currentIndex() == 2 )
				 {
					 // Categories are highlighted in bold to distinguish
					 // them from buttons.
					 // Provided they need the least care,
					 // We do all what's needed and return here to
					 // make things easier.
					 auto listItemFont = pNewListItem->font();
					 listItemFont.setBold( true );
					 pNewListItem->setFont( listItemFont );
					 listItemJSONContents["urlType"] = "category";
					 pNewListItem->setData( Qt::UserRole, listItemJSONContents );
					 editCFG->m_pEditList->currentRowChanged( 0 );
					 int currentRow = editCFG->m_pEditList->row( pCurrentItem );
					 editCFG->m_pEditList->insertItem( currentRow + 1, pNewListItem );
					 return;
				 }
				 listItemJSONContents["url"] = pEditConfigPopup->m_pUrlLineEdit->text();
				 listItemJSONContents["icon"] = pEditConfigPopup->m_pIconPathLineEdit->text();
				 listItemJSONContents["urlType"] = "url";
				 if ( pEditConfigPopup->m_pTypeComboBox->currentIndex() == 0 )
				 {
					 // We need to convert the arguments from a
					 // QString to a QStringList.
					 auto jsonArgumentList = QJsonArray();
					 foreach( auto listArgument, pEditConfigPopup->m_pArgumentsListTextEdit->toPlainText().split( " " ) )
					 {
						 jsonArgumentList.append( listArgument );
					 }
					 listItemJSONContents["urlType"] = "process";
					 listItemJSONContents["args"] = jsonArgumentList;
				 }
				 // We then store the JSON data and call the list's row changed function.
				 pNewListItem->setData( Qt::UserRole, listItemJSONContents );
				 editCFG->m_pEditList->currentRowChanged( 0 );

				 int currentRow = pCurrentItem ? editCFG->m_pEditList->row( pCurrentItem ) : 0;
				 editCFG->m_pEditList->insertItem( currentRow + 1, pNewListItem );

				 editCFG->m_pApplyButton->click();
				 delete editCFG;

				 // We alter the gameinfo.txt to reflect the mod's name.
				 auto gameInfoFile = QFile( modPath + modName + "/gameinfo.txt" );
				 // We need this particular read then write, ReadWrite doesn't appear to work.
				 if ( gameInfoFile.open( QFile::ReadOnly ) )
				 {
					 QString gameInfoFileContent = QString( gameInfoFile.readAll() );
					 gameInfoFileContent = gameInfoFileContent.replace( "\"Portal 2: My Cool Template Mod\"", "\"" + modName + "\"" );
					 gameInfoFile.close();
					 if ( gameInfoFile.open( QFile::WriteOnly ) )
					 {
						 gameInfoFile.write( QByteArray( gameInfoFileContent.toStdString().c_str() ) );
						 gameInfoFile.close();
					 }
				 }

				 QMessageBox::information( this, "SUCCESS!", "Source mod successfully created!" );
				 this->close();
			 } );
}
