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


#include "FilterEditDialog.h"
#include "DBObjectManager.h"
#include "DBObject.h"
#include "ATSDB.h"

#include <QLabel>
#include <QMessageBox>
#include <QLineEdit>


/**********************************************************************************
FilterRuleWidget
***********************************************************************************/

/**
Constructor.
@param dbo_type DBO type assigned to the widget.
@param parent Parent widget.
  */
FilterRuleWidget::FilterRuleWidget( const std::string &dbo_type, QWidget* parent )
:   QWidget( parent ),
    dbo_type_( dbo_type )
{
    QHBoxLayout* layout = new QHBoxLayout;
    dbo_label_ = new QLabel( QString::fromStdString( dbo_type ), this );
    rule_combo_ = new QComboBox( this );
    QPushButton* button = new QPushButton( this );
    button->setText( "Delete" );

    layout->addWidget( dbo_label_ );
    layout->addWidget( rule_combo_ );
    layout->addWidget( button );

    setLayout( layout );

    dbo_label_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    rule_combo_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    button->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

    connect( button, SIGNAL(pressed()), this, SIGNAL(deleteMe()) );
}

/**
Destructor.
  */
FilterRuleWidget::~FilterRuleWidget()
{
}

/**
Returns the widgets rule combo box.
@return The rule combo box.
  */
QComboBox* FilterRuleWidget::getRuleCombo()
{
    return rule_combo_;
}

/**
Returns the widgets filter rule.
@return The current filter rule.
  */
BufferFilter::BufferFilterRule FilterRuleWidget::getRule() const
{
    return (BufferFilter::BufferFilterRule)(rule_combo_->itemData( rule_combo_->currentIndex() ).toInt());
}

/**
Returns the DBO type assigned to the widget.
@return The widgets DBO type.
  */
const std::string &FilterRuleWidget::getDBOType() const
{
    return dbo_type_;
}

/**
Sets the widgets filter rule.
@param rule New filter rule.
  */
void FilterRuleWidget::setRule( BufferFilter::BufferFilterRule rule )
{
    rule_combo_->setCurrentIndex( (int)rule );
}

/**********************************************************************************
PropertyRuleWidget
***********************************************************************************/

/**
Constructor.
@param parent Parent widget.
  */
PropertyRuleWidget::PropertyRuleWidget( QWidget* parent )
:   QWidget( parent )
{
    QHBoxLayout* layout = new QHBoxLayout;

    dbo_edit_ = new QLineEdit( this );
    name_edit_ = new QLineEdit( this );
    dtype_combo_ = new QComboBox( this );
    QPushButton* button = new QPushButton( this );
    button->setText( "Delete" );

    layout->addWidget( dbo_edit_ );
    layout->addWidget( name_edit_ );
    layout->addWidget( dtype_combo_ );
    layout->addWidget( button );

    setLayout( layout );

    dbo_edit_->setReadOnly( true );

    name_edit_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    dbo_edit_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    dtype_combo_->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    button->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

    connect( button, SIGNAL(pressed()), this, SIGNAL(deleteMe()) );
}

/**
Destructor.
  */
PropertyRuleWidget::~PropertyRuleWidget()
{
}

/**
Returns the widgets data type combo.
@return The data type combo box.
  */
QComboBox* PropertyRuleWidget::getDataTypeCombo()
{
    return dtype_combo_;
}

/**
Returns the widgets Property.
@return The current Property.
  */
Property PropertyRuleWidget::getProperty() const
{
    const QString& name = name_edit_->text();
    //PROPERTY_DATA_TYPE dtype = (PROPERTY_DATA_TYPE)(dtype_combo_->itemData( dtype_combo_->currentIndex() ).toInt());
    // TODO FIX WIDGET
    assert (false);

    PropertyDataType dtype = PropertyDataType::STRING;

    return Property( name.toStdString(), dtype );
}

/**
Returns the DBO type assigned to the widget.
@return DBO type assigned to the widget.
  */
const std::string &PropertyRuleWidget::getDBOType() const
{
    return dbo_type_;
}

/**
Sets the DBO type assigned to the widget.
@param dbo_type The new DBO type.
  */
