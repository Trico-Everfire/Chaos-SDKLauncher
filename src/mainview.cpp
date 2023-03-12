#include "mainview.h"

#include "FilesystemSearchProvider.h"
#include "config.h"
#include "editconfig.h"
#include "modmanager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>

using namespace ui;

CMainView::CMainView( QWidget *pParent ) :
	QDialog( pParent )
{
	int gameAppID = qEnvironmentVariableIntValue( "SteamAppId" );
	
	
	CFileSystemSearchProvider provider;
	if ( !provider.Available() || !provider.BIsAppInstalled( gameAppID ) )
	{
		QMessageBox::critical( this, "Missing Game!", "P2CE is not installed on this machine or is otherwise unavailable." );
		exit( EXIT_FAILURE );
	}

	char installDir[1048];
	provider.GetAppInstallDir( gameAppID, installDir, 1048 );
	m_InstallDir = QString( installDir );

	
	
	auto pSDKLayout = new QGridLayout( this );
	pSDKLayout->setObjectName( "SDKLayout" );

	
	
	
	
	m_pListWidget = new QListWidget( this );
	auto pSDKListWidgetLayout = new QVBoxLayout( m_pListWidget );
	pSDKListWidgetLayout->setAlignment( Qt::AlignTop );
	pSDKListWidgetLayout->setObjectName( "SDKItemLayout" );
	pSDKLayout->addWidget( m_pListWidget, 0, 0, 2, 1 );

	
	
	
	
	QFile configFile( "./config.json" );
	QJsonDocument JSONConfigDocument;
	if ( !configFile.exists() && configFile.open( QFile::WriteOnly ) )
	{
		
		JSONConfigDocument = defaultConfig();
		configFile.write( JSONConfigDocument.toJson() );
		configFile.close();
	}
	else if ( configFile.open( QFile::ReadOnly ) )
	{
		
		JSONConfigDocument = QJsonDocument::fromJson( configFile.readAll() );
		configFile.close();
	}
	else
	{
		QMessageBox::critical( this, "Config Error", "Couldn't load config file" );
		this->close();
		return;
	}

	
	
	QJsonArray JSONConfig = JSONConfigDocument.array();
	for ( auto it = JSONConfig.begin(); it != JSONConfig.end(); it++ )
	{
		
		
		auto HeaderObject = it->toObject();
		auto pHeader = new QLabel( HeaderObject["header"].toString(), this );
		pHeader->setObjectName( "Header" );
		pSDKListWidgetLayout->addWidget( pHeader );

		
		QJsonArray contentArray = HeaderObject["content"].toArray();

		for ( const auto &jsonContentObject : contentArray )
		{
			
			
			auto contentObject = jsonContentObject.toObject();

			
			
			
			auto *pUrlButton = new QPushButton( this );
			pUrlButton->setIcon( QIcon( contentObject["icon"].toString() ) );
			pUrlButton->setText( contentObject["name"].toString() );
			pUrlButton->setObjectName( "MediaItem" );
			pUrlButton->setProperty( "JSONData", contentObject );
			pSDKListWidgetLayout->addWidget( pUrlButton );

			
			auto onProcessPushButtonPressedCallback = [this, contentObject]()
			{
				
				
				
				auto argumentVariantList = contentObject["args"].toArray().toVariantList();
				QStringList argumentStringList;
				foreach( QVariant vArgumentItem, argumentVariantList )
				{
					argumentStringList << vArgumentItem.toString();
				}

				
				
				if ( contentObject["urlType"].toString() == "url" )
					OpenUrl( contentObject["url"].toString().replace( "${INSTALLDIR}", m_InstallDir ) );
				else if ( contentObject["urlType"].toString() == "process" )
					OpenProcess( contentObject["url"].toString().replace( "${INSTALLDIR}", m_InstallDir ), argumentStringList.replaceInStrings( "${INSTALLDIR}", m_InstallDir ) );
				else
					qDebug() << "Unknown URL Type: " << contentObject["urlType"].toString();
			};

			connect( pUrlButton, &QPushButton::pressed, this, onProcessPushButtonPressedCallback );
		}
	}

	
	
	auto pEditButton = new QPushButton( this );
	pEditButton->setIcon( QIcon( ":/resource/edit.png" ) );
	pSDKLayout->addWidget( pEditButton, 0, 1, Qt::AlignTop );
	auto onEditConfigButtonPressedCallback = [this]
	{
		auto editConfigDialog = new CEditConfig( this );
		editConfigDialog->exec();
	};

	auto pNewModButton = new QPushButton( this );
	pNewModButton->setIcon( QIcon( ":/resource/add.png" ) );
	pSDKLayout->addWidget( pNewModButton, 1, 1, Qt::AlignTop );

	auto onNewModButtonPressedCallback = [this]
	{
		auto modManager = new CModManager( this );
		modManager->exec();
	};

	pSDKLayout->setRowStretch( 1, 1 );

	connect( pEditButton, &QPushButton::pressed, this, onEditConfigButtonPressedCallback );
	connect( pNewModButton, &QPushButton::pressed, this, onNewModButtonPressedCallback );

	m_pListWidget->setFixedSize( pSDKListWidgetLayout->sizeHint() );
	pSDKLayout->setAlignment( Qt::AlignTop );

	
	this->setFocus( Qt::NoFocusReason );
}

void CMainView::OpenUrl( const QString &url )
{
	QDesktopServices::openUrl( QUrl( url ) );
}

void CMainView::OpenProcess( const QString &execName, const QStringList &params )
{
	qInfo() << execName;
	qInfo() << params;
	auto pProcess = new QProcess( this );
	pProcess->start( execName, params );
}
QString CMainView::GetInstallDir()
{
	return m_InstallDir;
}