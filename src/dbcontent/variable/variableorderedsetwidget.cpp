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

#include "dbcontent/variable/variableorderedsetwidget.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "files.h"
#include "global.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Utils;
using namespace std;

namespace dbContent
{

VariableOrderedSetWidget::VariableOrderedSetWidget(VariableOrderedSet& set,
                                                         QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), set_(set)
{
    QVBoxLayout* main_layout = new QVBoxLayout();
    main_layout->setContentsMargins(0, 0, 0, 0);

    QFont font_bold;
    font_bold.setBold(true);

    QHBoxLayout* hvars_layout = new QHBoxLayout();
    QVBoxLayout* vvars_layout = new QVBoxLayout();

    QLabel* var_label = new QLabel("Variables");
    var_label->setFont(font_bold);
    main_layout->addWidget(var_label);

    list_widget_ = new QListWidget();
    updateVariableListSlot();
    vvars_layout->addWidget(list_widget_);
    hvars_layout->addLayout(vvars_layout);

    // up/down buttons
    {
        QVBoxLayout* vupdown_layout = new QVBoxLayout();

        QPushButton* up = new QPushButton();
        up->setIcon(QIcon(Files::getIconFilepath("up.png").c_str()));
        up->setFixedSize(UI_ICON_SIZE);
        up->setFlat(UI_ICON_BUTTON_FLAT);
        up->setToolTip(tr("Move variable up"));
        connect(up, &QPushButton::clicked, this, &VariableOrderedSetWidget::moveUpSlot);

        vupdown_layout->addWidget(up);

        vupdown_layout->addStretch();

        QPushButton* down = new QPushButton();
        down->setIcon(QIcon(Files::getIconFilepath("down.png").c_str()));
        down->setFixedSize(UI_ICON_SIZE);
        down->setFlat(UI_ICON_BUTTON_FLAT);
        down->setToolTip(tr("Move variable down"));
        connect(down, &QPushButton::clicked, this, &VariableOrderedSetWidget::moveDownSlot);

        vupdown_layout->addWidget(down);

        hvars_layout->addLayout(vupdown_layout);
    }

    main_layout->addLayout(hvars_layout);

    // buttons
    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        QPushButton* remove_button = new QPushButton();
        remove_button->setText("Remove");
        connect(remove_button, &QPushButton::clicked, this,
                &VariableOrderedSetWidget::removeSlot);
        button_layout->addWidget(remove_button);

        QPushButton* add_button = new QPushButton();
        add_button->setText("Add");
        connect(add_button, &QPushButton::clicked, this,
                &VariableOrderedSetWidget::showMenuSlot);
        button_layout->addWidget(add_button);

        main_layout->addLayout(button_layout);
    }

    connect(&menu_, &QMenu::triggered, this, &VariableOrderedSetWidget::triggerSlot);

    setLayout(main_layout);
    updateMenuEntries();
}

VariableOrderedSetWidget::~VariableOrderedSetWidget() {}

void VariableOrderedSetWidget::updateMenuEntries()
{
    menu_.clear();
    menu_.setToolTipsVisible(true);

    QMenu* meta_menu = menu_.addMenu(META_OBJECT_NAME.c_str());
    meta_menu->setToolTipsVisible(true);

    for (auto& meta_it : COMPASS::instance().dbContentManager().metaVariables())
    {
        QAction* action = meta_menu->addAction(meta_it->name().c_str());
        action->setToolTip(meta_it->description().c_str());
        action->setData(QVariantMap({{meta_it->name().c_str(),QVariant(META_OBJECT_NAME.c_str())}}));
    }

    for (auto& object_it : COMPASS::instance().dbContentManager())
    {
        QMenu* m2 = menu_.addMenu(object_it.first.c_str());
        m2->setToolTipsVisible(true);

        for (auto& var_it : object_it.second->variables())
        {
            QAction* action = m2->addAction(var_it->name().c_str());
            action->setToolTip(var_it->description().c_str());
            action->setData(QVariantMap({{var_it->name().c_str(), QVariant(object_it.first.c_str())}}));
        }
    }
}

void VariableOrderedSetWidget::showMenuSlot() { menu_.exec(QCursor::pos()); }

void VariableOrderedSetWidget::triggerSlot(QAction* action)
{
    QVariantMap vmap = action->data().toMap();
    std::string var_name = vmap.begin().key().toStdString();
    std::string obj_name = vmap.begin().value().toString().toStdString();

    DBContentManager& manager = COMPASS::instance().dbContentManager();

    if (obj_name == META_OBJECT_NAME)
    {
        assert(manager.existsMetaVariable(var_name));
        set_.add(manager.metaVariable(var_name));
    }
    else
    {
        assert(manager.existsDBContent(obj_name));
        assert(manager.dbContent(obj_name).hasVariable(var_name));
        set_.add(manager.dbContent(obj_name).variable(var_name));
    }
}

void VariableOrderedSetWidget::removeSlot()
{
    assert(list_widget_);
    int index = list_widget_->currentRow();

    loginf << "VariableOrderedSetWidget: remove: index " << index;

    if (index < 0)
        return;

    set_.removeVariableAt(index);
    current_index_ = -1;
}

void VariableOrderedSetWidget::moveUpSlot()
{
    assert(list_widget_);
    int index = list_widget_->currentRow();
    loginf << "VariableOrderedSetWidget: moveUp: index " << index;

    if (index <= 0)
        return;

    set_.moveVariableUp(index);

    current_index_ = index - 1;
    list_widget_->setCurrentRow(current_index_);
}
void VariableOrderedSetWidget::moveDownSlot()
{
    assert(list_widget_);
    int index = list_widget_->currentRow();
    loginf << "VariableOrderedSetWidget: moveDown: index " << index;

    if (index < 0 || index == (int)set_.getSize() - 1)
        return;

    set_.moveVariableDown(index);

    current_index_ = index + 1;
    list_widget_->setCurrentRow(current_index_);
}

void VariableOrderedSetWidget::updateVariableListSlot()
{
    logdbg << "VariableOrderedSetWidget: updateVariableListSlot";

    assert(list_widget_);

    list_widget_->clear();

    logdbg << "VariableOrderedSetWidget: updateVariableListSlot: clear done";

    const std::map<unsigned int, VariableOrderDefinition*>& variables = set_.definitions();
    std::map<unsigned int, VariableOrderDefinition*>::const_iterator it;

    DBContentManager& manager = COMPASS::instance().dbContentManager();
    VariableOrderDefinition* def = nullptr;

    string tooltip;

    for (it = variables.begin(); it != variables.end(); it++)
    {
        def = it->second;

        if (def->dboName() == META_OBJECT_NAME)
        {
            assert(manager.existsMetaVariable(def->variableName()));
            tooltip = manager.metaVariable(def->variableName()).description();
        }
        else
        {
            assert(manager.existsDBContent(def->dboName()));
            assert(manager.dbContent(def->dboName()).hasVariable(def->variableName()));
            tooltip = manager.dbContent(def->dboName()).variable(def->variableName()).description();
        }

        QListWidgetItem* item = new QListWidgetItem((def->dboName() + ", " + def->variableName()).c_str());
        item->setToolTip(tooltip.c_str());

        list_widget_->addItem(item);
    }

    if (current_index_ != -1)
    {
        logdbg << "VariableOrderedSetWidget: updateVariableListSlot: current index "
               << current_index_;
        list_widget_->setCurrentRow(current_index_);
        current_index_ = -1;
    }
}

}