void PropertyRuleWidget::setDBOType( const std::string &dbo_type )
{
    dbo_type_ = dbo_type;
    dbo_edit_->setText( QString::fromStdString( dbo_type ) );
}

/**
Sets the properties data type.
@param dtype New data type.
  */
void PropertyRuleWidget::setDataType( PropertyDataType dtype )
{
    // TODO FIX WIDGET
    assert (false);

//    PROPERTY_DATA_TYPE type;
//    int i, n = dtype_combo_->count();
//    for( i=0; i<n; ++i )
//    {
//        type = (PROPERTY_DATA_TYPE)(dtype_combo_->itemData( i ).toInt());
//        if( dtype == type )
//        {
//            dtype_combo_->setCurrentIndex( i );
//            break;
//        }
//    }
}

/**
Sets the properties string id.
@param name New string id.
  */
void PropertyRuleWidget::setPropertyName( const QString& name )
{
    name_edit_->setText( name );
}

/**********************************************************************************
FilterEditDialog
***********************************************************************************/

/**
Constructor.
@param filter Buffer filter to edit.
@param parent Parent widget.
  */
FilterEditDialog::FilterEditDialog( BufferFilter* filter, QWidget* parent )
:   QDialog( parent ),
    filter_( filter )
{
    setupUi( this );

    assert( filter_ );

    init();
}

/**
Destructor.
  */
FilterEditDialog::~FilterEditDialog()
{
}

/**
Inits the dialog.
  */
void FilterEditDialog::init()
{
    connect( ok_button_, SIGNAL(pressed()), this, SLOT(okSlot()) );
    connect( cancel_button_, SIGNAL(pressed()), this, SLOT(cancelSlot()) );

    //RULES TAB

    //fill the combos
    fillRuleCombo( base_rule_combo_ );
    fillDBOTypeCombo( rule_dbo_combo_ );
    fillRuleCombo( rule_combo_ );

    //setup base rule
    base_rule_check_->setChecked( filter_->hasBaseRule() );
    base_rule_combo_->setEnabled( filter_->hasBaseRule() );
    if( filter_->hasBaseRule() )
        base_rule_combo_->setCurrentIndex( (int)filter_->getBaseRule() );
    else
        base_rule_combo_->setCurrentIndex( (int)BufferFilter::BLOCK );

    connect( rule_add_button_, SIGNAL(pressed()), this, SLOT(addRuleSlot()) );
    connect( base_rule_check_, SIGNAL(toggled(bool)), this, SLOT(enableBaseRuleSlot(bool)) );

    //add rule entries
    updateRuleEntries();

    //PROP TAB

    //fill the combos
    fillDBOTypeCombo( prop_dbotype_combo_ );
    fillDataTypeCombo( prop_datatype_combo_ );

    connect( add_dbo_button_, SIGNAL(pressed()), this, SLOT(addPropertyFromVarSlot()) );
    connect( add_manual_button_, SIGNAL(pressed()), this, SLOT(addPropertyManualSlot()) );

    //add properties
    updatePropertyEntries();
}

/**
Fills a filter rule combo.
@param box Combo box to be filled.
  */
void FilterEditDialog::fillRuleCombo( QComboBox* box )
{
    box->clear();
    box->addItem( "FORWARD", QVariant( (int)BufferFilter::FORWARD ) );
    box->addItem( "BLOCK", QVariant( (int)BufferFilter::BLOCK ) );
    box->addItem( "TRANSFORM", QVariant( (int)BufferFilter::TRANSFORM ) );
}

/**
Fills a DBO type combo.
@param box Combo box to be filled.
  */
void FilterEditDialog::fillDBOTypeCombo( QComboBox* box )
{
    box->clear();

    std::string dbo_type;

    const std::map <std::string,DBObject*>& dobs = DBObjectManager::getInstance().getDBObjects();
    std::map <std::string,DBObject*>::const_iterator it, itend = dobs.end();

    for( it=dobs.begin(); it!=itend; ++it )
    {
        dbo_type = it->second->getType();

//        if( type == DBO_UNDEFINED )
//            continue;
        //TODO FIX WIDGET
        assert (false);

        //box->addItem( QString::fromStdString( dbo_type ), QVariant( QString(dbo_type) ) );
    }
}

