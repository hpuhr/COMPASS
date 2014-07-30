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


#ifndef FILTEREDITWIDGET_H
#define FILTEREDITWIDGET_H

#include "BufferFilter.h"

#include "ui_FilterEditDialogBase.h"

#include <QDialog>

class QLabel;


/**
@brief Displays a buffer filter rule in a FilterEditDialog.

Used to display and edit a filter rule of a BufferFilter.
Such a filter rule is also assigned a specific DBO type and for
this type provides a rule how to treat an incoming Buffer.
  */
class FilterRuleWidget : public QWidget
{
    Q_OBJECT
public:
    /// @brief Constructor
    FilterRuleWidget( DB_OBJECT_TYPE dbo_type, QWidget* parent=NULL );
    /// @brief Destructor
    ~FilterRuleWidget();

    /// @brief Returns the widgets rule combo
    QComboBox* getRuleCombo();

    /// @brief Sets the widgets filter rule
    void setRule( BufferFilter::BufferFilterRule rule );
    /// @brief Returns the widgets filter rule
    BufferFilter::BufferFilterRule getRule() const;
    /// @brief Returns the DBO type assigned to the widget
    DB_OBJECT_TYPE getDBOType() const;

signals:
    /// @brief Sends a delete request
    void deleteMe();

private:
    QLabel* dbo_label_;
    QComboBox* rule_combo_;
    QPushButton* delete_button_;

    /// DBO type assigned to the widget
    DB_OBJECT_TYPE dbo_type_;
};

/**
@brief Displays a buffer filter Property rule in a FilterEditDialog.

Used to display and edit a Property rule of a Buffer filter.
Such a rule consists of a DBO type and a Property. It is used to
precheck the existence of specific properties in an incoming Buffer.
  */
class PropertyRuleWidget : public QWidget
{
    Q_OBJECT
public:
    /// @brief Constructor
    PropertyRuleWidget( QWidget* parent=NULL );
    /// @brief Destructor
    ~PropertyRuleWidget();

    /// @brief Returns the widgets data type combo
    QComboBox* getDataTypeCombo();

    /// @brief Returns the widgets Property
    Property getProperty() const;
    /// @brief Returns the DBO type assigned to the widget
    DB_OBJECT_TYPE getDBOType() const;

    /// @brief Sets the DBO type assigned to the widget
    void setDBOType( DB_OBJECT_TYPE dbo_type );
    /// @brief Sets the properties data type
    void setDataType( PROPERTY_DATA_TYPE dtype );
    /// @brief Sets the properties string id
    void setPropertyName( const QString& name );

signals:
    /// @brief Sends a delete request
    void deleteMe();

private:
    QLineEdit* name_edit_;
    QLineEdit* dbo_edit_;
    QComboBox* dtype_combo_;
    QPushButton* delete_button_;

    /// DBO type assigned to the widget
    DB_OBJECT_TYPE dbo_type_;
};

/**
@brief Shows a dialog to edit a BufferFilter.

This dialog can be used to edit a BufferFilter. One can edit its filter rules
and property entries.

Note that only by pressing the Ok-button the changes will be applied to the filter.
Pressing the Cancel-button will discard all changes.
  */
class FilterEditDialog : public QDialog, private Ui::FilterEditDialogBase
{
    Q_OBJECT
public:
    typedef std::map<DB_OBJECT_TYPE,FilterRuleWidget*> RuleWidgets;
    typedef std::map<std::string,PropertyRuleWidget*> PropertyWidgetsMap;
    typedef std::map<DB_OBJECT_TYPE,PropertyWidgetsMap> PropertyWidgets;

    /// @brief Constructor
    FilterEditDialog( BufferFilter* filter, QWidget* parent=NULL );
    /// @brief Destructor
    virtual ~FilterEditDialog();

protected slots:
    /// @brief Adds a new filter rule
    void addRuleSlot();
    /// @brief Deletes the clicked filter rule
    void deleteRuleSlot();
    /// @brief Triggered if the base rule checkbox has changed
    void enableBaseRuleSlot( bool enabled );

    /// @brief Adds a filter Property from a DBO variable
    void addPropertyFromVarSlot();
    /// @brief Adds a filter Property from manually provided information
    void addPropertyManualSlot();
    /// @brief Deletes the clicked filter Property
    void deletePropertySlot();

    /// @brief Accepts the changes made in the dialog
    void okSlot();
    /// @brief Cancels the changes made in the dialog
    void cancelSlot();

protected:
    /// @brief Inits the dialog
    void init();
    /// @brief Fills a filter rule combo
    void fillRuleCombo( QComboBox* box );
    /// @brief Fills a DBO type combo
    void fillDBOTypeCombo( QComboBox* box );
    /// @brief Fills a data type combo
    void fillDataTypeCombo( QComboBox* box );

    /// @brief Updates the rule entries from the filter
    void updateRuleEntries();
    /// @brief Updates the Property entries from the filter
    void updatePropertyEntries();

    /// The edited filter
    BufferFilter* filter_;
    /// The filter rule widgets
    RuleWidgets rule_widgets_;
    /// The Property rule widgets
    PropertyWidgets prop_widgets_;
    /// A temporary filter
    BufferFilter filter_tmp_;
};

#endif //FILTEREDITWIDGET_H
