#pragma once

#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QVector>

namespace ui
{
	class CMainView : public QDialog
	{
		Q_OBJECT;
	public:
		CMainView( QWidget *pParent );
	private:
		QString m_pInstallDir;
		void OpenUrl( QString url );
		void OpenProcess( QString execName, QStringList params );
	};
} // namespace ui