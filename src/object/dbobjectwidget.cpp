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

#include "dbobjectwidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>
#include <boost/algorithm/string.hpp>

#include "atsdb.h"
#include "configuration.h"
#include "configurationmanager.h"
#include "dboadddatasourcedialog.h"
#include "dboaddschemametatabledialog.h"
#include "dbobject.h"
#include "dbodatasourcedefinitionwidget.h"
#include "dboeditdatasourceswidget.h"
#include "dbolabeldefinitionwidget.h"
#include "dbovariable.h"
#include "dbovariabledatatypecombobox.h"
#include "dbovariablewidget.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbtablecolumn.h"
#include "dbtablecolumncombobox.h"
#include "files.h"
#include "logger.h"
#include "metadbtable.h"
#include "stringconv.h"
#include "stringrepresentationcombobox.h"
#include "unitselectionwidget.h"

using namespace Utils;

DBObjectWidget::DBObjectWidget(DBObject* object, DBSchemaManager& schema_manager, QWidget* parent,
                               Qt::WindowFlags f)
    : QWidget(parent, f), object_(object), schema_manager_(schema_manager)
{
    assert(object_);

    setMinimumSize(QSize(1500, 800));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    int frame_width_small = 1;

    QVBoxLayout* main_layout = new QVBoxLayout();

    QLabel* main_label = new QLabel("Edit DBObject");
    main_label->setFont(font_big);
    main_layout->addWidget(main_label);

    QHBoxLayout* top_layout = new QHBoxLayout();

    {
        QFrame* properties_frame = new QFrame();
        properties_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        properties_frame->setLineWidth(frame_width_small);

        QVBoxLayout* properties_layout = new QVBoxLayout();

        QLabel* properties_label = new QLabel("Properties");
        properties_label->setFont(font_big);
        properties_layout->addWidget(properties_label);

        QGridLayout* grid_layout = new QGridLayout();

        QLabel* name_label = new QLabel("Database object name");
        grid_layout->addWidget(name_label, 0, 0);

        name_edit_ = new QLineEdit(object_->name().c_str());
        connect(name_edit_, &QLineEdit::returnPressed, this, &DBObjectWidget::editNameSlot);
        grid_layout->addWidget(name_edit_, 0, 1);

        QLabel* info_label = new QLabel("Description");
        grid_layout->addWidget(info_label, 1, 0);

        info_edit_ = new QLineEdit(object_->info().c_str());
        connect(info_edit_, &QLineEdit::returnPressed, this, &DBObjectWidget::editInfoSlot);
        grid_layout->addWidget(info_edit_, 1, 1);

        properties_layout->addLayout(grid_layout);

        edit_label_button_ = new QPushButton("Edit Label Definition");
        connect(edit_label_button_, &QPushButton::clicked, this,
                &DBObjectWidget::showLabelDefinitionWidgetSlot);
        properties_layout->addWidget(edit_label_button_);

        QPushButton* print_button_ = new QPushButton("Print");
        connect(print_button_, &QPushButton::clicked, this, &DBObjectWidget::printSlot);
        properties_layout->addWidget(print_button_);

        properties_frame->setLayout(properties_layout);

        top_layout->addWidget(properties_frame, 1);
    }

    // metas
    {
        QFrame* meta_frame = new QFrame();
        meta_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        meta_frame->setLineWidth(frame_width_small);

        QVBoxLayout* meta_layout = new QVBoxLayout();

        QLabel* meta_label = new QLabel("Meta Tables");
        meta_label->setFont(font_big);
        meta_layout->addWidget(meta_label);

        meta_table_grid_ = new QGridLayout();
        updateMetaTablesGridSlot();

        meta_layout->addLayout(meta_table_grid_);

        new_meta_button_ = new QPushButton("Add");
        connect(new_meta_button_, SIGNAL(clicked()), this, SLOT(addMetaTableSlot()));
        meta_layout->addWidget(new_meta_button_);

        meta_frame->setLayout(meta_layout);

        top_layout->addWidget(meta_frame, 1);
    }

    // data sources
    {
        QFrame* ds_frame = new QFrame();
        ds_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        ds_frame->setLineWidth(frame_width_small);

        QVBoxLayout* ds_layout = new QVBoxLayout();

        QLabel* ds_label = new QLabel("Data Source Definitions");
        ds_label->setFont(font_big);
        ds_layout->addWidget(ds_label);

        ds_grid_ = new QGridLayout();
        updateDataSourcesGridSlot();
        ds_layout->addLayout(ds_grid_);

        new_ds_button_ = new QPushButton("Add");
        connect(new_ds_button_, &QPushButton::clicked, this, &DBObjectWidget::addDataSourceSlot);
        ds_layout->addWidget(new_ds_button_);

        ds_frame->setLayout(ds_layout);

        top_layout->addWidget(ds_frame, 1);
    }
    main_layout->addLayout(top_layout);

    // dobvars
    QLabel* dbo_label = new QLabel("Variables");
    dbo_label->setFont(font_big);
    main_layout->addWidget(dbo_label);

    QFrame* dbo_frame = new QFrame();
    dbo_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    dbo_frame->setLineWidth(frame_width_small);

    dbovars_grid_ = new QGridLayout();
    updateDBOVarsGridSlot();

    dbo_frame->setLayout(dbovars_grid_);

    QScrollArea* dbo_scroll_ = new QScrollArea();
    dbo_scroll_->setWidgetResizable(true);
    dbo_scroll_->setWidget(dbo_frame);

    main_layout->addWidget(dbo_scroll_);

    // add all

    QHBoxLayout* all_var_layout = new QHBoxLayout();

    all_var_layout->addStretch();

    update_variables_button_ = new QPushButton("Update Variables");
    connect(update_variables_button_, &QPushButton::clicked, this,
            &DBObjectWidget::updateVariablesSlot);
    all_var_layout->addWidget(update_variables_button_);

    main_layout->addLayout(all_var_layout);

    setLayout(main_layout);

    show();
}

