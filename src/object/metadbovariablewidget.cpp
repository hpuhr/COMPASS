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

#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>

#include "configuration.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "metadbovariablewidget.h"
#include "metadbovariable.h"
#include "dbovariableselectionwidget.h"
#include "logger.h"
#include "atsdb.h"

#include "stringconv.h"

using namespace Utils;

MetaDBOVariableWidget::MetaDBOVariableWidget(MetaDBOVariable& variable, QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f), variable_(variable)
{
    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout* main_layout = new QVBoxLayout ();

    QLabel* main_label = new QLabel ("Edit Meta DB object variable");
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    // object parameters
    QGridLayout* properties_layout = new QGridLayout ();

    unsigned int row=0;
    QLabel* name_label = new QLabel ("Name");
    properties_layout->addWidget(name_label, row, 0);

    name_edit_ = new QLineEdit (variable_.name().c_str());
    connect(name_edit_, SIGNAL( returnPressed() ), this, SLOT( editNameSlot() ));
    properties_layout->addWidget (name_edit_, row, 1);
    row++;

    QLabel* description_label = new QLabel ("Description");
    properties_layout->addWidget(description_label, row, 0);

    description_edit_ = new QLineEdit (variable_.description().c_str());
    connect(description_edit_, SIGNAL( returnPressed() ), this, SLOT( editDescriptionSlot() ));
    properties_layout->addWidget (description_edit_, row, 1);
    row++;

    main_layout->addLayout (properties_layout);

    grid_layout_ = new QGridLayout ();
    main_layout->addLayout (grid_layout_);

    main_layout->addStretch();

    setLayout (main_layout);

    updateSlot ();

    show();
}

MetaDBOVariableWidget::~MetaDBOVariableWidget()
{

}

void MetaDBOVariableWidget::lock ()
{
    if (locked_)
        return;

    locked_ = true;

    setDisabled (true);
}

void MetaDBOVariableWidget::unlock ()
{
    if (!locked_)
        return;

    locked_ = false;

    setDisabled (false);
}

void MetaDBOVariableWidget::editNameSlot ()
{
    logdbg  << "MetaDBOVariableWidget: editName";
    assert (name_edit_);

    std::string text = name_edit_->text().toStdString();
    assert (text.size()>0);
    variable_.name (text);

    emit metaVariableChangedSignal();
}
void MetaDBOVariableWidget::editDescriptionSlot()
{
    logdbg  << "MetaDBOVariableWidget: editDescriptionSlot";
    assert (description_edit_);

    std::string text = description_edit_->text().toStdString();
    assert (text.size()>0);
    variable_.description (text);

    emit metaVariableChangedSignal();
}

void MetaDBOVariableWidget::subVariableChangedSlot ()
{
    DBOVariableSelectionWidget* var_sel = dynamic_cast <DBOVariableSelectionWidget*> (QObject::sender());
    assert (var_sel);
    assert (!var_sel->hasMetaVariable());
    assert (selection_widgets_.count(var_sel) > 0);

    std::string obj_name = selection_widgets_.at(var_sel);

    if (variable_.existsIn(obj_name))
        variable_.removeVariable(obj_name);

    if (var_sel->hasVariable())
    {
        assert (!variable_.existsIn(obj_name));
        DBOVariable& variable = var_sel->selectedVariable();
        variable_.addVariable(obj_name, variable.name());
    }
}

void MetaDBOVariableWidget::updateSlot ()
{
    assert (grid_layout_);

    QLayoutItem *child;
    while ((child = grid_layout_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    selection_widgets_.clear();

    unsigned int row = 0;
    for (auto& obj_it : ATSDB::instance().objectManager())
    {
        grid_layout_->addWidget(new QLabel (obj_it.first.c_str()), row, 0);

        DBOVariableSelectionWidget* var_sel = new DBOVariableSelectionWidget (true);
        var_sel->showDBOOnly(obj_it.first);

        if (variable_.existsIn(obj_it.first))
            var_sel->selectedVariable (variable_.getFor(obj_it.first));

        connect (var_sel, SIGNAL(selectionChanged()), this, SLOT(subVariableChangedSlot()));
        selection_widgets_[var_sel] = obj_it.first;

        grid_layout_->addWidget(var_sel, row, 1);
        row++;
    }
}