/**
Fills a data type combo.
@param box Combo box to be filled.
  */
void FilterEditDialog::fillDataTypeCombo( QComboBox* box )
{
    //TODO FIX WIDGET
    assert (false);

//    box->clear();
//    box->addItem( "BOOL", QVariant( (int)P_TYPE_BOOL ) );
//    box->addItem( "CHAR", QVariant( (int)P_TYPE_CHAR ) );
//    box->addItem( "INT", QVariant( (int)P_TYPE_INT ) );
//    box->addItem( "UCHAR", QVariant( (int)P_TYPE_UCHAR ) );
//    box->addItem( "UINT", QVariant( (int)P_TYPE_UINT ) );
//    box->addItem( "STRING", QVariant( (int)P_TYPE_STRING ) );
//    box->addItem( "FLOAT", QVariant( (int)P_TYPE_FLOAT ) );
//    box->addItem( "DOUBLE", QVariant( (int)P_TYPE_DOUBLE ) );
//    box->addItem( "POINTER", QVariant( (int)P_TYPE_POINTER ) );
}

/**
Updates the rule entries from the filter.
  */
void FilterEditDialog::updateRuleEntries()
{
    //delete old rule widgets
    {
        RuleWidgets::iterator it ,itend = rule_widgets_.end();
        for( it=rule_widgets_.begin(); it!=itend; ++it )
            delete it->second;
        rule_widgets_.clear();
    }

    //read rule widgets from the filter
    {
        const std::map<std::string,DBObject*>& dobs = DBObjectManager::getInstance().getDBObjects();
        std::string dbo_type;
        bool errors = false;
        std::map<std::string,BufferFilter::BufferFilterRule> rules = filter_->getRules();
        std::map<std::string,BufferFilter::BufferFilterRule>::const_iterator it, itend = rules.end();
        for( it=rules.begin(); it!=itend; ++it )
        {
            dbo_type = it->first;

            //DBO type not found, skip and show error later
            if( dobs.find( dbo_type ) == dobs.end() )
            {
                errors = true;
                continue;
            }

            //create widget
            FilterRuleWidget* widget = new FilterRuleWidget( dbo_type, this );
            fillRuleCombo( widget->getRuleCombo() );
            widget->setRule( it->second );

            //add widget
            rule_layout_->addWidget( widget );
            rule_widgets_[ dbo_type ] = widget;

            connect( widget, SIGNAL(deleteMe()), this, SLOT(deleteRuleSlot()) );
        }

        //DBO type related error
        if( errors )
        {
            QString msg = "Rules with non-present DBO type have been detected and skipped. By pressing ok these rules will be deleted.";
            QMessageBox::warning( this, "Warning", msg );
        }
    }
}

/**
Updates the Property entries from the filter.
  */
void FilterEditDialog::updatePropertyEntries()
{
    //delete old property widgets
    {
        PropertyWidgetsMap::iterator itm, itmend;
        PropertyWidgets::iterator it ,itend = prop_widgets_.end();
        for( it=prop_widgets_.begin(); it!=itend; ++it )
        {
            PropertyWidgetsMap& pmap = it->second;
            itm = pmap.begin();
            itmend = pmap.end();
            for( ; itm!=itmend; ++itm )
                delete itm->second;
        }
        prop_widgets_.clear();
    }

    //read new properties from the filter
    {
        const std::map<std::string,DBObject*>& dobs = DBObjectManager::getInstance().getDBObjects();
        std::string dbo_type;
        bool errors = false;
        std::multimap<std::string,Property> props = filter_->getProperties();
        std::multimap<std::string,Property>::const_iterator it, itend = props.end();
        for( it=props.begin(); it!=itend; ++it )
        {
            dbo_type = it->first;

            //DBO type not found, skip and show error later
            if( dobs.find( dbo_type ) == dobs.end() )
            {
                errors = true;
                continue;
            }

            //create widget
            PropertyRuleWidget* widget = new PropertyRuleWidget( this );
            fillDataTypeCombo( widget->getDataTypeCombo() );
            widget->setDBOType( dbo_type );
            widget->setDataType( it->second.getDataType() );
            widget->setPropertyName( QString::fromStdString( it->second.getId() ) );

            //add widget
            prop_layout_->addWidget( widget );
            prop_widgets_[ dbo_type ][ it->second.getId() ] = widget;

            connect( widget, SIGNAL(deleteMe()), this, SLOT(deletePropertySlot()) );
        }

        //DBO type related error
        if( errors )
        {
            QString msg = "Rules with non-present DBO type have been detected and skipped. By pressing ok these rules will be deleted.";
            QMessageBox::warning( this, "Warning", msg );
        }
    }
}

