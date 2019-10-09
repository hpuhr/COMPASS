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

#include "jsonobjectparserwidget.h"
#include "jsonobjectparser.h"
#include "logger.h"
#include "files.h"
#include "configuration.h"
#include "dbovariableselectionwidget.h"
#include "datatypeformatselectionwidget.h"
#include "unitselectionwidget.h"
#include "dbovariable.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QMessageBox>

using namespace Utils;

JSONObjectParserWidget::JSONObjectParserWidget(JSONObjectParser& parser, QWidget *parent)
    : QWidget(parent), parser_ (&parser)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    std::string tmp ="JSON Object Parser " + parser_->dbObjectName();
    QLabel *main_label = new QLabel (tmp.c_str());
    main_label->setFont (font_bold);
    main_layout->addWidget (main_label);

    QGridLayout* grid = new QGridLayout ();

    int row = 0;

    grid->addWidget(new QLabel("JSON Container Key"), row, 0);

    json_container_key_edit_ = new QLineEdit ();
    connect(json_container_key_edit_, SIGNAL(textEdited(const QString&)), this, SLOT(jsonContainerKeyChangedSlot()));
    grid->addWidget(json_container_key_edit_, row++, 1);

    grid->addWidget(new QLabel("JSON Key"));

    json_key_edit_ = new QLineEdit ();
    connect(json_key_edit_, SIGNAL(textEdited(const QString&)), this, SLOT(jsonKeyChangedSlot()));
    grid->addWidget(json_key_edit_, row++, 1);

    grid->addWidget(new QLabel("JSON Value"));

    json_value_edit_ = new QLineEdit ();
    connect(json_value_edit_, SIGNAL(textEdited(const QString&)), this, SLOT(jsonValueChangedSlot()));
    grid->addWidget(json_value_edit_, row++, 1);

    grid->addWidget(new QLabel("Override Data Source"));

    override_data_source_check_ = new QCheckBox ();
    connect(override_data_source_check_, SIGNAL(stateChanged(int)), this, SLOT(overrideDataSourceChangedSlot()));
    grid->addWidget(override_data_source_check_, row++, 1);

    grid->addWidget(new QLabel("Data Source Variable"));

    data_source_variable_name_edit_ = new QLineEdit ();
    connect(data_source_variable_name_edit_, SIGNAL(textEdited(const QString&)),
            this, SLOT(dataSourceVariableChangedSlot()));
    grid->addWidget(data_source_variable_name_edit_, row++, 1);

    main_layout->addLayout(grid);

    QScrollArea* scroll_area = new QScrollArea();
    scroll_area->setWidgetResizable (true);

    mappings_grid_ = new QGridLayout ();
    updateMappingsGrid ();

    QWidget* scroll_widget = new QWidget ();
    scroll_widget->setLayout(mappings_grid_);

    scroll_area->setWidget(scroll_widget);
    main_layout->addWidget(scroll_area, 1);

    main_layout->addStretch();

    QPushButton* add_file_button = new QPushButton ("Add");
    connect (add_file_button, SIGNAL(clicked()), this, SLOT(addNewMappingSlot()));
    main_layout->addWidget(add_file_button);

    setLayout (main_layout);

    update ();

    show();
}

void JSONObjectParserWidget::update ()
{
    assert (parser_);
    assert (json_container_key_edit_);
    assert (json_key_edit_);
    assert (json_value_edit_);
    assert (override_data_source_check_);
    assert (data_source_variable_name_edit_);

    json_container_key_edit_->setText(parser_->JSONContainerKey().c_str());
    json_key_edit_->setText(parser_->JSONKey().c_str());
    json_value_edit_->setText(parser_->JSONValue().c_str());

    data_source_variable_name_edit_->setText(parser_->dataSourceVariableName().c_str());
}

