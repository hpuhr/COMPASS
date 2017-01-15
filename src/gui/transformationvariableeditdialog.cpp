/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "TransformationVariableEditDialog.h"
#include "DBObjectManager.h"
#include "DBObject.h"
#include "Global.h"

#include <QMessageBox>


/**********************************************************************************
PropertyWidget
***********************************************************************************/

/**
Constructor.
@param parent Parent widget.
  */
PropertyWidget::PropertyWidget( QWidget* parent )
:   QWidget( parent ),
    var_( NULL )
{
    QHBoxLayout* layout = new QHBoxLayout;

    name_edit_ = new QLineEdit( this );
    dbo_edit_ = new QLineEdit( this );
    dtype_combo_ = new QComboBox( this );
    QPushButton* button = new QPushButton( this );
    button->setText( "Delete" );

    layout->addWidget( dbo_edit_ );
    layout->addWidget( name_edit_ );
    layout->addWidget( dtype_combo_ );
    layout->addWidget( button );

    setLayout( layout );

    name_edit_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    dbo_edit_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    dtype_combo_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    button->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

    dbo_edit_->setReadOnly( true );

    connect( button, SIGNAL(pressed()), this, SIGNAL(deleteMe()) );
}

/**
Destructor.
  */
PropertyWidget::~PropertyWidget()
{
}

/**
Returns the widgets data type combo.
Used to fill the combo box from the outside.
@return Data type combo box.
  */
QComboBox* PropertyWidget::getDataTypeCombo()
{
    return dtype_combo_;
}

/**
Returns the displayed Property.
@return The Property displayed by the widget.
  */
Property PropertyWidget::getProperty() const
{
    const QString& name = name_edit_->text();
    PROPERTY_DATA_TYPE dtype = (PROPERTY_DATA_TYPE)(dtype_combo_->itemData( dtype_combo_->currentIndex() ).toInt());

    return Property( name.toStdString(), dtype );
}

/**
Returns the displayed DBO type.
@return DBO type assigned to the Property entry.
  */
DB_OBJECT_TYPE PropertyWidget::getDBOType() const
{
    return dbo_type_;
}

/**
Returns the associated DBOVariable.
@return The associated DBOVariable.
  */
DBOVariable* PropertyWidget::getDBOVariable()
{
    return var_;
}

/**
Sets the entries DBO type.
@param dbo_type The new DBO type to be stored/displayed.
  */
void PropertyWidget::setDBOType( DB_OBJECT_TYPE dbo_type )
{
    dbo_type_ = dbo_type;
    dbo_edit_->setText( QString::fromStdString( DB_OBJECT_TYPE_STRINGS[ dbo_type ] ) );
}

/**
Sets the entries data type.
@param dtype The data type to be stored/displayed.
  */
void PropertyWidget::setDataType( PROPERTY_DATA_TYPE dtype )
{
    PROPERTY_DATA_TYPE type;
    int i, n = dtype_combo_->count();
    for( i=0; i<n; ++i )
    {
        type = (PROPERTY_DATA_TYPE)(dtype_combo_->itemData( i ).toInt());
        if( dtype == type )
        {
            dtype_combo_->setCurrentIndex( i );
            break;
        }
    }
}

/**
Sets the entries name.
@param name New name to be stored/displayed.
  */
void PropertyWidget::setPropertyName( const QString& name )
{
    name_edit_->setText( name );
}

/**
Sets an associated DBOVariable.
@param var DBOVariable to store.
  */
void PropertyWidget::setDBOVariable( DBOVariable* var )
{
    var_ = var;
}

/**********************************************************************************
TransformationVariableEditDialog
***********************************************************************************/

/**
Constructor.
@param variable TransformationVariable to edit.
@param parent Parent widget.
  */
TransformationVariableEditDialog::TransformationVariableEditDialog( TransformationVariable* variable, QWidget* parent )
:   QDialog( parent ),
    variable_( variable ),
    variable_tmp_( "tmp" )
{
    setupUi( this );

    assert( variable_ );

    init();
}

/**
Destuctor.
  */
TransformationVariableEditDialog::~TransformationVariableEditDialog()
{
}

/**
Inits the dialog.
  */
