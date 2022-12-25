//
// Created by trico on 14-12-22.
//

#ifndef SDKLAUNCHER_MODMANAGER_H
#define SDKLAUNCHER_MODMANAGER_H

#include "mainview.h"

#include <QDialog>
#include <QFileInfo>
#include <QSplashScreen>

using namespace ui;
namespace ui
{
	class CModManager : public QDialog
	{
	public:
		CModManager( CMainView *pCreateOrCancelButton );

	private:
		void modCreationHandler();
		static bool modZipHandler( const QFileInfo &zFile, QSplashScreen *sps, const QString &modName, const QString &modPath );
		QLineEdit *m_pModNameLineEdit;
		QLineEdit *m_pModPathLineEdit;
		QPushButton *m_pCreateButton;
	};
} // namespace ui
#endif // SDKLAUNCHER_MODMANAGER_H