void JSONObjectParserWidget::updateMappingsGrid()
{
    loginf  << "JSONObjectParserWidget: updateMappingsGrid";
    assert (parser_);
    assert (mappings_grid_);

    QLayoutItem *child;
    while ((child = mappings_grid_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    //format_selections_.clear();

    QFont font_bold;
    font_bold.setBold(true);

    QIcon del_icon(Files::getIconFilepath("delete.png").c_str());

    unsigned int row=0;

    QLabel *active_label = new QLabel ("Active");
    active_label->setFont (font_bold);
    mappings_grid_->addWidget (active_label, row, 0);

    QLabel *key_label = new QLabel ("JSON Key");
    key_label->setFont (font_bold);
    mappings_grid_->addWidget (key_label, row, 1);

    QLabel *comment_label = new QLabel ("Comment");
    comment_label->setFont (font_bold);
    mappings_grid_->addWidget (comment_label, row, 2);

    QLabel *dbovar_label = new QLabel ("DBOVariable");
    dbovar_label->setFont (font_bold);
    mappings_grid_->addWidget (dbovar_label, row, 3);

    QLabel *mandatory_label = new QLabel ("Mandatory");
    mandatory_label->setFont (font_bold);
    mappings_grid_->addWidget (mandatory_label, row, 4);

    QLabel *unit_label = new QLabel ("Unit");
    unit_label->setFont (font_bold);
    mappings_grid_->addWidget (unit_label, row, 5);

    QLabel *format_label = new QLabel ("Format");
    format_label->setFont (font_bold);
    mappings_grid_->addWidget (format_label, row, 6);

    ++row;

    std::multimap<std::string, std::pair<unsigned int,JSONDataMapping*>> sorted_mappings;

    unsigned int index = 0;
    for (auto& map_it : *parser_)
    {
        map_it.initializeIfRequired();
        sorted_mappings.insert({map_it.jsonKey(), {index, &map_it}});
        ++index;
        //loginf << "UGA insert " << map_it.jsonKey();
    }


    for (auto& map_it : sorted_mappings)
    {
        QVariant data = QVariant::fromValue(map_it.second.second); // JSONDataMapping* as QVariant

        QCheckBox* active_check = new QCheckBox ();
        active_check->setChecked(map_it.second.second->active());
        connect(active_check, SIGNAL(stateChanged(int)), this, SLOT(mappingActiveChangedSlot()));
        active_check->setProperty("mapping", data);
        mappings_grid_->addWidget(active_check, row, 0);

        QLineEdit* key_edit = new QLineEdit (map_it.first.c_str());
        connect(key_edit, SIGNAL(textEdited(const QString&)), this, SLOT(mappingKeyChangedSlot()));
        key_edit->setProperty("mapping", data);
        mappings_grid_->addWidget(key_edit, row, 1);

        QLineEdit* comment_edit = new QLineEdit (map_it.second.second->comment().c_str());
        connect(comment_edit, SIGNAL(textEdited(const QString&)), this, SLOT(mappingCommentChangedSlot()));
        comment_edit->setProperty("mapping", data);
        mappings_grid_->addWidget(comment_edit, row, 2);


        DBOVariableSelectionWidget* var_sel = new DBOVariableSelectionWidget ();
        var_sel->showMetaVariables(false);
        var_sel->showDBOOnly (map_it.second.second->dbObjectName());
        var_sel->showEmptyVariable(true);
        if (map_it.second.second->hasVariable())
            var_sel->selectedVariable(map_it.second.second->variable());
        connect(var_sel, SIGNAL(selectionChanged()), this, SLOT(mappingDBOVariableChangedSlot()));
        var_sel->setProperty("mapping", data);
        //var_sel->setProperty("row", row);
        mappings_grid_->addWidget(var_sel, row, 3);

        QCheckBox* mandatory_check = new QCheckBox ();
        mandatory_check->setChecked(map_it.second.second->mandatory());
        connect(mandatory_check, SIGNAL(stateChanged(int)), this, SLOT(mappingMandatoryChangedSlot()));
        mandatory_check->setProperty("mapping", data);
        mappings_grid_->addWidget(mandatory_check, row, 4);

        UnitSelectionWidget* unit_sel = new UnitSelectionWidget(map_it.second.second->dimensionRef(),
                                                                map_it.second.second->unitRef());
        mappings_grid_->addWidget (unit_sel, row, 5);
        //column_unit_selection_widgets_[unit_widget] = it.second;

//        if (map_it.hasVariable())
//        {
            DataTypeFormatSelectionWidget* data_format_widget
                    = new DataTypeFormatSelectionWidget (map_it.second.second->formatDataTypeRef(),
                                                         map_it.second.second->jsonValueFormatRef());

            mappings_grid_->addWidget (data_format_widget, row, 6);
//            format_selections_[row-1] = data_format_widget;
//        }

        QPushButton *del = new QPushButton ();
        del->setIcon(del_icon);
        del->setFixedSize ( UI_ICON_SIZE );
        del->setFlat(UI_ICON_BUTTON_FLAT);
        connect(del, SIGNAL(clicked()), this, SLOT(mappingDeleteSlot()));
        del->setProperty("mapping", data);
        del->setProperty("index", map_it.second.first);
        mappings_grid_->addWidget (del, row, 7);

        row++;
    }
}

void JSONObjectParserWidget::setParser (JSONObjectParser& parser)
{
    parser_ = &parser;
}

void JSONObjectParserWidget::jsonContainerKeyChangedSlot()
{
    assert (parser_);
    assert (json_container_key_edit_);

    parser_->JSONContainerKey(json_container_key_edit_->text().toStdString());
}

void JSONObjectParserWidget::jsonKeyChangedSlot ()
{
    assert (parser_);
    assert (json_key_edit_);

    parser_->JSONKey(json_key_edit_->text().toStdString());
}

void JSONObjectParserWidget::jsonValueChangedSlot ()
{
    assert (parser_);
    assert (json_value_edit_);

    parser_->JSONValue(json_value_edit_->text().toStdString());
}

void JSONObjectParserWidget::overrideDataSourceChangedSlot ()
{
    assert (parser_);
    assert (override_data_source_check_);

    parser_->overrideDataSource(override_data_source_check_->isChecked());
}

void JSONObjectParserWidget::dataSourceVariableChangedSlot ()
{
    assert (parser_);
    assert (data_source_variable_name_edit_);

    parser_->dataSourceVariableName(data_source_variable_name_edit_->text().toStdString());
}

void JSONObjectParserWidget::addNewMappingSlot()
{
    assert (parser_);

    Configuration &new_cfg = parser_->configuration().addNewSubConfiguration ("JSONDataMapping");
    new_cfg.addParameterString ("json_key", new_cfg.getInstanceId());
    new_cfg.addParameterString ("db_object_name", parser_->dbObjectName());

    parser_->generateSubConfigurable("JSONDataMapping", new_cfg.getInstanceId());

    updateMappingsGrid();
}

void JSONObjectParserWidget::mappingActiveChangedSlot()
{
    loginf << "JSONObjectParserWidget: mappingActiveChangedSlot";

    QCheckBox* widget = static_cast<QCheckBox*>(sender());
    assert (widget);
    QVariant data = widget->property("mapping");

    JSONDataMapping* mapping = data.value<JSONDataMapping*>();
    assert (mapping);

    if (mapping->mandatory() && widget->checkState() != Qt::Checked)
    {
        QMessageBox m_warning (QMessageBox::Warning, "Deactivation failed",
                               "Deactivation of mandatory variables is not allowed.",
                               QMessageBox::Ok);

        m_warning.exec();

        widget->setChecked(true);

        return;
    }

    parser_->setMappingActive(*mapping, widget->checkState() == Qt::Checked);
}

void JSONObjectParserWidget::mappingKeyChangedSlot()
{
    loginf << "JSONObjectParserWidget: mappingKeyChangedSlot";

    QLineEdit* widget = static_cast<QLineEdit*>(sender());
    assert (widget);
    QVariant data = widget->property("mapping");

    JSONDataMapping* mapping = data.value<JSONDataMapping*>();
    assert (mapping);

    mapping->jsonKey (widget->text().toStdString());
}

void JSONObjectParserWidget::mappingCommentChangedSlot()
{
    loginf << "JSONObjectParserWidget: mappingCommentChangedSlot";

    QLineEdit* widget = static_cast<QLineEdit*>(sender());
    assert (widget);
    QVariant data = widget->property("mapping");

    JSONDataMapping* mapping = data.value<JSONDataMapping*>();
    assert (mapping);

    mapping->comment (widget->text().toStdString());
}

void JSONObjectParserWidget::mappingDBOVariableChangedSlot()
{
    loginf << "JSONObjectParserWidget: mappingDBOVariableChangedSlot";

    DBOVariableSelectionWidget* var_widget = static_cast<DBOVariableSelectionWidget*>(sender());
    assert (var_widget);
    QVariant data = var_widget->property("mapping");
    //unsigned int row = var_widget->property("row").toUInt();

    JSONDataMapping* mapping = data.value<JSONDataMapping*>();
    assert (mapping);

    if (var_widget->hasVariable())
    {
        loginf << "JSONObjectParserWidget: mappingDBOVariableChangedSlot: variable set";

        mapping->dboVariableName(var_widget->selectedVariable().name());
//        if (format_selections_.count(row) == 1)
//            format_selections_.at(row)->update(mapping->formatDataTypeRef(),
//                                               mapping->jsonValueFormatRef());
//        else
//        {
//            DataTypeFormatSelectionWidget* data_format_widget
//                    = new DataTypeFormatSelectionWidget (mapping->formatDataTypeRef(),
//                                                         mapping->jsonValueFormatRef());

//            mappings_grid_->addWidget (data_format_widget, row, 5);
//            format_selections_[row] = data_format_widget;
//        }
    }
    else
    {
        loginf << "JSONObjectParserWidget: mappingDBOVariableChangedSlot: variable removed";

        mapping->dboVariableName("");

//        if (format_selections_.count(row) == 1)
//        {
//            mappings_grid_->removeWidget(format_selections_.at(row));
//            delete format_selections_.at(row);
//            format_selections_.erase(row);
//        }
    }
}

void JSONObjectParserWidget::mappingMandatoryChangedSlot()
{
    loginf << "JSONObjectParserWidget: mappingMandatoryChangedSlot";

    QCheckBox* widget = static_cast<QCheckBox*>(sender());
    assert (widget);
    QVariant data = widget->property("mapping");

    JSONDataMapping* mapping = data.value<JSONDataMapping*>();
    assert (mapping);

    mapping->mandatory(widget->checkState() == Qt::Checked);
}

void JSONObjectParserWidget::mappingDeleteSlot()
{
    loginf << "JSONObjectParserWidget: mappingDeleteSlot";

    QPushButton* widget = static_cast<QPushButton*>(sender());
    assert (widget);
    QVariant data = widget->property("mapping");

    JSONDataMapping* mapping = data.value<JSONDataMapping*>();
    assert (mapping);

    unsigned int index = widget->property("index").toUInt();

    assert (parser_->hasMapping(index));
    parser_->removeMapping(index);

    updateMappingsGrid();
}

