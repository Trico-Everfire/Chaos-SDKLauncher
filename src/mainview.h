#pragma once

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QProcess>
#include <QVector>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

namespace ui
{
	class CMainView : public QDialog
	{
		Q_OBJECT;

	public:
		CMainView( QWidget *pParent );

	private:
		void OpenUrl( QString url );
		void OpenProcess( QString execName, QStringList params );
	};
}