void TransformationVariableEditDialog::init()
{
    //name is read only, the transformation relies on that!
    name_edit_->setText( QString::fromStdString( variable_->name() ) );
    name_edit_->setReadOnly( true );

    //fill combos
    fillDataTypeCombo( dtype_combo_, true );
    fillDataTypeCombo( prop_dtype_combo_ );
    fillDBOTypeCombo( prop_dbotype_combo_ );

    //init data type combo
    if( variable_->dataTypeChecked() )
        setDataType( (int)variable_->dataType() );
    else
        setDataType( -1 );

    //init default property ui
    Property def_prop;
    bool def_ok = variable_->getDefaultProperty( def_prop );
    if( def_ok )
        default_edit_->setText( QString::fromStdString( def_prop.id_ ) );
    default_box_->setChecked( def_ok );
    default_edit_->setEnabled( def_ok );

    //add entry widgets
    std::map<DB_OBJECT_TYPE,DBObject*>& dobs = DBObjectManager::getInstance().getDBObjects();
    DB_OBJECT_TYPE type;
    bool errors = false;
    const TransformationVariable::PropertyMap& props = variable_->getProperties();
    TransformationVariable::PropertyMap::const_iterator it, itend = props.end();
    for( it=props.begin(); it!=itend; ++it )
    {
        type = it->first;

        //Oo, DBO types of saved entries not found, skip item but display warning later
        if( dobs.find( type ) == dobs.end() )
        {
            errors = true;
            continue;
        }

        TransformationVariablePropertyEntry* entry = it->second;

        //create proper widget
        PropertyWidget* widget = new PropertyWidget( this );
        fillDataTypeCombo( widget->getDataTypeCombo() );
        widget->setDBOType( type );
        widget->setDataType( (PROPERTY_DATA_TYPE)entry->getProperty().data_type_int_ );
        widget->setPropertyName( QString::fromStdString( entry->getProperty().id_ ) );
        widget->setDBOVariable( entry->getVariable() );

        //add widget
        prop_layout_->addWidget( widget );
        widgets_[ type ] = widget;

        //catch delete requests from the widget
        connect( widget, SIGNAL(deleteMe()), this, SLOT(deletePropertySlot()) );
    }

    //update data type combos of widgets
    checkDataTypeCombos();

    //DBO type related errors
    if( errors )
    {
        QString msg = "Properties with non-present DBO type have been detected and skipped. By pressing ok these properties will be deleted.";
        QMessageBox::warning( this, "Warning", msg );
    }

    //connect stuff
    connect( dtype_combo_, SIGNAL(currentIndexChanged(int)), this, SLOT(dataTypeChangedSlot(int)) );
    connect( add_manual_button_, SIGNAL(pressed()), this, SLOT(addPropertyManualSlot()) );
    connect( add_dbo_button_, SIGNAL(pressed()), this, SLOT(addPropertyDBOSlot()) );
    connect( ok_button_, SIGNAL(pressed()), this, SLOT(okSlot()) );
    connect( cancel_button_, SIGNAL(pressed()), this, SLOT(cancelSlot()) );
    connect( default_box_, SIGNAL(toggled(bool)), this, SLOT(defaultIDEnabledSlot(bool)) );
}

/**
Fills a data type combo box.
@param combo Combo box to be filled.
@param add_notype Add an entry displaying 'NoType'.
  */
void TransformationVariableEditDialog::fillDataTypeCombo( QComboBox* combo, bool add_notype )
{
    combo->clear();
    std::map<PROPERTY_DATA_TYPE,std::string>::const_iterator it, itend = PROPERTY_DATA_TYPE_STRINGS.end();
    for( it=PROPERTY_DATA_TYPE_STRINGS.begin(); it!=itend; ++it )
        combo->addItem( QString::fromStdString( it->second ), QVariant( (int)it->first ) );

    if( add_notype )
        combo->addItem( "NoType", QVariant( (int)-1 ) );
}

/**
Fills a DBO type combo box.
@param combo Combo box to be filled.
  */
void TransformationVariableEditDialog::fillDBOTypeCombo( QComboBox* combo )
{
    combo->clear();

    DB_OBJECT_TYPE type;
    std::string typestr;

    //add all registered DBO types
    std::map <DB_OBJECT_TYPE,DBObject*>& dobs = DBObjectManager::getInstance().getDBObjects();
    std::map <DB_OBJECT_TYPE,DBObject*>::iterator it, itend = dobs.end();
    for( it=dobs.begin(); it!=itend; ++it )
    {
        type = it->second->getType();

        //skip undefined type
        if( type == DBO_UNDEFINED )
            continue;

        combo->addItem( QString::fromStdString( DB_OBJECT_TYPE_STRINGS[ type ] ), QVariant( (int)type ) );
    }
}

