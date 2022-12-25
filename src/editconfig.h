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
		QListWidget *m_pEditList;
		QPushButton *m_pApplyButton;
		QPushButton *pAddCurrentButton;
	};

	// The popup for config items we want to edit.
	class CEditConfigPopup : public QDialog
	{
		bool m_applyChanges = false;

	public:
		CEditConfigPopup( CEditConfig *parent );
		bool shouldApplyChanges() const;
		QComboBox *m_pTypeComboBox;
		QLineEdit *m_pNameLineEdit;
		QLineEdit *m_pUrlLineEdit;
		QLineEdit *m_pIconPathLineEdit;
		QTextEdit *m_pArgumentsListTextEdit;
		QPushButton *m_pApplyButton;
	};
} // namespace ui