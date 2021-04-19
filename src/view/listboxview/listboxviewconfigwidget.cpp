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

#include "listboxviewconfigwidget.h"
#include "dbobjectmanager.h"
#include "dbovariableorderedsetwidget.h"
#include "listboxview.h"
#include "listboxviewdatasource.h"
#include "logger.h"
#include "stringconv.h"

#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QInputDialog>
#include <QTabWidget>

using namespace Utils;
using namespace std;

ListBoxViewConfigWidget::ListBoxViewConfigWidget(ListBoxView* view, QWidget* parent)
    : QWidget(parent), view_(view)
{
    QTabWidget* tab_widget = new QTabWidget(this);
    tab_widget->setStyleSheet("QTabBar::tab { height: 42px; }");

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(0, 0, 0, 0);

    assert(view_);

    // config
    {
        QWidget* cfg_widget = new QWidget();
        QVBoxLayout* cfg_layout = new QVBoxLayout();

        QFont font_bold;
        font_bold.setBold(true);


        // sets

        QLabel* set_label = new QLabel("Variable Lists");
        set_label->setFont(font_bold);
        cfg_layout->addWidget(set_label);

        set_box_ = new QComboBox();
        connect(set_box_, SIGNAL(activated(const QString&)), this, SLOT(selectedSetSlot(const QString&)));

        updateSetBox();

        cfg_layout->addWidget(set_box_);

        QHBoxLayout* set_manage_layout = new QHBoxLayout();

        add_set_button_ = new QPushButton("Add");
        connect(add_set_button_, &QPushButton::clicked, this, &ListBoxViewConfigWidget::addSetSlot);
        set_manage_layout->addWidget(add_set_button_);

        copy_set_button_ = new QPushButton("Copy");
        connect(copy_set_button_, &QPushButton::clicked, this, &ListBoxViewConfigWidget::copySetSlot);
        set_manage_layout->addWidget(copy_set_button_);

        rename_set_button_ = new QPushButton("Rename");
        connect(rename_set_button_, &QPushButton::clicked, this, &ListBoxViewConfigWidget::renameSetSlot);
        set_manage_layout->addWidget(rename_set_button_);

        remove_set_button_ = new QPushButton("Remove");
        connect(remove_set_button_, &QPushButton::clicked, this, &ListBoxViewConfigWidget::removeSetSlot);
        set_manage_layout->addWidget(remove_set_button_);

        updateSetButtons();

        cfg_layout->addLayout(set_manage_layout);

        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        cfg_layout->addWidget(line);

        // set widget
        set_stack_ = new QStackedWidget();
        cfg_layout->addWidget(set_stack_);

        updateSetWidget();

        //    variable_set_widget_ = view_->getDataSource()->getSet()->widget();
        //    connect(view_->getDataSource()->getSet(), &DBOVariableOrderedSet::variableAddedChangedSignal,
        //            this, &ListBoxViewConfigWidget::reloadWantedSlot);
        //    vlayout->addWidget(variable_set_widget_);

        QFrame* line2 = new QFrame();
        line2->setFrameShape(QFrame::HLine);
        line2->setFrameShadow(QFrame::Sunken);
        cfg_layout->addWidget(line2);

        // rest

        only_selected_check_ = new QCheckBox("Show Only Selected");
        only_selected_check_->setChecked(view_->showOnlySelected());
        connect(only_selected_check_, &QCheckBox::clicked, this,
                &ListBoxViewConfigWidget::toggleShowOnlySeletedSlot);
        cfg_layout->addWidget(only_selected_check_);

        presentation_check_ = new QCheckBox("Use Presentation");
        presentation_check_->setChecked(view_->usePresentation());
        connect(presentation_check_, &QCheckBox::clicked, this,
                &ListBoxViewConfigWidget::toggleUsePresentation);
        cfg_layout->addWidget(presentation_check_);

        associations_check_ = new QCheckBox("Show Associations");
        associations_check_->setChecked(view_->showAssociations());
        connect(associations_check_, &QCheckBox::clicked, this,
                &ListBoxViewConfigWidget::showAssociationsSlot);
        if (!view_->canShowAssociations())
            associations_check_->setDisabled(true);
        cfg_layout->addWidget(associations_check_);

        //vlayout->addStretch();

        overwrite_check_ = new QCheckBox("Overwrite Exported File");
        overwrite_check_->setChecked(view_->overwriteCSV());
        connect(overwrite_check_, &QCheckBox::clicked, this,
                &ListBoxViewConfigWidget::toggleUseOverwrite);
        cfg_layout->addWidget(overwrite_check_);

        cfg_widget->setLayout(cfg_layout);

        tab_widget->addTab(cfg_widget, "Config");
    }
    //    QFrame* line3 = new QFrame();
    //    line3->setFrameShape(QFrame::HLine);
    //    line3->setFrameShadow(QFrame::Sunken);
    //    vlayout->addWidget(line3);

    vlayout->addWidget(tab_widget);

    export_button_ = new QPushButton("Export");
    connect(export_button_, SIGNAL(clicked(bool)), this, SLOT(exportSlot()));
    vlayout->addWidget(export_button_);

    QFont font_status;
    font_status.setItalic(true);

    status_label_ = new QLabel();
    status_label_->setFont(font_status);
    status_label_->setVisible(false);
    vlayout->addWidget(status_label_);

    update_button_ = new QPushButton("Reload");
    connect(update_button_, &QPushButton::clicked, this,
            &ListBoxViewConfigWidget::reloadRequestedSlot);
    update_button_->setEnabled(reload_needed_);
    vlayout->addWidget(update_button_);

    setLayout(vlayout);

    setStatus("No Data Loaded", true);
}

