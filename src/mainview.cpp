#include "mainview.h"

#include "FilesystemSearchProvider.h"
#include "config.h"
#include "editconfig.h"
#include "filedownloader.h"
#include "libs/QConsoleWidget/QConsoleWidget.h"
#include "modmanager.h"
#include "mz_strm.h"
#include "mz_strm_os.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

// #include "zlib.h"
// #define CHUNK 16384
//
//
//
// int inf(FILE *source, FILE *dest)
//{
//	int ret;
//	unsigned have;
//	z_stream strm;
//	unsigned char in[CHUNK];
//	unsigned char out[CHUNK];
//
//	/* allocate inflate state */
//	strm.zalloc = Z_nullptr;
//	strm.zfree = Z_nullptr;
//	strm.opaque = Z_nullptr;
//	strm.avail_in = 0;
//	strm.next_in = Z_nullptr;
//	ret = inflateInit(&strm);
//	if (ret != Z_OK)
//		return ret;
//
//	/* decompress until deflate stream ends or end of file */
//	do {
//		strm.avail_in = fread(in, 1, CHUNK, source);
//		if (ferror(source)) {
//			(void)inflateEnd(&strm);
//			return Z_ERRNO;
//		}
//		if (strm.avail_in == 0)
//			break;
//		strm.next_in = in;
//
//		do
//		{
//			strm.avail_out = CHUNK;
//			strm.next_out = out;
//
//			ret = inflate( &strm, Z_NO_FLUSH );
//			assert( ret != Z_STREAM_ERROR ); /* state not clobbered */
//			switch ( ret )
//			{
//				case Z_NEED_DICT:
//					ret = Z_DATA_ERROR; /* and fall through */
//				case Z_DATA_ERROR:
//				case Z_MEM_ERROR:
//					(void)inflateEnd( &strm );
//					return ret;
//			}
//			have = CHUNK - strm.avail_out;
//			if ( fwrite( out, 1, have, dest ) != have || ferror( dest ) )
//			{
//				(void)inflateEnd( &strm );
//				return Z_ERRNO;
//			}
//		} while (strm.avail_out == 0);
//		/* done when inflate() says it's done */
//	} while (ret != Z_STREAM_END);
//	(void)inflateEnd(&strm);
//	/* clean up and return */
//	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
// }

using namespace ui;

