#pragma once

#include "mainview.h"

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QSpinBox>
#include <QTextEdit>
namespace ui
{
	// The main edit config dialog class
	class CEditConfig : public QDialog
	{
	public:
		CEditConfig( CMainView *parent );
		QListWidget *pEditList;
		QPushButton *applyButton;
		QPushButton *cancelButton;
	};

	// The popup for config items we want to edit.
	class CEditConfigPopup : public QDialog
	{
		bool applyChanges = false;

	public:
		CEditConfigPopup( CEditConfig *parent );
		bool shouldApplyChanges() const;
		QComboBox *pTypeComboBox;
		QLineEdit *pNameLineEdit;
		QLineEdit *pUrlLineEdit;
		QLineEdit *pIconPathLineEdit;
		QTextEdit *pArgumentsListTextEdit;
		QLabel *pNameLabel;
		QLabel *pTypeLabel;
		QLabel *pUrlLabel;
		QLabel *pIconLabel;
		QLabel *pArgumentsLabel;
		QPushButton *applyBox;
	};
} // namespace ui

//{
//	"args": [
//		"-tools"
//],
//	"icon": ":/resource/logo_tools.png",
//	"name": "P2:CE (Panorama) (Tools Mode)",
//	"url": "${INSTALLDIR}p2ce.sh",
//	"urlType": "process"
//},