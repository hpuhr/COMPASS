/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbcontent/dbcontentwidget.h"

//#include "compass.h"
//#include "configuration.h"
//#include "configurationmanager.h"
#include "dbcontent/dbcontent.h"
//#include "dbodatasourcedefinitionwidget.h"
//#include "dboeditdatasourceswidget.h"
//#include "dbcontent/labeldefinitionwidget.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variabledatatypecombobox.h"
//#include "dbcontent/variable/variablewidget.h"
#include "files.h"
#include "logger.h"
//#include "stringconv.h"
#include "stringrepresentationcombobox.h"
#include "unitselectionwidget.h"

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

using namespace Utils;
using namespace dbContent;

DBContentWidget::DBContentWidget(DBContent* object, QWidget* parent,
                               Qt::WindowFlags f)
    : QWidget(parent, f), object_(object)
{
    assert(object_);

    setMinimumSize(QSize(1500, 800));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    int frame_width_small = 1;

    QVBoxLayout* main_layout = new QVBoxLayout();

    QLabel* main_label = new QLabel("Edit DBContent");
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
        connect(name_edit_, &QLineEdit::returnPressed, this, &DBContentWidget::editNameSlot);
        grid_layout->addWidget(name_edit_, 0, 1);

        QLabel* info_label = new QLabel("Description");
        grid_layout->addWidget(info_label, 1, 0);

        info_edit_ = new QLineEdit(object_->info().c_str());
        connect(info_edit_, &QLineEdit::returnPressed, this, &DBContentWidget::editInfoSlot);
        grid_layout->addWidget(info_edit_, 1, 1);

        properties_layout->addLayout(grid_layout);

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
        //updateMetaTablesGridSlot();

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

    QScrollArea* dbo_scroll = new QScrollArea();
    dbo_scroll->setWidgetResizable(true);
    dbo_scroll->setWidget(dbo_frame);

    main_layout->addWidget(dbo_scroll);

    setLayout(main_layout);

    show();
}

DBContentWidget::~DBContentWidget() {}

void DBContentWidget::editNameSlot()
{
    assert(name_edit_);
    assert(object_);

    std::string text = name_edit_->text().toStdString();
    assert(text.size() > 0);
    object_->name(text);
    emit changedDBOSignal();
}
void DBContentWidget::editInfoSlot()
{
    assert(info_edit_);
    assert(object_);

    std::string text = info_edit_->text().toStdString();
    assert(text.size() > 0);
    object_->info(text);
    emit changedDBOSignal();
}

void DBContentWidget::editDBContentVariableNameSlot()
{
    QLineEdit* edit = static_cast<QLineEdit*>(sender());
    assert(edit);

    std::string new_name = edit->text().toStdString();

    Variable* variable = edit->property("variable").value<Variable*>();
    assert(variable);
    assert(object_->hasVariable(variable->name()));

    if (new_name.size() == 0 || object_->hasVariable(new_name))
    {
        QMessageBox m_warning(QMessageBox::Warning, "DBContent Variable Renaming Failed",
                              "New name is empty or already exists.", QMessageBox::Ok);

        m_warning.exec();
        edit->setText(variable->name().c_str());
        return;
    }

    object_->renameVariable(variable->name(), new_name);
}

void DBContentWidget::editDBContentVariableDescriptionSlot()
{
    QLineEdit* edit = static_cast<QLineEdit*>(sender());
    assert(edit);

    Variable* variable = edit->property("variable").value<Variable*>();
    assert(variable);

    variable->description(edit->text().toStdString());
}

void DBContentWidget::editDBContentVariableDBColumnSlot(const QString& text)
{
    assert (false); // TODO
}

void DBContentWidget::deleteDBOVarSlot()
{
    QPushButton* button = static_cast<QPushButton*>(sender());
    assert(button);

    QVariant data = button->property("variable");

    Variable* variable = data.value<Variable*>();
    assert(variable);
    object_->deleteVariable(variable->name());

    updateDBOVarsGridSlot();
}

void DBContentWidget::updateDataSourcesGridSlot()
{
    assert(object_);
    assert(ds_grid_);

    QLayoutItem* child;
    while (!ds_grid_->isEmpty() && (child = ds_grid_->takeAt(0)) != nullptr)
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

    QIcon edit_icon(Files::IconProvider::getIcon("edit.png"));
    QIcon del_icon(Files::IconProvider::getIcon("delete.png"));
}

void DBContentWidget::updateDBOVarsGridSlot()
{
    assert(object_);
    assert(dbovars_grid_);

    QLayoutItem* child;
    while (!dbovars_grid_->isEmpty() && (child = dbovars_grid_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    logdbg << "creating first row";

    QFont font_bold;
    font_bold.setBold(true);

    unsigned int col = 0;
    unsigned int row = 0;

    //auto& meta_tables = object_->metaTables();
    //auto& schemas = COMPASS::instance().schemaManager().getSchemas();
    //std::string schema_name;

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

    col++;
    std::string schema_string = "Schema";
    QLabel* label = new QLabel(schema_string.c_str());
    dbovars_grid_->addWidget(label, row, col);

    logdbg << "getting schemas";

    QIcon del_icon(Files::IconProvider::getIcon("delete.png"));

    logdbg << "reating variable rows";

    for (auto& var_it : object_->variables())
    {
        row++;
        col = 0;

        Variable& variable = *var_it.second.get();

        // QVariant data = QVariant(qMetaTypeId<QObject*>(), var_it.second);
        // QVariant data = QVariant::fromValue(dynamic_cast<QObject*>(var_it.second));
        QVariant data = QVariant::fromValue(&variable);

        // logdbg  << "creating variable row for " << it->first
        // << " name";
        QLineEdit* name_edit = new QLineEdit(var_it.first.c_str());
        name_edit->setMaximumWidth(200);
        name_edit->setProperty("variable", data);
        connect(name_edit, SIGNAL(returnPressed()), this, SLOT(editDBContentVariableNameSlot()));
        dbovars_grid_->addWidget(name_edit, row, col);

        // logdbg  << "creating variable row for " << it->first
        // << " info";
        col++;
        QLineEdit* description_edit = new QLineEdit(var_it.second->description().c_str());
        description_edit->setMaximumWidth(300);
        description_edit->setProperty("variable", data);
        connect(description_edit, SIGNAL(returnPressed()), this,
                SLOT(editDBContentVariableDescriptionSlot()));
        dbovars_grid_->addWidget(description_edit, row, col);

        col++;
        VariableDataTypeComboBox* type_combo = new VariableDataTypeComboBox(
                    variable.dataTypeRef(), variable.dataTypeStringRef());
        dbovars_grid_->addWidget(type_combo, row, col);

        col++;
        UnitSelectionWidget* unit_sel =
            new UnitSelectionWidget(var_it.second->dimension(), var_it.second->unit());
        dbovars_grid_->addWidget(unit_sel, row, col);

        col++;
        StringRepresentationComboBox* representation_box =
            new StringRepresentationComboBox(variable.representationRef(), variable.representationStringRef());
        dbovars_grid_->addWidget(representation_box, row, col);

        col++;

        // TODO
//        DBTableColumnComboBox* box = new DBTableColumnComboBox(
//                    test.currentMetaTable().name(), var_it.second);
//        box->setProperty("variable", data);
//        connect(box, SIGNAL(activated(const QString&)), this,
//                SLOT(editDBOVariableDBColumnSlot(const QString&)));
//        dbovars_grid_->addWidget(box, row, col);

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
    // logdbg  << "done";
}
