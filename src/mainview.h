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
		QString m_pInstallDir;
	public:
		CMainView( QWidget *pParent );

	private:
		void OpenUrl( QString url );
		void OpenProcess( QString execName, QStringList params );
	};
} // namespace ui