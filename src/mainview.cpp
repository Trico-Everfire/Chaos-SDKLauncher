#include "mainview.h"

using namespace ui;

// General list things

struct MediaItem_t
{
	QString name;
	QString icon;
	QString urlType;
	QString url;
	QStringList args;
};

struct MediaSection_t
{
	QString header;
	QVector<MediaItem_t> items;
};

static QVector<MediaSection_t> Sections(
{
	{
		"Applications",
		{
			{ "Hammer World Editor", ":/resource/hammer.png", "process", "hammer.exe", { "" } },
			{ "Model Viewer", ":/resource/modelviewer.png", "process", "hlmv.exe", { "-game p2ce" } },
			{ "Face Poser", ":/resource/faceposer.png", "process", "hlfaceposer.exe", { "-game p2ce" } },
			{ "P2:CE (Tools Mode)", ":/resource/logo_tools.png", "process", "chaos.exe", { "-game p2ce", "-tools" } },
			{ "P2:CE", ":/resource/logo.png", "process", "chaos.exe", { "-game p2ce" } }
		},
	},
	{
		"Documentation",
		{
			{ "Valve Developer Community", ":/resource/vdc.png", "url", "https://developer.valvesoftware.com", { "" } },
			{ "Chaos Wiki", ":/resource/chaos.png", "url", "https://chaosinitiative.github.io/Wiki/", { "" } },
			{ "Momentum Wiki", ":/resource/momentum.png", "url", "https://docs.momentum-mod.org/", { "" } }
		}
	}
});


CMainView::CMainView( QWidget *pParent ) : QDialog( pParent )
{
	auto pLayout = new QVBoxLayout( this );
	pLayout->setObjectName( "SDKLayout" );

	for ( int i = 0; i < Sections.count(); i++ )
	{
		MediaSection_t section = Sections.at( i );

		auto pHeader = new QLabel( section.header, this );
		pHeader->setObjectName( "Header" );
		pLayout->addWidget( pHeader );

		for ( int j = 0; j < section.items.count(); j++ )
		{
			MediaItem_t item = section.items.at( j );

			QPushButton *pButton = new QPushButton( this );
			pButton->setIcon( QIcon( item.icon ) );
			pButton->setText( item.name );
			pButton->setObjectName( "MediaItem" );

			pLayout->addWidget( pButton );

			connect(pButton, &QPushButton::released, this,
				[=]()
				{
						 if ( item.urlType == "url" )
							 OpenUrl( item.url );
						 else if ( item.urlType == "process" )
							 OpenProcess( item.url, item.args );
						 else
							 qDebug() << "Unknown URL Type: " << item.urlType;
				});
		}
	}

	// Set focus so we don't have focus directly on the top most button
	this->setFocus( Qt::NoFocusReason );

	this->setFixedWidth( 250 );
	this->setFixedHeight( this->sizeHint().height() );
}

void CMainView::OpenUrl( QString url )
{
	QDesktopServices::openUrl( QUrl( url ) );
}

void CMainView::OpenProcess( QString execName, QStringList params )
{
	auto pProcess = new QProcess( this );
	
	pProcess->start( execName, params);
}
