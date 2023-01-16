#pragma once

#include "mainview.h"

#include <QDialog>
#include <QFileInfo>
#include <QSplashScreen>

#define GIT_URL "https://codeload.github.com/ChaosInitiative/p2ce-mod-template/zip/refs/heads/main"

namespace ui
{
	class CModManager : public QDialog
	{
	public:
		CModManager( CMainView *pCreateOrCancelButton );

	private:
		void modCreationHandler();
		static bool modZipHandler( const QFileInfo &zFile, const QString &modName, const QString &modPath );
		QLineEdit *m_pModNameLineEdit;
		QLineEdit *m_pModPathLineEdit;
		QPushButton *m_pCreateButton;
	};
} // namespace ui