/**
Sets the variables data type using the given property data type.
@param dtype Property data type as integer number.
  */
void TransformationVariableEditDialog::setDataType( int dtype )
{
    int i, n = dtype_combo_->count();
    for( i=0; i<n; ++i )
    {
        if( dtype == dtype_combo_->itemData( i ).toInt() )
        {
            dtype_combo_->setCurrentIndex( i );
            return;
        }
    }
}

/**
Deletes the clicked property entry. Received from a Property widget.
  */
void TransformationVariableEditDialog::deletePropertySlot()
{
    PropertyWidget* widget = (PropertyWidget*)QObject::sender();
    widgets_.erase( widget->getDBOType() );
    widget->deleteLater();
}

/**
Triggered if the variables data type changes. Updates all Property widgets
data type combos accordingly.
@param idx New active entry.
  */
void TransformationVariableEditDialog::dataTypeChangedSlot( int idx )
{
    checkDataTypeCombos();
}

/**
Changes the data type combos of all Property widgets according to the variables data type combo.
  */
void TransformationVariableEditDialog::checkDataTypeCombos()
{
    int dtype = dtype_combo_->itemData( dtype_combo_->currentIndex() ).toInt();
    int i, n = prop_layout_->count();
    for( i=0; i<n; ++i )
    {
        PropertyWidget* widget = (PropertyWidget*)prop_layout_->itemAt( i )->widget();

        //only enable the widget if the variables type is deactivated
        widget->getDataTypeCombo()->setEnabled( dtype == -1 );
    }
}

/**
Adds a Property from the specified DBO variable.
  */
void TransformationVariableEditDialog::addPropertyDBOSlot()
{
    //no variable set
    if( !dbo_widget_->hasVariable() )
    {
        QMessageBox::warning( this, "Warning", "Please select a DBO variable." );
        return;
    }

    DBOVariable* var = dbo_widget_->getSelectedVariable();

    //use the temporary transformation var to read in the DBOVariable
    variable_tmp_.clearProperties();
    variable_tmp_.addProperty( var );

    std::map<DB_OBJECT_TYPE,DBObject*>& dobs = DBObjectManager::getInstance().getDBObjects();
    DB_OBJECT_TYPE type;
    bool errors = false;
    const TransformationVariable::PropertyMap& props = variable_tmp_.getProperties();
    TransformationVariable::PropertyMap::const_iterator it, itend = props.end();
    for( it=props.begin(); it!=itend; ++it )
    {
        type = it->first;

        //Oo, DBO types of saved entries not found, skip item but display warning later
        if( dobs.find( type ) == dobs.end() )
        {
            errors = true;
            continue;
        }

        TransformationVariablePropertyEntry* entry = it->second;

        //widget for this type is already present, update
        if( widgets_.find( type ) != widgets_.end() )
        {
            PropertyWidget* widget = widgets_[ type ];
            widget->setPropertyName( QString::fromStdString( entry->getProperty().id_ ) );
            widget->setDataType( (PROPERTY_DATA_TYPE)entry->getProperty().data_type_int_ );
            widget->setDBOVariable( entry->getVariable() );
            continue;
        }

        //create new Property widget
        PropertyWidget* widget = new PropertyWidget( this );
        fillDataTypeCombo( widget->getDataTypeCombo() );
        widget->setDBOType( type );
        widget->setDataType( (PROPERTY_DATA_TYPE)entry->getProperty().data_type_int_ );
        widget->setPropertyName( QString::fromStdString( entry->getProperty().id_ ) );
        widget->setDBOVariable( entry->getVariable() );

        //add widget
        prop_layout_->addWidget( widget );
        widgets_[ type ] = widget;

        //catch delete requests from the widget
        connect( widget, SIGNAL(deleteMe()), this, SLOT(deletePropertySlot()) );
    }

    //update data type combos of widgets
    checkDataTypeCombos();

    //DBO type related errors
    if( errors )
    {
        QString msg = "Properties with non-present DBO type have been detected and skipped. By pressing ok these properties will be deleted.";
        QMessageBox::warning( this, "Warning", msg );
    }
}

/**
Adds a Property from the provided manual information.
  */
