#include "mainview.h"

#include "FilesystemSearchProvider.h"
#include "config.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QScrollArea>

using namespace ui;

CMainView::CMainView( QWidget *pParent ) :
	QDialog( pParent )
{
	CFileSystemSearchProvider provider;
	char *installDir = new char[1048];
	provider.GetAppInstallDir( qgetenv( "SteamAppId" ).toInt(), installDir, 1048 );
	m_pInstallDir = QString( installDir );
	delete[] installDir;

	auto pLayout = new QGridLayout( this );
	pLayout->setObjectName( "SDKLayout" );

	auto scrollArea = new QScrollArea();
	scrollArea->setAlignment( Qt::AlignTop );
	auto pLayout2 = new QVBoxLayout( scrollArea );
	pLayout2->setAlignment( Qt::AlignTop );
	pLayout2->setObjectName( "SDKItemLayout" );

	pLayout->addWidget( scrollArea, 0, 0 );

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
		return;
	}

	QJsonObject JSONConfig = JSONConfigDocument.object();
	for ( auto it = JSONConfig.begin(); it != JSONConfig.end(); it++ )
	{
		auto pHeader = new QLabel( it.key(), this );
		pHeader->setObjectName( "Header" );
		pLayout2->addWidget( pHeader );
		QJsonArray arr = it.value().toArray();

		for ( const auto &i : arr )
		{
			auto item = i.toObject();

			auto *pButton = new QPushButton( this );
			pButton->setIcon( QIcon( item["icon"].toString() ) );
			pButton->setText( item["name"].toString() );
			pButton->setObjectName( "MediaItem" );

			pLayout2->addWidget( pButton );

			auto pushButton = [&]()
			{
				auto arr = item["args"].toArray().toVariantList();
				QStringList args;

				foreach( QVariant vItem, arr )
				{
					args << vItem.toString();
				}

				if ( item["urlType"].toString() == "url" )
					OpenUrl( item["url"].toString() );
				else if ( item["urlType"].toString() == "process" )
					OpenProcess( item["url"].toString(), args );
				else
					qDebug() << "Unknown URL Type: " << item["urlType"].toString();
			};

			connect( pButton, &QPushButton::pressed, this, pushButton );
		}
	}

	scrollArea->setFixedSize( pLayout2->sizeHint() );
	pLayout->setAlignment( Qt::AlignTop );
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
