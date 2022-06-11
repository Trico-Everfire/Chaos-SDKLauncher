#include "mainview.h"

using namespace ui;

// General list things

struct MediaItem_t
{
	QString name;
	QString icon;
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
			{ "Hammer World Editor", ":/resource/logo.png" },
			{ "Model Viewer", ":/resource/logo.png" },
			{ "Face Poser", ":/resource/logo.png" },
			{ "P2:CE (Tools Mode)", ":/resource/logo.png" },
			{ "P2:CE", ":/resource/logo.png" }
		},
	},
	{
		"Documentation",
		{
			{ "Valve Developer Community", ":/resource/logo.png" },
			{ "Chaos Wiki", ":/resource/logo.png" },
			{ "Momentum Wiki", ":/resource/logo.png" }
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
		}
	}

	// Set focus so we don't have focus directly on the top most button
	this->setFocus( Qt::NoFocusReason );

	this->setFixedWidth( 250 );
	this->setFixedHeight( this->sizeHint().height() );
}