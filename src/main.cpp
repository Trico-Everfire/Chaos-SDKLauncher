#include "config.h"
#include "mainview.h"

#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QJsonDocument>

int main( int argc, char **argv )
{
	QApplication app( argc, argv );

	QApplication::setAttribute( Qt::AA_DisableWindowContextHelpButton );
	QApplication::setWindowIcon( QIcon( ":/resource/logo.png" ) );

	QFile file( ":/resource/style.qss" );
	file.open( QFile::ReadOnly );
	QString styleSheet = QLatin1String( file.readAll() );
	qApp->setStyleSheet( styleSheet );

	QFile configFile( "./config.json" );
	if ( !configFile.exists() )
	{
		configFile.open( QFile::WriteOnly );
		QJsonDocument doc = defaultConfig();
		configFile.write( doc.toJson() );
		configFile.close();
	}

	auto pDialog = new ui::CMainView( nullptr );
	pDialog->setWindowTitle( "P2:CE SDK Launcher" );
	pDialog->show();

	return QApplication::exec();
}