#pragma once

#include "ziphandler.h"

#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QLabel>
#include <QListWidget>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QSplashScreen>
#include <QUrl>
#include <QVBoxLayout>
#include <QVector>

namespace ui
{
	// This class is responsible for the main window of the application.
	class CMainView : public QDialog
	{
		Q_OBJECT;
	public:
		CMainView( QWidget *pParent );
		QListWidget *m_pListWidget;
		static void OpenUrl( const QString &url );
		void OpenProcess( const QString &execName, const QStringList &params );
		QString GetInstallDir();

	private:
		QString m_InstallDir;
	};
} // namespace ui