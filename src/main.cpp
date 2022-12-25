#include "mainview.h"

#include <QApplication>
#include <QFile>
#include <QIcon>
#ifdef USE_STEAM
#include <QMessageBox>
#include <stdlib.h>
#endif

// The Steam API is used by other applications.
// There is the choice to have the application
// run in "online" and "offline" mode.
// The Steam API REQUIRES you to have
// both internet access AND steam running.
// So we are locking anything that's steam related.
// behind a steam related if def.
#ifdef USE_STEAM
#include <steam_api.h>
#endif

// AppID which we will use for steam.
// 440000 - P2CE
// 1400890 - P2CE SDK
// 1802710 - Momentum Mod
constexpr int APP_ID = 440000;

#ifdef USE_STEAM
// This function is called when steam shuts down.
void shutdown_steam()
{
	SteamAPI_Shutdown();
}
#endif

int main( int argc, char **argv )
{
	// Set the env variables for this application because we don't need steam_appid.txt
	// We also use these across the application.
	qputenv( "SteamAppId", QString::number( APP_ID ).toLocal8Bit() );
	qputenv( "SteamGameId", QString::number( APP_ID ).toLocal8Bit() );

	QApplication app( argc, argv );

#ifdef USE_STEAM
	// Call Steam
	if ( !SteamAPI_Init() )
	{
		QMessageBox::critical( nullptr, "Fatal Error", "Steam must be running to use this tool (SteamAPI_Init() failed)." );
		return 1;
	}
#endif

	// AA_DisableWindowContextHelpButton
	// QApplication::setAttribute( Qt::AA_DisableWindowContextHelpButton  );
	QApplication::setWindowIcon( QIcon( ":/resource/logo.png" ) );

	// We set the style from the resources.
	QFile file( ":/resource/style.qss" );
	file.open( QFile::ReadOnly );
	QString styleSheet = QLatin1String( file.readAll() );
	qApp->setStyleSheet( styleSheet );

	auto pDialog = new ui::CMainView( nullptr );
	pDialog->setWindowTitle( "P2:CE SDK Launcher" );
	pDialog->show();

#ifdef USE_STEAM
	atexit( shutdown_steam );
#endif

	return QApplication::exec();
}