#pragma once

#include "zip_handler.h"

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
		friend class CEditConfig;
		friend class CModManager;
		Q_OBJECT;

	public:
		CMainView( QWidget *pParent );
		QListWidget *m_pListWidget;
		static void OpenUrl( const QString &url );
		void OpenProcess( const QString &execName, const QStringList &params );

	private:
		QString m_installDir;
	};
} // namespace ui