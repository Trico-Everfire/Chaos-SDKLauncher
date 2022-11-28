#include "editconfig.h"

#include <QDialogButtonBox>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSizePolicy>

ui::CEditConfig::CEditConfig( CMainView *parent ) :
	QDialog( parent )
{
	// We set the title and create a grid layout for our elements.
	setWindowTitle( "Edit Launcher Configuration" );
	auto pEditConfigurationLayout = new QGridLayout( this );
	pEditList = new QListWidget( this );
	pEditList->setItemAlignment( Qt::AlignCenter );
	// We then loop over the items inside m_pListWidget from mainview.cpp
	// and populate our own list widget but with normal labels instead
	// of buttons, this way we can select and edit them to our liking.
	int listChildCount = parent->m_pListWidget->layout()->count();
	for ( int i = 0; i < listChildCount; i++ )
	{
		// We extract each label or button and read its
		// contents which is stored in JSON and then
		// stored inside the item's data.
		// labels are always categories and only hold
		// two pieces of data: their name and the category identifier.
		auto pMainWindowListItem = parent->m_pListWidget->layout()->itemAt( i );
		if ( auto pMainWindowListLabel = dynamic_cast<QLabel *>( pMainWindowListItem->widget() ) )
		{
			auto pListWidgetItem = new QListWidgetItem();
			pListWidgetItem->setText( pMainWindowListLabel->text() );
			auto listItemFont = pListWidgetItem->font();
			listItemFont.setBold( true );
			pListWidgetItem->setFont( listItemFont );
			auto listItemContentObject = QJsonObject();
			listItemContentObject["name"] = pMainWindowListLabel->text();
			listItemContentObject["urlType"] = "category";
			pListWidgetItem->setData( Qt::UserRole, listItemContentObject );
			pEditList->addItem( pListWidgetItem );
		}
		// Buttons hold a lot more information, so we pass the data
		// they hold directly as we need everything stored.
		if ( auto pMainWindowListButton = dynamic_cast<QPushButton *>( pMainWindowListItem->widget() ) )
		{
			auto pListWidgetItem = new QListWidgetItem();
			pListWidgetItem->setText( pMainWindowListButton->text() );
			pListWidgetItem->setIcon( pMainWindowListButton->icon() );
			pListWidgetItem->setData( Qt::UserRole, pMainWindowListButton->property( "JSONData" ) );
			pEditList->addItem( pListWidgetItem );
		}
	}

	// We set the general alignment for all the items in the configuration layout.
	pEditConfigurationLayout->setAlignment( Qt::AlignTop );

	// We then create the 5 buttons responsible for editing individual
	// instances of executables/urls/categories.
	pEditConfigurationLayout->addWidget( pEditList, 0, 0, 5, 1 );
	auto addCurrentButton = new QPushButton( this );
	addCurrentButton->setIcon( QIcon( ":/resource/add.png" ) );
	pEditConfigurationLayout->addWidget( addCurrentButton, 0, 1, Qt::AlignTop | Qt::AlignRight );
	auto shiftUpButton = new QPushButton( this );
	shiftUpButton->setIcon( QIcon( ":/resource/arrowup.png" ) );
	pEditConfigurationLayout->addWidget( shiftUpButton, 1, 1, Qt::AlignTop | Qt::AlignRight );
	auto editButton = new QPushButton( this );
	editButton->setIcon( QIcon( ":/resource/edit.png" ) );
	pEditConfigurationLayout->addWidget( editButton, 2, 1, Qt::AlignTop | Qt::AlignRight );
	auto shiftDownButton = new QPushButton( this );
	shiftDownButton->setIcon( QIcon( ":/resource/arrowdown.png" ) );
	pEditConfigurationLayout->addWidget( shiftDownButton, 3, 1, Qt::AlignTop | Qt::AlignRight );
	auto removeCurrentButton = new QPushButton( this );
	removeCurrentButton->setIcon( QIcon( ":/resource/remove.png" ) );
	pEditConfigurationLayout->addWidget( removeCurrentButton, 4, 1, Qt::AlignTop | Qt::AlignRight );

	// We create a dialog button box to allow us
	// to cancel or apply the changes the user may have made.
	auto dialogButtons = new QDialogButtonBox( this );
	applyButton = dialogButtons->addButton( "Apply", QDialogButtonBox::ButtonRole::ApplyRole );
	cancelButton = dialogButtons->addButton( "Cancel", QDialogButtonBox::ButtonRole::RejectRole );
	pEditConfigurationLayout->addWidget( dialogButtons, 5, 0, 1, 2, Qt::AlignTop | Qt::AlignLeft );

	// This callback checks if the first element in the list
	// is a category. As a category defies what elements are
	// inside of it.
	auto onCurrentRowChangedCallback = [&]( int row )
	{
		// We first check if there is an item in the first place.
		// Then check if the item is a category.
		auto current = pEditList->item( 0 );
		if ( !current )
		{
			applyButton->setDisabled( true );
			applyButton->setToolTip( "The top element must be a category." );
			return;
		};
		applyButton->setDisabled( false );
		applyButton->setToolTip( "" );
		if ( current->data( Qt::UserRole ).toJsonObject()["urlType"] != "category" )
		{
			applyButton->setDisabled( true );
			applyButton->setToolTip( "The top element must be a category." );
		}
	};

	// This callback creates a dialog to add a new item to
	// the list. It's similar to onEditItemButtonPressedCallback
	// but with the key difference that it doesn't set any
	// presets and creates a new item instead of editing one.
	auto onAddItemButtonPressedCallback = [&]
	{
		// We first check if an item is selected.
		// and use it later to determine the new
		// item's position in the list.
		// Then create an instance of CEditConfigPopup
		// The application then waits for the closing
		// of its exec(); and check if we should apply
		// the changes made by the user.
		auto pCurrentItem = pEditList->currentItem();
		auto pEditConfigPopup = new CEditConfigPopup( this );
		pEditConfigPopup->exec();
		if ( !pEditConfigPopup->shouldApplyChanges() )
			return;

		// We then apply the visuals of the item.
		// depending on the type, it'll receive JSON data.
		// it's either only the name and type,
		// name, type, icon and url, or name, type, url, icon and arguments.
		auto pNewListItem = new QListWidgetItem();
		pNewListItem->setText( pEditConfigPopup->pNameLineEdit->text() );
		pNewListItem->setIcon( QIcon() );
		if ( QFileInfo::exists( pEditConfigPopup->pIconPathLineEdit->text() ) )
			pNewListItem->setIcon( QIcon( pEditConfigPopup->pIconPathLineEdit->text() ) );

		auto listItemJSONContents = QJsonObject();
		listItemJSONContents["name"] = pEditConfigPopup->pNameLineEdit->text();
		if ( pEditConfigPopup->pTypeComboBox->currentIndex() == 2 )
		{
			// Categories are highlighted in bold to distinguish
			// them from buttons.
			// Provided they need the least care,
			// We do all what's needed and return here to
			// make things easier.
			auto listItemFont = pNewListItem->font();
			listItemFont.setBold( true );
			pNewListItem->setFont( listItemFont );
			listItemJSONContents["urlType"] = "category";
			pNewListItem->setData( Qt::UserRole, listItemJSONContents );
			pEditList->currentRowChanged( 0 );
			int currentRow = pEditList->row( pCurrentItem );
			pEditList->insertItem( currentRow + 1, pNewListItem );
			return;
		}
		listItemJSONContents["url"] = pEditConfigPopup->pUrlLineEdit->text();
		listItemJSONContents["icon"] = pEditConfigPopup->pIconPathLineEdit->text();
		listItemJSONContents["urlType"] = "url";
		if ( pEditConfigPopup->pTypeComboBox->currentIndex() == 0 )
		{
			// We need to convert the arguments from a
			// QString to a QStringList.
			auto jsonArgumentList = QJsonArray();
			foreach( auto listArgument, pEditConfigPopup->pArgumentsListTextEdit->toPlainText().split( " " ) )
			{
				jsonArgumentList.append( listArgument );
			}
			listItemJSONContents["urlType"] = "process";
			listItemJSONContents["args"] = jsonArgumentList;
		}
		// We then store the JSON data and call the list's row changed function.
		pNewListItem->setData( Qt::UserRole, listItemJSONContents );
		pEditList->currentRowChanged( 0 );

		int currentRow = pCurrentItem ? pEditList->row( pCurrentItem ) : 0;
		pEditList->insertItem( currentRow + 1, pNewListItem );
	};

	// This callback shifts an item's position down by 1.
	// We shift both the selected item position and the
	// selection itself to align it with the shift.
	auto onShiftItemDownButtonPressedCallback = [&]
	{
		auto item = pEditList->currentItem();
		if ( item )
		{
			if ( pEditList->row( item ) + 1 > pEditList->count() - 1 )
				return;
			int currentRow = pEditList->row( item );
			pEditList->takeItem( currentRow );
			pEditList->insertItem( currentRow + 1, item );
			pEditList->setCurrentRow( currentRow + 1 );
		}
	};

	// This callback grabs the contents from an item and
	// puts it into an instance of CEditConfigPopup.
	// Where the user can edit the contents and type
	// of the item, which changes will be applied if
	// the apply button in CEditConfigPopup is pressed.
	auto onEditItemButtonPressedCallback = [&]
	{
		// This function is very similar to onAddItemButtonPressedCallback
		// The difference being is that instead of creating a new pSelectedItem
		// we use the current selected pSelectedItem and fill the new instance
		// of the edit config class with its contents.
		// then later store the JSON data and also call
		// the list's row changed function.
		auto pSelectedItem = pEditList->currentItem();
		if ( !pSelectedItem )
			return;
		auto listItemJSONContents = pSelectedItem->data( Qt::UserRole ).toJsonObject();
		auto pEditConfigPopup = new CEditConfigPopup( this );
		pEditConfigPopup->setWindowTitle( "Edit " + pSelectedItem->text() );
		if ( listItemJSONContents["urlType"] == "category" )
		{
			pEditConfigPopup->pNameLineEdit->setText( pSelectedItem->text() );
			pEditConfigPopup->pTypeComboBox->setCurrentIndex( 2 );
		}
		else
		{
			pEditConfigPopup->pNameLineEdit->setText( listItemJSONContents["name"].toString() );
			pEditConfigPopup->pUrlLineEdit->setText( listItemJSONContents["url"].toString() );
			pEditConfigPopup->pIconPathLineEdit->setText( listItemJSONContents["icon"].toString() );
			pEditConfigPopup->pTypeComboBox->setCurrentIndex( 1 );
			if ( listItemJSONContents["urlType"] == "process" )
			{
				pEditConfigPopup->pTypeComboBox->setCurrentIndex( 0 );
				auto oldJSONArgumentList = QString( "" );
				// We need to convert the arguments from a
				// JSON Array to a QString for the
				// QTextEdit to understand.
				foreach( QJsonValue str, listItemJSONContents["args"].toArray() )
				{
					oldJSONArgumentList.append( str.toString() + " " );
				}
				pEditConfigPopup->pArgumentsListTextEdit->setText( oldJSONArgumentList );
			}
		}

		pEditConfigPopup->exec();
		if ( !pEditConfigPopup->shouldApplyChanges() )
			return;

		pSelectedItem->setText( pEditConfigPopup->pNameLineEdit->text() );
		pSelectedItem->setIcon( QIcon() );
		if ( QFileInfo::exists( pEditConfigPopup->pIconPathLineEdit->text() ) )
			pSelectedItem->setIcon( QIcon( pEditConfigPopup->pIconPathLineEdit->text() ) );

		auto newListItemJSONContents = QJsonObject();
		newListItemJSONContents["name"] = pEditConfigPopup->pNameLineEdit->text();
		if ( pEditConfigPopup->pTypeComboBox->currentIndex() == 2 )
		{
			// Categories are highlighted in bold to distinguish
			// them from buttons.
			// Provided they need the least care,
			// We do all what's needed and return here to
			// make things easier.
			auto listItemFont = pSelectedItem->font();
			listItemFont.setBold( true );
			pSelectedItem->setFont( listItemFont );
			newListItemJSONContents["urlType"] = "category";
			pSelectedItem->setData( Qt::UserRole, newListItemJSONContents );
			pSelectedItem->setIcon( QIcon() );
			pEditList->currentRowChanged( 0 );
			return;
		}
		newListItemJSONContents["url"] = pEditConfigPopup->pUrlLineEdit->text();
		newListItemJSONContents["icon"] = pEditConfigPopup->pIconPathLineEdit->text();
		newListItemJSONContents["urlType"] = "url";
		if ( pEditConfigPopup->pTypeComboBox->currentIndex() == 0 )
		{
			// We need to convert the arguments from a
			// QString to a QStringList.
			auto newJSONArgumentList = QJsonArray();
			foreach( auto arg, pEditConfigPopup->pArgumentsListTextEdit->toPlainText().split( " " ) )
			{
				newJSONArgumentList.append( arg );
			}
			newListItemJSONContents["urlType"] = "process";
			newListItemJSONContents["args"] = newJSONArgumentList;
		}
		pSelectedItem->setData( Qt::UserRole, newListItemJSONContents );
		pEditList->currentRowChanged( 0 );
	};

	// This callback is like onShiftItemDownButtonPressedCallback
	// except it shifts it up by 1.
	auto onShiftItemUpButtonPressedCallback = [&]
	{
		auto item = pEditList->currentItem();
		if ( item )
		{
			if ( pEditList->row( item ) - 1 < 0 )
				return;
			int currentRow = pEditList->row( item );
			pEditList->takeItem( currentRow );
			pEditList->insertItem( currentRow - 1, item );
			pEditList->setCurrentRow( currentRow - 1 );
		}
	};

	// This callback removes the currently selected item.
	auto onRemoveItemButtonPressedCallback = [&]
	{
		auto item = pEditList->currentItem();
		if ( !item )
			return;
		int currentItemRow = pEditList->row( item );
		pEditList->takeItem( currentItemRow );
		delete item;
		pEditList->currentRowChanged( 0 );
	};

	// This callback checks if the user pressed to apply or cancel buttons.
	// If the apply button is set, we apply all changes to the items
	// in CMainView list widget and write the config file to save these changes.
	auto onDialogButtonsPressedCallback = [&]( QAbstractButton *button )
	{
		if ( button->text() == "Apply" )
		{
			// We first empty the CMainView's list widget and destroy its items.
			// Then we repopulate that list widget with the contents the user
			// created.
			auto pParentWidget = dynamic_cast<CMainView *>( this->parentWidget() );
			auto pMainListLayout = pParentWidget->m_pListWidget->layout();
			while ( pMainListLayout->count() > 0 )
			{
				auto pMainListLayoutItem = pMainListLayout->takeAt( 0 );
				auto pMainListLayoutItemWidget = pMainListLayoutItem->widget();
				delete pMainListLayoutItemWidget;
				delete pMainListLayoutItem;
			}
			// We now need to now write the new config configFile
			// as well as repopulate the now empty list widget.
			// however, due to the config's structure,
			// we need to be able to set what category it's
			// currently using. This requires us to use
			// a JSON Array pointer, which we store in both a vector
			// and the variable pCurrentCategory,
			// which will be populated until a new category is found or
			// the loop runs out of items.
			auto configJSONDocument = QJsonDocument();
			auto mainJSONArray = QJsonArray();

			QJsonArray *pCurrentCategory;
			auto jsonArrayVector = QVector<QPair<QString, QJsonArray *>>();

			for ( int i = 0; i < pEditList->count(); i++ )
			{
				auto pListItem = pEditList->item( i );
				auto itemJSONContents = pListItem->data( Qt::UserRole ).toJsonObject();
				if ( itemJSONContents["urlType"] == "category" )
				{
					// When a category is found
					// We set a new QJsonArray in pCurrentCategory,
					// then store pCurrentCategory inside jsonArrayVector
					// We then apply the header's name to a new label
					// and add it to pMainListLayout.
					pCurrentCategory = new QJsonArray();
					jsonArrayVector.append( QPair<QString, QJsonArray *>( itemJSONContents["name"].toString(), pCurrentCategory ) );
					auto pHeader = new QLabel( itemJSONContents["name"].toString(), this );
					pHeader->setObjectName( "Header" );
					pMainListLayout->addWidget( pHeader );
					continue;
				}
				// We append the current JSON contents to pCurrentCategory.
				// Then create a new button and add it to pMainListLayout.
				pCurrentCategory->append( itemJSONContents );
				auto *pItemButton = new QPushButton( this );
				pItemButton->setIcon( QIcon( itemJSONContents["icon"].toString() ) );
				pItemButton->setText( itemJSONContents["name"].toString() );
				pItemButton->setObjectName( "MediaItem" );
				pItemButton->setProperty( "JSONData", itemJSONContents );
				pMainListLayout->addWidget( pItemButton );

				// We create a callback function for handling the URL/Process trigger.
				auto onItemButtonPushedCallback = [&, itemJSONContents, pParentWidget]
				{
					// We need to convert the arguments from a
					// JSON variant list to a QStringList for the
					// process executor to understand.
					auto arr = itemJSONContents["processArguments"].toArray().toVariantList();
					QStringList processArguments;

					foreach( QVariant vItem, arr )
					{
						processArguments << vItem.toString();
					}

					if ( itemJSONContents["urlType"].toString() == "url" )
						CMainView::OpenUrl( itemJSONContents["url"].toString() );
					else if ( itemJSONContents["urlType"].toString() == "process" )
						pParentWidget->OpenProcess( itemJSONContents["url"].toString(), processArguments );
					else
						qDebug() << "Unknown URL Type: " << itemJSONContents["urlType"].toString();
				};

				connect( pItemButton, &QPushButton::pressed, this, onItemButtonPushedCallback );
			}

			// We now unravel our stored contents, a QPair containing a QString
			// and QJsonArray pointer. We grab the string for the header's name
			// and the QJsonArray for the header's contents. We need to
			// convert it from a pointer into a reference first.
			for ( const auto &jsonArrayStringPair : jsonArrayVector )
			{
				qInfo() << jsonArrayStringPair.first;
				auto headerObject = QJsonObject();
				headerObject["header"] = jsonArrayStringPair.first;
				headerObject["content"] = *jsonArrayStringPair.second;
				mainJSONArray.append( headerObject );
				// we delete the QJsonArray pointers, I don't trust QT enough to get rid of them after
				// QVector is done with them.
				delete jsonArrayStringPair.second;
			}
			configJSONDocument.setArray( mainJSONArray );
			auto configFile = QFile( "./config.json" );
			configFile.open( QFile::WriteOnly );
			configFile.write( configJSONDocument.toJson() );
			configFile.close();
		}
		this->close();
	};

	// We then use these callbacks.
	connect( pEditList, &QListWidget::currentRowChanged, this, onCurrentRowChangedCallback );
	connect( addCurrentButton, &QPushButton::pressed, this, onAddItemButtonPressedCallback );
	connect( shiftDownButton, &QPushButton::pressed, this, onShiftItemDownButtonPressedCallback );
	connect( editButton, &QPushButton::pressed, this, onEditItemButtonPressedCallback );
	connect( shiftUpButton, &QPushButton::pressed, this, onShiftItemUpButtonPressedCallback );
	connect( removeCurrentButton, &QPushButton::pressed, this, onRemoveItemButtonPressedCallback );
	connect( dialogButtons, &QDialogButtonBox::clicked, this, onDialogButtonsPressedCallback );

	// to prevent buttons from aligning equally across the pEditList widget's
	// height, we cap the stretch at 1, keeping them all neatly lined up.
	pEditConfigurationLayout->setRowStretch( 4, 1 );

	// Set focus, so we don't have focus directly on the top most item
	this->setFocus( Qt::NoFocusReason );
}

