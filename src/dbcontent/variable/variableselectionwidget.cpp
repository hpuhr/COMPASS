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

#include "dbcontent/variable/variableselectionwidget.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "files.h"
#include "global.h"
#include "dbcontent/variable/metavariable.h"
#include "logger.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include <algorithm>

using namespace Utils;
using namespace std;

namespace dbContent
{

VariableSelectionWidget::VariableSelectionWidget(bool h_box, QWidget* parent)
    : QFrame(parent), dbo_man_(COMPASS::instance().dbContentManager())
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setLineWidth(1);

    QBoxLayout* layout;

    object_label_ = new QLabel(this);
    variable_label_ = new QLabel(this);
    variable_label_->setAlignment(Qt::AlignRight);

    sel_button_ = new QPushButton(this);
    sel_button_->setIcon(QIcon(Files::getIconFilepath("expand.png").c_str()));
    sel_button_->setFixedSize(UI_ICON_SIZE);
    sel_button_->setFlat(UI_ICON_BUTTON_FLAT);

    QSizePolicy sp_retain = sel_button_->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    sel_button_->setSizePolicy(sp_retain);

    if (h_box)
    {
        layout = new QHBoxLayout;
        layout->setContentsMargins(1, 1, 1, 1);
        layout->setSpacing(1);

        layout->addWidget(object_label_);
        layout->addWidget(variable_label_);

        layout->addWidget(sel_button_);
    }
    else
    {
        layout = new QVBoxLayout;
        layout->setContentsMargins(1, 1, 1, 1);
        layout->setSpacing(1);

        QHBoxLayout* select_layout = new QHBoxLayout();
        select_layout->setContentsMargins(1, 1, 1, 1);
        select_layout->setSpacing(1);

        select_layout->addWidget(object_label_);
        select_layout->addWidget(sel_button_);
        layout->addLayout(select_layout);

        layout->addWidget(variable_label_);
    }
    setLayout(layout);

    connect(&menu_, SIGNAL(triggered(QAction*)), this, SLOT(triggerSlot(QAction*)));
    connect(sel_button_, SIGNAL(clicked()), this, SLOT(showMenuSlot()));

    updateMenuEntries();
}

VariableSelectionWidget::~VariableSelectionWidget() {}

void VariableSelectionWidget::setReadOnly(bool read_only)
{
    assert (sel_button_);

    sel_button_->setDisabled(read_only);
}

void VariableSelectionWidget::updateMenuEntries()
{
    menu_.clear();

    if (show_empty_variable_) // show empty
    {
        QAction* action = menu_.addAction("");
        QVariantMap vmap;
        vmap.insert(QString(""), QVariant(QString("")));
        action->setData(QVariant(vmap));
    }

    if (show_dbo_only_)
    {
        assert(dbo_man_.existsDBContent(only_dbcontent_name_));

        for (auto& var_it : dbo_man_.dbContent(only_dbcontent_name_).variables())
        {
            if (show_data_types_only_ && !showDataType(var_it.second->dataType()))
                continue;

            QAction* action = menu_.addAction(var_it.first.c_str());
            action->setToolTip(var_it.second->description().c_str());

            QVariantMap vmap;
            vmap.insert(QString::fromStdString(var_it.first),
                        QVariant(QString::fromStdString(only_dbcontent_name_)));
            action->setData(QVariant(vmap));
        }
    }
    else
    {
        if (show_meta_variables_)
        {
            QMenu* meta_menu = menu_.addMenu(QString::fromStdString(META_OBJECT_NAME));
            for (auto& meta_it : dbo_man_.metaVariables())
            {
                if (show_data_types_only_ && !showDataType(meta_it.second->dataType()))
                    continue;

                QAction* action = meta_menu->addAction(QString::fromStdString(meta_it.first));
                action->setToolTip(meta_it.second->description().c_str());

                QVariantMap vmap;
                vmap.insert(QString::fromStdString(meta_it.first),
                            QVariant(QString::fromStdString(META_OBJECT_NAME)));
                action->setData(QVariant(vmap));
            }
        }

        if (show_meta_variables_only_)
            return;

        for (auto& object_it : dbo_man_)
        {
            QMenu* m2 = menu_.addMenu(QString::fromStdString(object_it.first));

            for (auto& var_it : object_it.second->variables())
            {
                if (show_data_types_only_ && !showDataType(var_it.second->dataType()))
                    continue;

                QAction* action = m2->addAction(QString::fromStdString(var_it.first));
                action->setToolTip(var_it.second->description().c_str());

                QVariantMap vmap;
                vmap.insert(QString::fromStdString(var_it.first),
                            QVariant(QString::fromStdString(object_it.first)));
                action->setData(QVariant(vmap));
            }
        }
    }
}

bool VariableSelectionWidget::showDataType(PropertyDataType type)
{
    return std::find(only_data_types_.begin(), only_data_types_.end(), type) != only_data_types_.end();
}

void VariableSelectionWidget::updateToolTip()
{
    if (hasVariable())
        setToolTip(selectedVariable().description().c_str());
    else if (hasMetaVariable())
        setToolTip(selectedMetaVariable().description().c_str());
    else
        setToolTip("");
}