CMainView::CMainView( QWidget *pParent ) :
	QDialog( pParent )
{
	int p2ceAppID = qEnvironmentVariableIntValue( "SteamAppId" );
	// We call the SAPP library to get the path of the game.
	// Which in our case is detemined by the env variable SteamAppId.
	CFileSystemSearchProvider provider;
	if ( !provider.Available() || !provider.BIsAppInstalled( p2ceAppID ) )
	{
		QMessageBox::critical( this, "Missing Game!", "P2CE is not installed on this machine or is otherwise unavailable." );
		exit( EXIT_FAILURE );
	}
	char *installDir = new char[1048];
	provider.GetAppInstallDir( p2ceAppID, installDir, 1048 );
	m_installDir = QString( installDir );
	delete[] installDir;

	// We create a QGridLayout to make the layout between the
	// list widget and the edit button.
	auto pSDKLayout = new QGridLayout( this );
	pSDKLayout->setObjectName( "SDKLayout" );

	// We create a QListWidget to later populate with
	// out configuration file's content.
	// We use a simple QVBoxLayout to store the widget and
	// place this inside the list widget.
	m_pListWidget = new QListWidget( this );
	auto pSDKListWidgetLayout = new QVBoxLayout( m_pListWidget );
	pSDKListWidgetLayout->setAlignment( Qt::AlignTop );
	pSDKListWidgetLayout->setObjectName( "SDKItemLayout" );
	pSDKLayout->addWidget( m_pListWidget, 0, 0, 2, 1 );

	// We then read the configuration file.
	// if we can't find it we create it with a
	// default configuration that's set up in
	// config.h
	QFile configFile( "./config.json" );
	QJsonDocument JSONConfigDocument;
	if ( !configFile.exists() && configFile.open( QFile::WriteOnly ) )
	{
		// If there is a config file, we use that...
		JSONConfigDocument = defaultConfig();
		configFile.write( JSONConfigDocument.toJson() );
		configFile.close();
	}
	else if ( configFile.open( QFile::ReadOnly ) )
	{
		//...otherwise create a new one.
		JSONConfigDocument = QJsonDocument::fromJson( configFile.readAll() );
		configFile.close();
	}
	else
	{
		QMessageBox::critical( this, "Config Error", "Couldn't load config file" );
		this->close();
		return;
	}

	// We now populate our list widget with the configuration's content.
	// This JSON information is handled by QT and is only read here.
	QJsonArray JSONConfig = JSONConfigDocument.array();
	for ( auto it = JSONConfig.begin(); it != JSONConfig.end(); it++ )
	{
		// We first get the main header object from the json config array/
		// then set the first header label and append it to the layout.
		auto HeaderObject = it->toObject();
		auto pHeader = new QLabel( HeaderObject["header"].toString(), this );
		pHeader->setObjectName( "Header" );
		pSDKListWidgetLayout->addWidget( pHeader );

		// We get the content from the main header object.
		QJsonArray contentArray = HeaderObject["content"].toArray();

		for ( const auto &jsonContentObject : contentArray )
		{
			// As we iterate through the content object, we first get
			// the actual content JSON object.
			auto contentObject = jsonContentObject.toObject();

			// We create a button that when clicked starts a process based on
			// the url (referring to both an executable path or web url)
			// and it's parameters.
			auto *pUrlButton = new QPushButton( this );
			pUrlButton->setIcon( QIcon( contentObject["icon"].toString() ) );
			pUrlButton->setText( contentObject["name"].toString() );
			pUrlButton->setObjectName( "MediaItem" );
			pUrlButton->setProperty( "JSONData", contentObject );
			pSDKListWidgetLayout->addWidget( pUrlButton );

			// We create a callback function for handling the URL/Process trigger.
			auto onProcessPushButtonPressedCallback = [&, contentObject]()
			{
				// We need to convert the arguments from a
				// JSON variant list to a QStringList for the
				// process executor to understand.
				auto argumentVariantList = contentObject["args"].toArray().toVariantList();
				QStringList argumentStringList;
				foreach( QVariant vArgumentItem, argumentVariantList )
				{
					argumentStringList << vArgumentItem.toString();
				}

				// We then determine on if the request is a URL or process.
				// Then execute it accordingly.
				if ( contentObject["urlType"].toString() == "url" )
					OpenUrl( contentObject["url"].toString().replace( "${INSTALLDIR}", m_installDir ) );
				else if ( contentObject["urlType"].toString() == "process" )
					OpenProcess( contentObject["url"].toString().replace( "${INSTALLDIR}", m_installDir ), argumentStringList.replaceInStrings( "${INSTALLDIR}", m_installDir ) );
				else
					qDebug() << "Unknown URL Type: " << contentObject["urlType"].toString();
			};

			connect( pUrlButton, &QPushButton::pressed, this, onProcessPushButtonPressedCallback );
		}
	}

	// We create an Edit button to edit the configuration
	// of the application according to the user's preferences.
	auto pEditButton = new QPushButton( this );
	pEditButton->setIcon( QIcon( ":/resource/edit.png" ) );
	pSDKLayout->addWidget( pEditButton, 0, 1, Qt::AlignTop );
	auto onEditConfigButtonPressedCallback = [&]
	{
		auto editConfigDialog = new CEditConfig( this );
		editConfigDialog->exec();
	};

	auto pNewModButton = new QPushButton( this );
	pNewModButton->setIcon( QIcon( ":/resource/add.png" ) );
	pSDKLayout->addWidget( pNewModButton, 1, 1, Qt::AlignTop );

	auto onNewModButtonPressedCallback = [&]
	{
		auto modManager = new CModManager( this );
		modManager->exec();
	};

	pSDKLayout->setRowStretch( 1, 1 );

	connect( pEditButton, &QPushButton::pressed, this, onEditConfigButtonPressedCallback );
	connect( pNewModButton, &QPushButton::pressed, this, onNewModButtonPressedCallback );

	m_pListWidget->setFixedSize( pSDKListWidgetLayout->sizeHint() );
	pSDKLayout->setAlignment( Qt::AlignTop );

	// Set focus so we don't have focus directly on the top most button
	this->setFocus( Qt::NoFocusReason );
}

void CMainView::OpenUrl( const QString &url )
{
	QDesktopServices::openUrl( QUrl( url ) );
}

void CMainView::OpenProcess( const QString &execName, const QStringList &params )
{
	auto pProcess = new QProcess( this );
	pProcess->setReadChannel( QProcess::StandardOutput );
	pProcess->setProcessChannelMode( QProcess::MergedChannels );
	pProcess->setCurrentReadChannel( QProcess::StandardOutput );
	pProcess->start( execName, params );

	QConsoleWidget *pConsoleWidget = new QConsoleWidget();
	pConsoleWidget->device()->open( QIODevice::WriteOnly );
	pConsoleWidget->show();

	QTextStream *pTextStream = new QTextStream( pConsoleWidget->device() );

	connect( pProcess, &QProcess::readyReadStandardOutput, pConsoleWidget, [pProcess, pTextStream]()
			 {
				 *pTextStream << ( pProcess->readAllStandardOutput() ) << Qt::endl;
			 } );
	connect( pProcess, &QProcess::finished, pProcess, [pProcess, pTextStream, pConsoleWidget]( int exitCode, QProcess::ExitStatus exitStatus )
			 {
				 delete pProcess;
				 delete pTextStream;
			 } );

	pConsoleWidget->setAttribute( Qt::WA_DeleteOnClose );
}