ui::CEditConfigPopup::CEditConfigPopup( CEditConfig *parent ) :
	QDialog( parent )
{
	auto pConfigPopupLayout = new QGridLayout( this );
	pConfigPopupLayout->setAlignment( Qt::AlignTop | Qt::AlignRight );

	pNameLabel = new QLabel( this );
	pNameLabel->setText( "Name:" );
	pConfigPopupLayout->addWidget( pNameLabel, 0, 0 );
	pNameLineEdit = new QLineEdit( this );
	pConfigPopupLayout->addWidget( pNameLineEdit, 0, 1 );

	pTypeLabel = new QLabel( this );
	pTypeLabel->setText( "Type:" );
	pConfigPopupLayout->addWidget( pTypeLabel, 0, 2 );
	pTypeComboBox = new QComboBox( this );
	pTypeComboBox->addItem( "Executable" );
	pTypeComboBox->addItem( "Url" );
	pTypeComboBox->addItem( "Category" );
	pConfigPopupLayout->addWidget( pTypeComboBox, 0, 3 );

	pUrlLabel = new QLabel( this );
	pUrlLabel->setText( "Exec:" );
	pConfigPopupLayout->addWidget( pUrlLabel, 1, 0 );
	pUrlLineEdit = new QLineEdit( this );
	pConfigPopupLayout->addWidget( pUrlLineEdit, 1, 1 );

	pIconLabel = new QLabel( this );
	pIconLabel->setText( "Icon:" );
	pConfigPopupLayout->addWidget( pIconLabel, 1, 2 );
	pIconPathLineEdit = new QLineEdit( this );
	pConfigPopupLayout->addWidget( pIconPathLineEdit, 1, 3 );

	pArgumentsLabel = new QLabel( this );
	pArgumentsLabel->setText( "Arg:" );
	pConfigPopupLayout->addWidget( pArgumentsLabel, 2, 0 );
	pArgumentsListTextEdit = new QTextEdit( this );
	pArgumentsListTextEdit->setFixedHeight( pIconPathLineEdit->sizeHint().height() );
	pConfigPopupLayout->addWidget( pArgumentsListTextEdit, 2, 1, 1, 3 );

	auto pAcceptCancelButtonBox = new QDialogButtonBox( this );
	applyBox = pAcceptCancelButtonBox->addButton( "Apply", QDialogButtonBox::ButtonRole::ApplyRole );
	pAcceptCancelButtonBox->addButton( "Cancel", QDialogButtonBox::ButtonRole::RejectRole );
	pConfigPopupLayout->addWidget( pAcceptCancelButtonBox, 3, 0, 1, 4, Qt::AlignLeft );

	// In this callback we need to check if pNameLineEdit is empty or only spaces for categories.
	// Because this can break things if they're empty, JSON doesn't fare well with empty keys.
	connect( pNameLineEdit, &QLineEdit::textChanged, this, [&]( const QString &text )
			 {
				 if ( pTypeComboBox->currentIndex() == 2 )
				 {
					 applyBox->setEnabled( text.count( " " ) != text.count() );
					 applyBox->setToolTip( pNameLineEdit->text().count( " " ) != pNameLineEdit->text().count() ? "" : "Categories MUST have a name." );
				 }
			 } );

	// This callback checks if the user pressed to apply or cancel buttons.
	// If the apply button is set, we apply all changes to the item.
	connect( pAcceptCancelButtonBox, &QDialogButtonBox::clicked, this, [&]( QAbstractButton *button )
			 {
				 if ( button->text() == "Apply" )
				 {
					 this->applyChanges = true;
				 }
				 this->close();
			 } );

	// When pTypeComboBox changes, certain aspects become disabled.
	// An URL doesn't use parameters, and a category only uses pNameLineEdit
	// So we disable and enable fields depending on the combo box's current
	// index.
	connect( pTypeComboBox, &QComboBox::currentTextChanged, this, [&]( const QString &text )
			 {
				 if ( text == "Executable" )
				 {
					 pUrlLabel->setText( "Exec:" );
					 pUrlLabel->setEnabled( true );
					 pUrlLineEdit->setEnabled( true );
					 pIconLabel->setEnabled( true );
					 pIconPathLineEdit->setEnabled( true );
					 pArgumentsLabel->setEnabled( true );
					 pArgumentsListTextEdit->setEnabled( true );
					 applyBox->setEnabled( true );
					 applyBox->setToolTip( "" );
					 return;
				 }
				 if ( text == "Url" )
				 {
					 pUrlLabel->setText( "Url:" );
					 pUrlLabel->setEnabled( true );
					 pUrlLineEdit->setEnabled( true );
					 pIconLabel->setEnabled( true );
					 pIconPathLineEdit->setEnabled( true );
					 pArgumentsLabel->setEnabled( false );
					 pArgumentsListTextEdit->setEnabled( false );
					 applyBox->setEnabled( true );
					 applyBox->setToolTip( "" );
					 return;
				 }
				 if ( text == "Category" )
				 {
					 pUrlLabel->setEnabled( false );
					 pUrlLineEdit->setEnabled( false );
					 pIconLabel->setEnabled( false );
					 pIconPathLineEdit->setEnabled( false );
					 pArgumentsLabel->setEnabled( false );
					 pArgumentsListTextEdit->setEnabled( false );
					 applyBox->setToolTip( pNameLineEdit->text().count( " " ) != pNameLineEdit->text().count() ? "" : "Categories MUST have a name." );
					 applyBox->setEnabled( pNameLineEdit->text().count( " " ) != pNameLineEdit->text().count() );
					 return;
				 }
			 } );
}

bool ui::CEditConfigPopup::shouldApplyChanges() const
{
	return this->applyChanges;
}