void VariableSelectionWidget::showMenuSlot() { menu_.exec(QCursor::pos()); }

void VariableSelectionWidget::triggerSlot(QAction* action)
{
    std::cout << "TRIGGERED!!!!!!!!!!!!" << std::endl;

    assert(object_label_);
    assert(variable_label_);

    QVariantMap vmap = action->data().toMap();
    std::string var_name = vmap.begin().key().toStdString();
    std::string obj_name = vmap.begin().value().toString().toStdString();

    if (var_name.size() == 0 && obj_name.size() == 0)
    {
        meta_variable_selected_ = false;
        variable_selected_ = false;
    }
    else
    {
        if (obj_name == META_OBJECT_NAME)
        {
            meta_variable_selected_ = true;
            variable_selected_ = false;
        }
        else
        {
            assert(dbo_man_.dbContent(obj_name).hasVariable(var_name));

            meta_variable_selected_ = false;
            variable_selected_ = true;
        }
    }

    object_label_->setText(obj_name.c_str());
    variable_label_->setText(var_name.c_str());

    loginf << "VariableSelectionWidget: triggerSlot: obj " << obj_name.c_str() << " var "
           << var_name.c_str();

    updateToolTip();

    emit selectionChanged();
}

void VariableSelectionWidget::selectedVariable(Variable& variable)
{
    assert(object_label_);
    assert(variable_label_);

    object_label_->setText(QString::fromStdString(variable.dbObject().name()));
    variable_label_->setText(variable.name().c_str());

    variable_selected_ = true;
    meta_variable_selected_ = false;

    updateToolTip();
}

void VariableSelectionWidget::selectEmptyVariable()
{
    assert (show_empty_variable_);

    assert(object_label_);
    assert(variable_label_);

    object_label_->setText("");
    variable_label_->setText("");

    variable_selected_ = false;
    meta_variable_selected_ = false;

    updateToolTip();
}

Variable& VariableSelectionWidget::selectedVariable() const
{
    assert(object_label_);
    assert(variable_label_);
    assert(variable_selected_);

    std::string obj_name = object_label_->text().toStdString();
    std::string var_name = variable_label_->text().toStdString();

    assert(dbo_man_.dbContent(obj_name).hasVariable(var_name));

    return dbo_man_.dbContent(obj_name).variable(var_name);
}

void VariableSelectionWidget::selectedMetaVariable(MetaVariable& variable)
{
    assert(object_label_);
    assert(variable_label_);

    object_label_->setText(QString::fromStdString(META_OBJECT_NAME));
    variable_label_->setText(variable.name().c_str());

    variable_selected_ = false;
    meta_variable_selected_ = true;

    updateToolTip();
}

MetaVariable& VariableSelectionWidget::selectedMetaVariable() const
{
    assert(object_label_);
    assert(variable_label_);
    assert(meta_variable_selected_);

    std::string obj_name = object_label_->text().toStdString();
    std::string var_name = variable_label_->text().toStdString();

    assert(obj_name == META_OBJECT_NAME);
    assert(dbo_man_.existsMetaVariable(var_name));

    return dbo_man_.metaVariable(var_name);
}

void VariableSelectionWidget::showDBContentOnly(const std::string& only_dbcontent_name)
{
    show_dbo_only_ = true;
    only_dbcontent_name_ = only_dbcontent_name;

    assert(object_label_);
    object_label_->hide();

    updateMenuEntries();
}

void VariableSelectionWidget::disableShowDBContentOnly()
{
    show_dbo_only_ = false;
    only_dbcontent_name_ = "";

    assert(object_label_);
    object_label_->show();

    updateMenuEntries();
}

std::string VariableSelectionWidget::onlyDBContentName() const { return only_dbcontent_name_; }

bool VariableSelectionWidget::showEmptyVariable() const { return show_empty_variable_; }

void VariableSelectionWidget::showEmptyVariable(bool show_empty_variable)
{
    show_empty_variable_ = show_empty_variable;

    updateMenuEntries();
}

void VariableSelectionWidget::showDataTypesOnly(const std::vector<PropertyDataType>& only_data_types)
{
    only_data_types_ = only_data_types;
    show_data_types_only_ = true;

    updateMenuEntries();
}

void VariableSelectionWidget::disableShowDataTypesOnly()
{
    show_data_types_only_ = false;
}

bool VariableSelectionWidget::showMetaVariablesOnly() const { return show_meta_variables_only_; }

void VariableSelectionWidget::showMetaVariablesOnly(bool show_meta_variables_only)
{
    show_meta_variables_only_ = show_meta_variables_only;

    if (show_meta_variables_only_)
        show_meta_variables_ = true;

    updateMenuEntries();
}

bool VariableSelectionWidget::showMetaVariables() const { return show_meta_variables_; }

void VariableSelectionWidget::showMetaVariables(bool show_meta_variables)
{
    show_meta_variables_ = show_meta_variables;
    updateMenuEntries();
}

QPushButton* VariableSelectionWidget::selectionButton() const
{
    return sel_button_;
}

}
