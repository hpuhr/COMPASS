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

#include "listboxviewsetconfigwidget.h"






#include "listboxviewwidget.h"
#include "listboxview.h"
//#include "compass.h"
//#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableorderedsetwidget.h"
#include "listboxview.h"
#include "listboxviewdatasource.h"
#include "logger.h"
//#include "stringconv.h"
//#include "test/ui_test_common.h"
#include "viewwidget.h"

#include <QWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPushButton>

#include <QMessageBox>
#include <QInputDialog>

/**
*/
ListBoxViewSetConfigWidget::ListBoxViewSetConfigWidget(ListBoxViewDataSource* data_source, 
                                                       QWidget* parent)
:   QWidget     (parent     )
,   data_source_(data_source)
{
    assert(data_source_);

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    current_set_combo_ = new QComboBox;
    connect(current_set_combo_, QOverload<const QString&>::of(&QComboBox::activated), this, &ListBoxViewSetConfigWidget::setCurrentSet);

    layout->addWidget(current_set_combo_);

    QHBoxLayout* set_manage_layout = new QHBoxLayout();

    add_set_button_ = new QPushButton("Add");
    connect(add_set_button_, &QPushButton::clicked, this, &ListBoxViewSetConfigWidget::addSet);
    set_manage_layout->addWidget(add_set_button_);

    copy_set_button_ = new QPushButton("Copy");
    connect(copy_set_button_, &QPushButton::clicked, this, &ListBoxViewSetConfigWidget::copySet);
    set_manage_layout->addWidget(copy_set_button_);

    rename_set_button_ = new QPushButton("Rename");
    connect(rename_set_button_, &QPushButton::clicked, this, &ListBoxViewSetConfigWidget::renameSet);
    set_manage_layout->addWidget(rename_set_button_);

    remove_set_button_ = new QPushButton("Remove");
    connect(remove_set_button_, &QPushButton::clicked, this, &ListBoxViewSetConfigWidget::removeSet);
    set_manage_layout->addWidget(remove_set_button_);

    updateActiveStates();

    layout->addLayout(set_manage_layout);

    QFrame* line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);

    // set widget
    set_stack_ = new QStackedWidget;
    layout->addWidget(set_stack_);

    updateSetWidget();

    //update ui on set change in data source
    connect(data_source_, &ListBoxViewDataSource::currentSetChangedSignal, this, &ListBoxViewSetConfigWidget::updateFromDataSource);
}

/**
*/
ListBoxViewSetConfigWidget::~ListBoxViewSetConfigWidget() = default;

/**
*/
void ListBoxViewSetConfigWidget::updateSetCombo()
{
    loginf << "ListBoxViewSetConfigWidget: updateSetCombo";

    assert(current_set_combo_);

    current_set_combo_->blockSignals(true);

    current_set_combo_->clear();

    for (const auto& it : data_source_->getSets())
        current_set_combo_->addItem(QString::fromStdString(it.first));

    QString current_set = QString::fromStdString(data_source_->currentSetName());
    assert(current_set_combo_->findText(current_set) >= 0);

    current_set_combo_->setCurrentText(current_set);

    current_set_combo_->blockSignals(false);
}

/**
*/
void ListBoxViewSetConfigWidget::updateActiveStates()
{
    loginf << "ListBoxViewSetConfigWidget: updateActiveStates";

    if (!add_set_button_)
        return;

    bool is_default = (data_source_->currentSetName() == ListBoxView::DefaultSetName);

    rename_set_button_->setDisabled(is_default);
    remove_set_button_->setDisabled(is_default);
}

/**
*/
void ListBoxViewSetConfigWidget::updateSetWidget()
{
    loginf << "ListBoxViewSetConfigWidget: updateSetWidget";

    if (!set_stack_)
    {
        loginf << "ListBoxViewSetConfigWidget: updateSetWidget: no stack";
        return;
    }

    assert (data_source_->hasCurrentSet());

    QWidget* set_widget = data_source_->getSet()->widget();

    bool new_set_added = (set_stack_->indexOf(set_widget) < 0);

    if (new_set_added)
        set_stack_->addWidget(set_widget);

    loginf << "ListBoxViewSetConfigWidget: updateSetWidget: setting '" << data_source_->currentSetName() << "'";

    set_stack_->setCurrentWidget(set_widget);
}