DBObjectWidget::~DBObjectWidget() {}

void DBObjectWidget::editNameSlot()
{
    logdbg << "DBObjectWidget: editName";
    assert(name_edit_);
    assert(object_);

    std::string text = name_edit_->text().toStdString();
    assert(text.size() > 0);
    object_->name(text);
    emit changedDBOSignal();
}
void DBObjectWidget::editInfoSlot()
{
    logdbg << "DBObjectWidget: editInfo";
    assert(info_edit_);
    assert(object_);

    std::string text = info_edit_->text().toStdString();
    assert(text.size() > 0);
    object_->info(text);
    emit changedDBOSignal();
}

void DBObjectWidget::editDBOVariableNameSlot()
{
    logdbg << "DBObjectWidget: editDBOVariableNameSlot";

    QLineEdit* edit = static_cast<QLineEdit*>(sender());
    assert(edit);

    std::string new_name = edit->text().toStdString();

    DBOVariable* variable = edit->property("variable").value<DBOVariable*>();
    assert(variable);
    assert(object_->hasVariable(variable->name()));

    if (new_name.size() == 0 || object_->hasVariable(new_name))
    {
        QMessageBox m_warning(QMessageBox::Warning, "DBOVariable Renaming Failed",
                              "New name is empty or already exists.", QMessageBox::Ok);

        m_warning.exec();
        edit->setText(variable->name().c_str());
        return;
    }

    object_->renameVariable(variable->name(), new_name);
}

void DBObjectWidget::editDBOVariableDescriptionSlot()
{
    logdbg << "DBObjectWidget: editDBOVariableDescriptionSlot";

    QLineEdit* edit = static_cast<QLineEdit*>(sender());
    assert(edit);

    DBOVariable* variable = edit->property("variable").value<DBOVariable*>();
    assert(variable);

    variable->description(edit->text().toStdString());
}

void DBObjectWidget::editDBOVariableDBColumnSlot(const QString& text)
{
    logdbg << "DBObjectWidget: editDBOVariableDBColumnSlot";

    DBTableColumnComboBox* box = static_cast<DBTableColumnComboBox*>(sender());
    assert(box);

    DBOVariable* variable = box->property("variable").value<DBOVariable*>();
    assert(variable);

    std::string schema = box->property("schema").value<QString>().toStdString();

    loginf << "DBObjectWidget: editDBOVariableDBColumnSlot: var " << variable->name() << " schema "
           << schema << " text '" << text.toStdString() << "'";

    variable->setVariableName(schema, text.toStdString());
}

