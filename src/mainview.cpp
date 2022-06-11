#include "mainview.h"

#include <QDebug>
#include <QVector>

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
		}
	}
});


CMainView::CMainView( QWidget *pParent ) : QDialog( pParent )
{
	auto pLayout = new QVBoxLayout( this );

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

	this->setFixedWidth( 250 );
	this->setFixedHeight( this->sizeHint().height() );
}