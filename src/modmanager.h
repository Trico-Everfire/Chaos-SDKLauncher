//
// Created by trico on 14-12-22.
//

#ifndef SDKLAUNCHER_MODMANAGER_H
#define SDKLAUNCHER_MODMANAGER_H

#include "mainview.h"

#include <QFileInfo>
#include <QSplashScreen>
#include <QDialog>

using namespace ui;
namespace ui
{
	class CModManager : public QDialog
	{
	public:
		CModManager( CMainView *pCreateOrCancelButton );

	private:
		void modCreationHandler();
		static bool modZipHandler( const QFileInfo& zFile, QSplashScreen *sps, const QString& modName, const QString& modPath );
		QLineEdit *m_pModNameLineEdit;
		QLineEdit *m_pModPathLineEdit;
		QPushButton *m_pCreateButton;
	};
}
#endif // SDKLAUNCHER_MODMANAGER_H