void DBObjectWidget::deleteDBOVarSlot()
{
    logdbg << "DBObjectWidget: deleteDBOVar";

    QPushButton* button = static_cast<QPushButton*>(sender());
    assert(button);

    QVariant data = button->property("variable");

    DBOVariable* variable = data.value<DBOVariable*>();
    assert(variable);
    object_->deleteVariable(variable->name());

    updateDBOVarsGridSlot();
}

void DBObjectWidget::updateDataSourcesGridSlot()
{
    logdbg << "DBObjectWidget: updateDataSourcesGrid";
    assert(object_);
    assert(ds_grid_);

    QLayoutItem* child;
    while ((child = ds_grid_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QFont font_bold;
    font_bold.setBold(true);

    QLabel* schema_label = new QLabel("Schema");
    schema_label->setFont(font_bold);
    ds_grid_->addWidget(schema_label, 0, 0);

    QLabel* edit_label = new QLabel("Edit");
    edit_label->setFont(font_bold);
    ds_grid_->addWidget(edit_label, 0, 1);

    QLabel* del_label = new QLabel("Delete");
    del_label->setFont(font_bold);
    ds_grid_->addWidget(del_label, 0, 2);

    QIcon edit_icon(Files::getIconFilepath("edit.png").c_str());
    QIcon del_icon(Files::getIconFilepath("delete.png").c_str());

    unsigned int row = 1;

    for (auto& ds_it : object_->dataSourceDefinitions())
    {
        QLabel* schema = new QLabel(ds_it.second.schema().c_str());
        ds_grid_->addWidget(schema, row, 0);

        DBODataSourceDefinition* dsdef = &ds_it.second;
        QVariant data = QVariant::fromValue(dsdef);

        QPushButton* edit = new QPushButton();
        edit->setIcon(edit_icon);
        edit->setFixedSize(UI_ICON_SIZE);
        edit->setFlat(UI_ICON_BUTTON_FLAT);
        connect(edit, SIGNAL(clicked()), this, SLOT(editDataSourceSlot()));
        edit->setProperty("data_source", data);
        ds_grid_->addWidget(edit, row, 1);

        QPushButton* del = new QPushButton();
        del->setIcon(del_icon);
        del->setFixedSize(UI_ICON_SIZE);
        del->setFlat(UI_ICON_BUTTON_FLAT);
        connect(del, SIGNAL(clicked()), this, SLOT(deleteDataSourceSlot()));
        del->setProperty("data_source", data);
        ds_grid_->addWidget(del, row, 2);

        row++;
    }
}

void DBObjectWidget::addDataSourceSlot()
{
    logdbg << "DBObjectWidget: addDataSource";

    DBOAddDataSourceDialog dialog;

    int ret = dialog.exec();

    if (ret == QDialog::Accepted)

    {
        std::string schema = dialog.schemaName();

        if (object_->hasDataSourceDefinition(schema))
        {
            QMessageBox m_warning(QMessageBox::Warning, "Data Source Adding Failed",
                                  "Data source for this schema is already defined.",
                                  QMessageBox::Ok);

            m_warning.exec();
            return;
        }

        std::string instance = "DBODataSourceDefinition" + object_->name() + schema + "0";

        Configuration& config =
            object_->addNewSubConfiguration("DBODataSourceDefinition", instance);
        config.addParameterString("schema", schema);

        object_->generateSubConfigurable("DBODataSourceDefinition", instance);
        updateDataSourcesGridSlot();
    }
}

void DBObjectWidget::editDataSourceSlot()
{
    logdbg << "DBObjectWidget: editDataSource";
    QPushButton* button = static_cast<QPushButton*>(sender());
    QVariant data = button->property("data_source");

    DBODataSourceDefinition* definition = data.value<DBODataSourceDefinition*>();
    assert(definition);
    definition->widget()->show();
}

void DBObjectWidget::deleteDataSourceSlot()
{
    logdbg << "DBObjectWidget: deleteDBOVar";

    QPushButton* button = static_cast<QPushButton*>(sender());
    QVariant data = button->property("data_source");

    DBODataSourceDefinition* definition = data.value<DBODataSourceDefinition*>();
    assert(definition);
    assert(object_);
    assert(object_->hasDataSourceDefinition(definition->schema()));
    object_->deleteDataSourceDefinition(definition->schema());
}

void DBObjectWidget::deleteMetaTableSlot()
{
    loginf << "DBObjectWidget: deleteMetaTableSlot";

    QPushButton* button = static_cast<QPushButton*>(sender());
    QVariant data = button->property("schema");

    std::string schema = data.value<QString>().toStdString();

    assert(object_);
    assert(object_->hasMetaTable(schema));
    object_->deleteMetaTable(schema);
}

void DBObjectWidget::updateMetaTablesGridSlot()
{
    logdbg << "DBObjectWidget: updateSchemaMetaTables";
    assert(object_);
    assert(meta_table_grid_);

    QLayoutItem* child;
    while ((child = meta_table_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QFont font_bold;
    font_bold.setBold(true);

    QIcon del_icon(Files::getIconFilepath("delete.png").c_str());

    QLabel* schema_label = new QLabel("Schema");
    schema_label->setFont(font_bold);
    meta_table_grid_->addWidget(schema_label, 0, 0);

    QLabel* meta_label = new QLabel("Meta table");
    meta_label->setFont(font_bold);
    meta_table_grid_->addWidget(meta_label, 0, 1);

    auto& metas = object_->metaTables();

    unsigned int row = 1;
    for (auto it = metas.begin(); it != metas.end(); it++)
    {
        QLabel* schema = new QLabel(it->first.c_str());
        meta_table_grid_->addWidget(schema, row, 0);

        QLabel* meta = new QLabel(it->second.metaTable().c_str());
        meta_table_grid_->addWidget(meta, row, 1);

        QPushButton* del = new QPushButton();
        del->setIcon(del_icon);
        del->setFixedSize(UI_ICON_SIZE);
        del->setFlat(UI_ICON_BUTTON_FLAT);
        connect(del, SIGNAL(clicked()), this, SLOT(deleteMetaTableSlot()));
        del->setProperty("schema", QVariant(it->first.c_str()));
        meta_table_grid_->addWidget(del, row, 2);

        row++;
    }
}

void DBObjectWidget::showLabelDefinitionWidgetSlot()
{
    assert(object_);
    object_->labelDefinitionWidget()->show();
}

void DBObjectWidget::printSlot()
{
    assert(object_);
    object_->print();
}

void DBObjectWidget::updateVariablesSlot()
{
    assert(object_);

    if (!schema_manager_.hasCurrentSchema())
    {
        logerr << "DBObjectWidget: updateVariablesSlot: no current schema defined";
        return;
    }

    std::string schema_name = schema_manager_.getCurrentSchema().name();

    loginf << "DBObjectWidget: updateVariablesSlot: object " << object_->name() << " schema "
           << schema_name;

    assert(object_->hasMetaTable(schema_name));
    std::string meta_name = object_->metaTable(schema_name);
    logdbg << "DBObjectWidget: updateVariablesSlot: for meta " << meta_name;

    assert(schema_manager_.getCurrentSchema().hasMetaTable(meta_name));
    const MetaDBTable& meta = schema_manager_.getCurrentSchema().metaTable(meta_name);

    const std::map<std::string, const DBTableColumn&>& meta_columns = meta.columns();

    loginf << "DBObjectWidget: updateVariablesSlot: updating variables";

    std::vector<std::string> vars_to_be_removed;

    for (auto& dbovar_it : *object_)
    {
        if (!dbovar_it.second.hasCurrentDBColumn())
        {
            loginf << "DBObjectWidget: updateVariablesSlot: variable '" << dbovar_it.first
                   << "' has no DBTableColumn";
            vars_to_be_removed.push_back(dbovar_it.first);
        }
    }

    for (auto& dbovar_name : vars_to_be_removed)
    {
        loginf << "DBObjectWidget: updateVariablesSlot: removing variable '" << dbovar_name;
        object_->deleteVariable(dbovar_name);
    }

    loginf << "DBObjectWidget: updateVariablesSlot: updating columns";
    for (auto& col_it : meta_columns)
    {
        const DBTableColumn& col = col_it.second;
        logdbg << "DBObjectWidget: updateVariablesSlot: checking column " << col.name();

        std::string column_name = col.name();
        boost::algorithm::to_lower(column_name);
        std::string column_identifier = col.identifier();
        boost::algorithm::to_lower(column_identifier);

        if (object_->uses(col))
        {
            logdbg << "DBObjectWidget: updateVariablesSlot: not adding column '" << column_name
                   << "' since already used";
            continue;
        }
        else
            loginf << "DBObjectWidget: updateVariablesSlot: adding column '" << column_name;

        std::string column_name_to_use;

        if (!object_->hasVariable(column_name))
            column_name_to_use = column_name;
        else
            column_name_to_use = column_identifier;

        std::string instance = "DBOVariable" + object_->name() + column_name_to_use + "0";

        Configuration& config = object_->addNewSubConfiguration("DBOVariable", instance);

        config.addParameterString("name", column_name_to_use);
        config.addParameterString("data_type_str", Property::asString(col.propertyType()));
        config.addParameterString("dbo_name", object_->name());
        config.addParameterString("description", col.comment());

        std::string var_instance =
            "DBOSchemaVariableDefinition" + object_->name() + column_name + "0";

        Configuration& var_configuration =
            config.addNewSubConfiguration("DBOSchemaVariableDefinition", var_instance);
        var_configuration.addParameterString("schema", schema_name);
        var_configuration.addParameterString("variable_identifier", col.identifier());

        object_->generateSubConfigurable("DBOVariable", instance);

        loginf << "DBObjectWidget: updateVariablesSlot: added column '" << column_name_to_use
               << "' as variable";
    }
    updateDBOVarsGridSlot();
}

void DBObjectWidget::addMetaTableSlot()
{
    logdbg << "DBObjectWidget: addMetaTable";
    assert(object_);

    DBOAddSchemaMetaTableDialog dialog;
    int ret = dialog.exec();

    if (ret == QDialog::Accepted)
    {
        std::string schema_name = dialog.schemaName();
        std::string meta_name = dialog.metaTableName();

        if (object_->hasMetaTable(schema_name))
        {
            QMessageBox m_warning(QMessageBox::Warning, "Schema Adding Failed",
                                  "There is already a Meta-Table defined for the selected Schema.",
                                  QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        std::string table_instance = "DBOSchemaMetaTableDefinition" + schema_name + meta_name + "0";

        Configuration& table_config =
            object_->addNewSubConfiguration("DBOSchemaMetaTableDefinition", table_instance);
        table_config.addParameterString("schema", schema_name);
        table_config.addParameterString("meta_table", meta_name);

        object_->generateSubConfigurable("DBOSchemaMetaTableDefinition", table_instance);

        updateMetaTablesGridSlot();
        updateDBOVarsGridSlot();
    }
}

void DBObjectWidget::updateDBOVarsGridSlot()
{
    logdbg << "DBObjectWidget: updateDBOVarsGrid";
    assert(object_);
    assert(dbovars_grid_);

    QLayoutItem* child;
    while ((child = dbovars_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    logdbg << "DBObjectWidget: updateDBOVarsGrid: creating first row";

    QFont font_bold;
    font_bold.setBold(true);

    unsigned int col = 0;
    unsigned int row = 0;

    auto& meta_tables = object_->metaTables();
    auto& schemas = ATSDB::instance().schemaManager().getSchemas();
    std::string schema_name;

    QLabel* name_label = new QLabel("Name");
    name_label->setFont(font_bold);
    dbovars_grid_->addWidget(name_label, row, col);

    col++;
    QLabel* info_label = new QLabel("Description");
    info_label->setFont(font_bold);
    dbovars_grid_->addWidget(info_label, row, col);

    col++;
    QLabel* type_label = new QLabel("Data type");
    type_label->setFont(font_bold);
    dbovars_grid_->addWidget(type_label, row, col);

    col++;
    QLabel* unit_label = new QLabel("Unit");
    unit_label->setFont(font_bold);
    dbovars_grid_->addWidget(unit_label, row, col);

    col++;
    QLabel* representation_label = new QLabel("Representation");
    representation_label->setFont(font_bold);
    dbovars_grid_->addWidget(representation_label, row, col);

    for (auto sit = schemas.begin(); sit != schemas.end(); sit++)
    {
        schema_name = sit->first;

        if (meta_tables.find(schema_name) == meta_tables.end())
            continue;

        col++;

        std::string schema_string = "Schema: " + schema_name;
        QLabel* label = new QLabel(schema_string.c_str());
        dbovars_grid_->addWidget(label, row, col);
    }

    logdbg << "DBObjectWidget: updateDBOVarsGrid: getting schemas";

    QIcon del_icon(Files::getIconFilepath("delete.png").c_str());

    logdbg << "DBObjectWidget: updateDBOVarsGrid: creating variable rows";

    for (auto& var_it : *object_)
    {
        row++;
        col = 0;

        DBOVariable& test = var_it.second;

        // QVariant data = QVariant(qMetaTypeId<QObject*>(), var_it.second);
        // QVariant data = QVariant::fromValue(dynamic_cast<QObject*>(var_it.second));
        QVariant data = QVariant::fromValue(&test);

        // logdbg  << "DBObjectWidget: updateDBOVarsGrid: creating variable row for " << it->first
        // << " name";
        QLineEdit* name_edit = new QLineEdit(var_it.second.name().c_str());
        name_edit->setMaximumWidth(200);
        name_edit->setProperty("variable", data);
        connect(name_edit, SIGNAL(returnPressed()), this, SLOT(editDBOVariableNameSlot()));
        dbovars_grid_->addWidget(name_edit, row, col);

        // logdbg  << "DBObjectWidget: updateDBOVarsGrid: creating variable row for " << it->first
        // << " info";
        col++;
        QLineEdit* description_edit = new QLineEdit(var_it.second.description().c_str());
        description_edit->setMaximumWidth(300);
        description_edit->setProperty("variable", data);
        connect(description_edit, SIGNAL(returnPressed()), this,
                SLOT(editDBOVariableDescriptionSlot()));
        dbovars_grid_->addWidget(description_edit, row, col);

        col++;
        DBOVariableDataTypeComboBox* type_combo = new DBOVariableDataTypeComboBox(var_it.second);
        dbovars_grid_->addWidget(type_combo, row, col);

        col++;
        UnitSelectionWidget* unit_sel =
            new UnitSelectionWidget(var_it.second.dimension(), var_it.second.unit());
        dbovars_grid_->addWidget(unit_sel, row, col);

        col++;
        StringRepresentationComboBox* representation_box =
            new StringRepresentationComboBox(var_it.second);
        dbovars_grid_->addWidget(representation_box, row, col);

        for (auto schema_it = schemas.begin(); schema_it != schemas.end(); schema_it++)
        {
            schema_name = schema_it->first;

            if (meta_tables.find(schema_name) == meta_tables.end())
                continue;

            col++;

            assert(meta_tables.count(schema_name) == 1);
            DBTableColumnComboBox* box = new DBTableColumnComboBox(
                schema_name, meta_tables.at(schema_name).metaTable(), var_it.second);
            box->setProperty("variable", data);
            box->setProperty("schema", QString(schema_name.c_str()));
            connect(box, SIGNAL(activated(const QString&)), this,
                    SLOT(editDBOVariableDBColumnSlot(const QString&)));
            dbovars_grid_->addWidget(box, row, col);
        }

        col++;
        QPushButton* del = new QPushButton();
        del->setIcon(del_icon);
        del->setFixedSize(UI_ICON_SIZE);
        del->setFlat(UI_ICON_BUTTON_FLAT);
        connect(del, SIGNAL(clicked()), this, SLOT(deleteDBOVarSlot()));
        del->setProperty("variable", data);
        dbovars_grid_->addWidget(del, row, col);

        row++;
    }
    // logdbg  << "DBObjectWidget: updateDBOVarsGrid: done";
}