void TransformationVariableEditDialog::addPropertyManualSlot()
{
    //we need a name, a DBO type and a data type
    DB_OBJECT_TYPE dbo_type = (DB_OBJECT_TYPE)prop_dbotype_combo_->itemData( prop_dbotype_combo_->currentIndex() ).toInt();
    PROPERTY_DATA_TYPE dtype = (PROPERTY_DATA_TYPE)prop_dtype_combo_->itemData( prop_dtype_combo_->currentIndex() ).toInt();
    QString name = prop_name_edit_->text();

    //no valid name
    if( name.isEmpty() )
    {
        QMessageBox::warning( this, "Warning", "Please enter a valid property name." );
        return;
    }

    //widget for this type is already present, update
    if( widgets_.find( dbo_type ) != widgets_.end() )
    {
        PropertyWidget* widget = widgets_[ dbo_type ];
        widget->setPropertyName( name );
        widget->setDataType( dtype );
        return;
    }

    //create new Property widget
    PropertyWidget* widget = new PropertyWidget( this );
    fillDataTypeCombo( widget->getDataTypeCombo() );
    widget->setDBOType( dbo_type );
    widget->setDataType( dtype );
    widget->setPropertyName( name );

    //add widget
    prop_layout_->addWidget( widget );
    widgets_[ dbo_type ] = widget;

    //clear name field, looks better
    prop_name_edit_->clear();

    //catch delete requests from the widget
    connect( widget, SIGNAL(deleteMe()), this, SLOT(deletePropertySlot()) );

    //update data type combos of widgets
    checkDataTypeCombos();
}

/**
Accepts the changes made in the dialog.
  */
void TransformationVariableEditDialog::okSlot()
{
    //check provided name
    QString name = name_edit_->text();
    if( name.isEmpty() )
    {
        QMessageBox::warning( this, "Warning", "Please enter a valid variable name." );
        return;
    }

    //check provided default id, if activated
    QString def_id = default_edit_->text();
    if( default_box_->isChecked() && def_id.isEmpty() )
    {
        QMessageBox::warning( this, "Warning", "Please enter a valid default id." );
        return;
    }

    int dtype = dtype_combo_->itemData( dtype_combo_->currentIndex() ).toInt();
    bool check_type = dtype != -1;
    bool def_enabled = default_box_->isChecked();

    //No default id without provided data type.
    /// @todo This is a restriction that may be unnecessary. The default properties data type should maybe be independent of the variables.
    if( !check_type && def_enabled )
    {
        QMessageBox::warning( this, "Warning", "Please provide a data type when using a default id." );
        return;
    }

    //precheck property names
    int i, n = prop_layout_->count();
    for( i=0; i<n; ++i )
    {
        PropertyWidget* widget = (PropertyWidget*)prop_layout_->itemAt( i )->widget();
        if( widget->getProperty().id_.empty() )
        {
            QMessageBox::warning( this, "Warning", "Please provide valid names for all yout properties." );
            return;
        }
    }

    //set data type
    variable_->enableDataTypeCheck( check_type );
    if( check_type )
        variable_->setDataType( (PROPERTY_DATA_TYPE)dtype );

    //set default property
    if( def_enabled )
        variable_->setDefaultProperty( def_id.toStdString() );  //data type set from variable data type internally
    else
        variable_->setDefaultProperty( NULL );

    //clear the variables entries
    variable_->clearProperties();

    //add new entries
    try{
        Property prop;
        for( i=0; i<n; ++i )
        {
            PropertyWidget* widget = (PropertyWidget*)prop_layout_->itemAt( i )->widget();
            prop = widget->getProperty();

            //if type check enabled, override the entries data type with the variables data type
            if( check_type )
                prop.data_type_int_ = dtype;

            variable_->addProperty( widget->getDBOType(), prop, widget->getDBOVariable() );
        }
    }
    catch( const std::runtime_error& err )
    {
        QString message = "Configuration resulted in an error:\n\n";
        message += QString( err.what() )+"\n\n";
        message += "Please check your filled in data.";
        QMessageBox::critical( NULL, "Error", message );
    }

    accept();
}

/**
Cancels the changes made in the dialog.
  */
void TransformationVariableEditDialog::cancelSlot()
{
    reject();
}

/**
Triggered when the default id has been enabled/disabled.
@param enabled True if enabled, false if disabled.
  */
void TransformationVariableEditDialog::defaultIDEnabledSlot( bool enabled )
{
    default_edit_->setEnabled( enabled );
}