/**
Adds a new filter rule.
Rule widgets that are already present for some DBO type will just get updated.
  */
void FilterEditDialog::addRuleSlot()
{
    // TODO FIX WIDGET
    assert (false);

//    //get data
//    int dbo_type_int = rule_dbo_combo_->itemData( rule_dbo_combo_->currentIndex() ).toInt();
//    DB_OBJECT_TYPE type = (DB_OBJECT_TYPE)dbo_type_int;

//    //widget already present
//    if( rule_widgets_.find( type ) != rule_widgets_.end() )
//    {
//        rule_widgets_[ type ]->getRuleCombo()->setCurrentIndex( rule_combo_->currentIndex() );
//        return;
//    }

//    //create widget
//    FilterRuleWidget* widget = new FilterRuleWidget( type, this );
//    fillRuleCombo( widget->getRuleCombo() );
//    widget->getRuleCombo()->setCurrentIndex( rule_combo_->currentIndex() );

//    //add widget
//    rule_widgets_[ type ] = widget;
//    rule_layout_->addWidget( widget );

//    connect( widget, SIGNAL(deleteMe()), this, SLOT(deleteRuleSlot()) );
}

/**
Adds a filter Property from a DBO variable.
  */
void FilterEditDialog::addPropertyFromVarSlot()
{
    //no variable set
    if( !dbo_widget_->hasVariable() )
    {
        QMessageBox::warning( this, "Warning", "Please provide a valid DBO variable." );
        return;
    }

    DBOVariable* var = dbo_widget_->getSelectedVariable();

    //use a temporary filter to read in DBO var
    filter_tmp_.clear();
    filter_tmp_.addPropertyToFilter( var );

    //read properties from filter
    {
        const std::map<std::string,DBObject*>& dobs = DBObjectManager::getInstance().getDBObjects();
        std::string dbo_type;
        PropertyDataType dtype;
        std::string name;
        bool errors = false;
        std::multimap<std::string,Property> props = filter_tmp_.getProperties();
        std::multimap<std::string,Property>::const_iterator it, itend = props.end();
        for( it=props.begin(); it!=itend; ++it )
        {
            dbo_type = it->first;
            dtype = it->second.getDataType();
            name = it->second.getId();

            //DBO type not found, skip property and show error later
            if( dobs.find( dbo_type ) == dobs.end() )
            {
                errors = true;
                continue;
            }

            //widget already present
            if( prop_widgets_.find( dbo_type ) != prop_widgets_.end() &&
                prop_widgets_[ dbo_type ].find( name ) != prop_widgets_[ dbo_type ].end() )
            {
                PropertyRuleWidget* widget = prop_widgets_[ dbo_type ][ name ];
                widget->setDataType( dtype );
                widget->setDBOType( dbo_type );
                widget->setPropertyName( QString::fromStdString( name ) );
                continue;
            }

            //create widget
            PropertyRuleWidget* widget = new PropertyRuleWidget( this );
            fillDataTypeCombo( widget->getDataTypeCombo() );
            widget->setDataType( dtype );
            widget->setDBOType( dbo_type );
            widget->setPropertyName( QString::fromStdString( name ) );

            //add widget
            prop_widgets_[ dbo_type ][ name ] = widget;
            prop_layout_->addWidget( widget );

            connect( widget, SIGNAL(deleteMe()), this, SLOT(deleteRuleSlot()) );
        }

        //DBO type related errors
        if( errors )
        {
            QString msg = "Rules with non-present DBO type have been detected and skipped. By pressing ok these rules will be deleted.";
            QMessageBox::warning( this, "Warning", msg );
        }
    }
}

/**
Adds a filter Property from manually provided information.
  */
