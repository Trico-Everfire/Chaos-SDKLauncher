#include "mainview.h"

#include "FilesystemSearchProvider.h"
#include "config.h"
#include "editconfig.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

using namespace ui;

CMainView::CMainView( QWidget *pParent ) :
	QDialog( pParent )
{
	// We call the SAPP library to get the path of the game.
	// Which in our case is detemined by the env variable SteamAppId.
	CFileSystemSearchProvider provider;
	char *installDir = new char[1048];
	provider.GetAppInstallDir( qEnvironmentVariableIntValue( "SteamAppId" ), installDir, 1048 );
	m_pInstallDir = QString( installDir );
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
	pSDKLayout->addWidget( m_pListWidget, 0, 0 );

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
				auto argumentVariantList = contentObject["argumentStringList"].toArray().toVariantList();
				QStringList argumentStringList;
				foreach( QVariant vArgumentItem, argumentVariantList )
				{
					argumentStringList << vArgumentItem.toString();
				}

				// We then determine on if the request is a URL or process.
				// Then execute it accordingly.
				if ( contentObject["urlType"].toString() == "url" )
					OpenUrl( contentObject["url"].toString().replace( "${INSTALLDIR}", m_pInstallDir ) );
				else if ( contentObject["urlType"].toString() == "process" )
					OpenProcess( contentObject["url"].toString().replace( "${INSTALLDIR}", m_pInstallDir ), argumentStringList.replaceInStrings( "${INSTALLDIR}", m_pInstallDir ) );
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

	connect( pEditButton, &QPushButton::pressed, this, onEditConfigButtonPressedCallback );

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
	pProcess->start( execName, params );
}