ListBoxViewConfigWidget::~ListBoxViewConfigWidget() {}

void ListBoxViewConfigWidget::setStatus (const std::string& status, bool visible, QColor color)
{
    assert (status_label_);
    status_label_->setText(status.c_str());
    //status_label_->setStyleSheet("QLabel { color : "+color.name()+"; }");

    QPalette palette = status_label_->palette();
    palette.setColor(status_label_->foregroundRole(), color);
    status_label_->setPalette(palette);

    status_label_->setVisible(visible);
}

void ListBoxViewConfigWidget::selectedSetSlot(const QString& text)
{
    string name = text.toStdString();

    loginf << "ListBoxViewConfigWidget: selectedSetSlot: name '" << name << "'";

    assert (view_->getDataSource()->hasSet(name));
    view_->getDataSource()->currentSetName(name);

    updateSetButtons();
    updateSetWidget();

    reloadWantedSlot();
}

void ListBoxViewConfigWidget::addSetSlot()
{
    loginf << "istBoxViewConfigWidget: addSetSlot";

    bool ok;
    QString text =
            QInputDialog::getText(nullptr, tr("Add Variable List"),
                                  tr("Specify a (unique) name:"), QLineEdit::Normal,
                                  "", &ok);

    if (!ok)
        return;

    std::string name;

    if (!text.isEmpty())
    {
        name = text.toStdString();
        if (!name.size())
        {
            QMessageBox m_warning(QMessageBox::Warning, "Add Variable List",
                                  "List has to have a non-empty name.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        if (view_->getDataSource()->hasSet(name))
        {
            QMessageBox m_warning(QMessageBox::Warning, "Add Variable List",
                                  "List with this name already exists.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }
    }

    view_->getDataSource()->addSet(name);
    view_->getDataSource()->currentSetName(name);

    updateSetBox();
}

void ListBoxViewConfigWidget::copySetSlot()
{
    loginf << "istBoxViewConfigWidget: copySetSlot";

    bool ok;
    QString text =
            QInputDialog::getText(nullptr, tr("Copy Variable List"),
                                  tr("Specify a (unique) name:"), QLineEdit::Normal,
                                  "", &ok);

    if (!ok)
        return;

    std::string new_name;

    if (!text.isEmpty())
    {
        new_name = text.toStdString();
        if (!new_name.size())
        {
            QMessageBox m_warning(QMessageBox::Warning, "Copy Variable List",
                                  "List has to have a non-empty name.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        if (view_->getDataSource()->hasSet(new_name))
        {
            QMessageBox m_warning(QMessageBox::Warning, "Copy Variable List",
                                  "List with this name already exists.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }
    }

    view_->getDataSource()->copySet(view_->getDataSource()->currentSetName(), new_name);
    view_->getDataSource()->currentSetName(new_name);

    updateSetBox();
}

void ListBoxViewConfigWidget::renameSetSlot()
{
    loginf << "istBoxViewConfigWidget: renameSetSlot";

    bool ok;
    QString text =
            QInputDialog::getText(nullptr, tr("Rename Variable List"),
                                  tr("Specify a (unique) name:"), QLineEdit::Normal,
                                  "", &ok);

    if (!ok)
        return;

    std::string new_name;

    if (!text.isEmpty())
    {
        new_name = text.toStdString();
        if (!new_name.size())
        {
            QMessageBox m_warning(QMessageBox::Warning, "Rename Variable List",
                                  "List has to have a non-empty name.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        if (view_->getDataSource()->hasSet(new_name))
        {
            QMessageBox m_warning(QMessageBox::Warning, "Rename Variable List",
                                  "List with this name already exists.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }
    }

    string old_name = view_->getDataSource()->currentSetName();

    view_->getDataSource()->copySet(view_->getDataSource()->currentSetName(), new_name);
    view_->getDataSource()->currentSetName(new_name);

    view_->getDataSource()->removeSet(old_name);

    updateSetBox();
    reloadWantedSlot();
}

void ListBoxViewConfigWidget::removeSetSlot()
{
    loginf << "istBoxViewConfigWidget: removeSetSlot";

    view_->getDataSource()->removeSet(view_->getDataSource()->currentSetName());

    updateSetBox();
    updateSetWidget();
}

void ListBoxViewConfigWidget::toggleShowOnlySeletedSlot()
{
    assert(only_selected_check_);
    bool checked = only_selected_check_->checkState() == Qt::Checked;
    loginf << "ListBoxViewConfigWidget: toggleShowOnlySeletedSlot: setting to " << checked;
    view_->showOnlySelected(checked);
}

void ListBoxViewConfigWidget::toggleUsePresentation()
{
    assert(presentation_check_);
    bool checked = presentation_check_->checkState() == Qt::Checked;
    logdbg << "ListBoxViewConfigWidget: toggleUsePresentation: setting use presentation to "
           << checked;
    view_->usePresentation(checked);
}

void ListBoxViewConfigWidget::toggleUseOverwrite()
{
    assert(overwrite_check_);
    bool checked = overwrite_check_->checkState() == Qt::Checked;
    logdbg << "ListBoxViewConfigWidget: toggleUseOverwrite: setting overwrite to " << checked;
    view_->overwriteCSV(checked);
}

void ListBoxViewConfigWidget::showAssociationsSlot()
{
    assert(associations_check_);
    bool checked = associations_check_->checkState() == Qt::Checked;
    logdbg << "ListBoxViewConfigWidget: showAssociationsSlot: setting to " << checked;
    view_->showAssociations(checked);
}

void ListBoxViewConfigWidget::exportSlot()
{
    logdbg << "ListBoxViewConfigWidget: exportSlot";
    assert(overwrite_check_);
    assert(export_button_);

    export_button_->setDisabled(true);
    emit exportSignal(overwrite_check_->checkState() == Qt::Checked);
}

void ListBoxViewConfigWidget::exportDoneSlot(bool cancelled)
{
    assert(export_button_);

    export_button_->setDisabled(false);

    if (!cancelled)
    {
        QMessageBox msgBox;
        msgBox.setText("Export complete.");
        msgBox.exec();
    }
}

void ListBoxViewConfigWidget::reloadWantedSlot()
{
    reload_needed_ = true;
    updateUpdateButton();
}

void ListBoxViewConfigWidget::reloadRequestedSlot()
{
    assert(reload_needed_);
    emit reloadRequestedSignal();
    reload_needed_ = false;

    updateUpdateButton();
}

void ListBoxViewConfigWidget::updateUpdateButton()
{
    assert(update_button_);
    update_button_->setEnabled(reload_needed_);
}

void ListBoxViewConfigWidget::updateSetBox()
{
    loginf << "ListBoxViewConfigWidget: updateSetBox";

    assert(set_box_);
    set_box_->clear();

    for (const auto& set_it : view_->getDataSource()->getSets())
    {
        set_box_->addItem(set_it.first.c_str());
    }

    int index = set_box_->findText(view_->getDataSource()->currentSetName().c_str());
    if(index >= 0)
    {
        set_box_->setCurrentIndex(index);
        loginf << "ListBoxViewConfigWidget: updateSetBox: setting '"
               << view_->getDataSource()->currentSetName() << "'";
    }
}

void ListBoxViewConfigWidget::updateSetButtons()
{
    loginf << "ListBoxViewConfigWidget: updateSetButtons";

    if (!add_set_button_)
        return;

    bool is_default = view_->getDataSource()->currentSetName() == "Default";

    assert (add_set_button_);
    assert (copy_set_button_);
    assert (rename_set_button_);
    rename_set_button_->setDisabled(is_default);
    assert (remove_set_button_);
    remove_set_button_->setDisabled(is_default);
}

void ListBoxViewConfigWidget::updateSetWidget()
{
    loginf << "ListBoxViewConfigWidget: updateWidgetBox";

    if (!set_stack_)
    {
        loginf << "ListBoxViewConfigWidget: updateWidgetBox: no stack";
        return;
    }

    //    while (set_stack_->count() > 0)  // remove all widgets
    //        set_stack_->removeWidget(set_stack_->widget(0));
    //    return;

    assert (view_->getDataSource()->hasCurrentSet());

    connect(view_->getDataSource()->getSet(), &DBOVariableOrderedSet::variableAddedChangedSignal,
            this, &ListBoxViewConfigWidget::reloadWantedSlot, Qt::UniqueConnection);

    QWidget* set_widget = view_->getDataSource()->getSet()->widget();

    if (set_stack_->indexOf(set_widget) < 0)
        set_stack_->addWidget(set_widget);

    loginf << "ListBoxViewConfigWidget: updateWidgetBox: setting '" << view_->getDataSource()->currentSetName() << "'";
    set_stack_->setCurrentWidget(set_widget);
}

void ListBoxViewConfigWidget::loadingStartedSlot()
{
    reload_needed_ = false;

    updateUpdateButton();
    setStatus("Loading Data", true);
}