/**
 * Updates all widgets to the new set.
*/
void ListBoxViewSetConfigWidget::updateFromDataSource()
{
    updateSetCombo();
    updateActiveStates();
    updateSetWidget();
}

/**
*/
void ListBoxViewSetConfigWidget::setCurrentSet(const QString& text)
{
    std::string name = text.toStdString();

    loginf << "ListBoxViewSetConfigWidget: setCurrentSet: name '" << name << "'";

    if (data_source_->currentSetName() == name)
        return;

    //set data source to new text
    assert (data_source_->hasSet(name));
    data_source_->currentSetName(name, true); //triggers ui update via set change

    //notify changes
    emit changed();
}

/**
*/
void ListBoxViewSetConfigWidget::addSet()
{
    loginf << "ListBoxViewSetConfigWidget: addSet";

    bool ok;
    QString name = QInputDialog::getText(nullptr, tr("Add Variable List"), tr("Specify a (unique) name:"), QLineEdit::Normal, "", &ok);
    if (!ok)
        return;

    if (name.isEmpty())
    {
        QMessageBox::warning(this, tr("Add Variable List"), tr("List has to have a non-empty name."));
        return;
    }
    if (data_source_->hasSet(name.toStdString()))
    {
        QMessageBox::warning(this, tr("Add Variable List"), tr("List with this name already exists."));
        return;
    }

    data_source_->addSet(name.toStdString());
    data_source_->currentSetName(name.toStdString(), false); //triggers ui update via set change

    emit changed();
}

/**
*/
void ListBoxViewSetConfigWidget::copySet()
{
    loginf << "ListBoxViewSetConfigWidget: copySet";

    bool ok;
    QString name = QInputDialog::getText(nullptr, tr("Copy Variable List"), tr("Specify a (unique) name:"), QLineEdit::Normal, "", &ok);
    if (!ok)
        return;

    if (name.isEmpty())
    {
        QMessageBox::warning(this, tr("Copy Variable List"), tr("List has to have a non-empty name."));
        return;
    }
    if (data_source_->hasSet(name.toStdString()))
    {
        QMessageBox::warning(this, tr("Copy Variable List"), tr("List with this name already exists."));
        return;
    }

    data_source_->copySet(data_source_->currentSetName(), name.toStdString());
    data_source_->currentSetName(name.toStdString(), false); //triggers ui update via set change

    emit changed();
}

/**
*/
void ListBoxViewSetConfigWidget::renameSet()
{
    loginf << "ListBoxViewSetConfigWidget: renameSet";

    bool ok;
    QString name = QInputDialog::getText(nullptr, tr("Rename Variable List"), tr("Specify a (unique) name:"), QLineEdit::Normal, "", &ok);
    if (!ok)
        return;

    if (name.isEmpty())
    {
        QMessageBox::warning(this, tr("Rename Variable List"), tr("List has to have a non-empty name."));
        return;
    }
    if (data_source_->hasSet(name.toStdString()))
    {
        QMessageBox::warning(this, tr("Rename Variable List"), tr("List with this name already exists."));
        return;
    }

    std::string old_name = data_source_->currentSetName();

    data_source_->copySet(old_name, name.toStdString());
    data_source_->currentSetName(name.toStdString(), false);
    data_source_->removeSet(old_name);

    updateFromDataSource();
    
    emit changed();
}

/**
*/
void ListBoxViewSetConfigWidget::removeSet()
{
    loginf << "ListBoxViewSetConfigWidget: removeSet";

    data_source_->removeSet(data_source_->currentSetName()); //triggers ui update via set change

    emit changed();
}