void FilterEditDialog::addPropertyManualSlot()
{
    // TODO FIX WIDGET
    assert (false);

//    //get data
//    int dbo_type_int = prop_dbotype_combo_->itemData( prop_dbotype_combo_->currentIndex() ).toInt();
//    DB_OBJECT_TYPE dbo_type = (DB_OBJECT_TYPE)dbo_type_int;

//    int dtype_int = prop_datatype_combo_->itemData( prop_datatype_combo_->currentIndex() ).toInt();
//    PROPERTY_DATA_TYPE dtype = (PROPERTY_DATA_TYPE)dtype_int;

//    std::string name = manual_prop_edit_->text().toStdString();

//    //name not valid
//    if( name.empty() )
//    {
//        QMessageBox::warning( this, "Warning", "Please provide a valid property name." );
//        return;
//    }

//    //widget already present
//    if( prop_widgets_.find( dbo_type ) != prop_widgets_.end() &&
//        prop_widgets_[ dbo_type ].find( name ) != prop_widgets_[ dbo_type ].end() )
//    {
//        PropertyRuleWidget* widget = prop_widgets_[ dbo_type ][ name ];
//        widget->setDataType( dtype );
//        widget->setDBOType( dbo_type );
//        widget->setPropertyName( QString::fromStdString( name ) );
//        return;
//    }

//    //create widget
//    PropertyRuleWidget* widget = new PropertyRuleWidget( this );
//    fillDataTypeCombo( widget->getDataTypeCombo() );
//    widget->setDataType( dtype );
//    widget->setDBOType( dbo_type );
//    widget->setPropertyName( QString::fromStdString( name ) );

//    //add widget
//    prop_layout_->addWidget( widget );
//    prop_widgets_[ dbo_type ][ name ] = widget;

//    manual_prop_edit_->clear();

//    connect( widget, SIGNAL(deleteMe()), this, SLOT(deleteRuleSlot()) );
}

/**
Deletes the clicked filter rule.
  */
void FilterEditDialog::deleteRuleSlot()
{
    FilterRuleWidget* widget = (FilterRuleWidget*)(QObject::sender());
    rule_widgets_.erase( widget->getDBOType() );
    widget->deleteLater();
}

/**
Triggered if the base rule checkbox has changed.
@param enabled Indicates if the base rule has been enabled/disabled.
  */
void FilterEditDialog::enableBaseRuleSlot( bool enabled )
{
    base_rule_combo_->setEnabled( enabled );
}

/**
Deletes the clicked filter Property.
  */
void FilterEditDialog::deletePropertySlot()
{
    PropertyRuleWidget* widget = (PropertyRuleWidget*)(QObject::sender());
    const std::string &dbo_type = widget->getDBOType();
    prop_widgets_[ dbo_type ].erase( widget->getProperty().getId() );

    if( prop_widgets_[ dbo_type ].empty() )
        prop_widgets_.erase( dbo_type );

    widget->deleteLater();
}

/**
Accepts the changes made in the dialog.
  */
void FilterEditDialog::okSlot()
{
    //clear filter
    filter_->clear();

    //set base rule
    if( base_rule_check_->isChecked() )
    {
        int rule_int = base_rule_combo_->itemData( base_rule_combo_->currentIndex() ).toInt();
        filter_->setBaseRule( (BufferFilter::BufferFilterRule)rule_int );
    }

    //add specific rules
    {
        RuleWidgets::iterator it ,itend = rule_widgets_.end();
        for( it=rule_widgets_.begin(); it!=itend; ++it )
        {
            filter_->setRule( it->first, it->second->getRule() );
        }
    }

    //add properties
    {
        PropertyWidgetsMap::iterator itm, itmend;
        PropertyWidgets::iterator it ,itend = prop_widgets_.end();
        for( it=prop_widgets_.begin(); it!=itend; ++it )
        {
            PropertyWidgetsMap& pmap = it->second;
            itm = pmap.begin();
            itmend = pmap.end();
            for( ; itm!=itmend; ++itm )
            {
                filter_->addPropertyToFilter( it->first, itm->second->getProperty() );
            }
        }
    }

    accept();
}

/**
Cancels the changes made in the dialog.
  */
void FilterEditDialog::cancelSlot()
{
    reject();
}
