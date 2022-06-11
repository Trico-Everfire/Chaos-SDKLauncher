#pragma once

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QProcess>
#include <QVector>

namespace ui
{
	class CMainView : public QDialog
	{
		Q_OBJECT;

	public:
		CMainView( QWidget *pParent );
	};
}