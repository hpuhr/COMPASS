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

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "dbobjectmanager.h"
#include "dbovariableorderedsetwidget.h"
#include "listboxview.h"
#include "listboxviewdatasource.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;

ListBoxViewConfigWidget::ListBoxViewConfigWidget(ListBoxView* view, QWidget* parent)
    : QWidget(parent), view_(view)
{
    QVBoxLayout* vlayout = new QVBoxLayout;

    assert(view_);

    variable_set_widget_ = view_->getDataSource()->getSet()->widget();
    connect(view_->getDataSource()->getSet(), &DBOVariableOrderedSet::variableAddedChangedSignal,
            this, &ListBoxViewConfigWidget::reloadWantedSlot);
    vlayout->addWidget(variable_set_widget_);

    only_selected_check_ = new QCheckBox("Show Only Selected");
    only_selected_check_->setChecked(view_->showOnlySelected());
    connect(only_selected_check_, &QCheckBox::clicked, this,
            &ListBoxViewConfigWidget::toggleShowOnlySeletedSlot);
    vlayout->addWidget(only_selected_check_);

    presentation_check_ = new QCheckBox("Use Presentation");
    presentation_check_->setChecked(view_->usePresentation());
    connect(presentation_check_, &QCheckBox::clicked, this,
            &ListBoxViewConfigWidget::toggleUsePresentation);
    vlayout->addWidget(presentation_check_);

    associations_check_ = new QCheckBox("Show Associations");
    associations_check_->setChecked(view_->showAssociations());
    connect(associations_check_, &QCheckBox::clicked, this,
            &ListBoxViewConfigWidget::showAssociationsSlot);
    if (!view_->canShowAssociations())
        associations_check_->setDisabled(true);
    vlayout->addWidget(associations_check_);

    vlayout->addStretch();

    overwrite_check_ = new QCheckBox("Overwrite Exported File");
    overwrite_check_->setChecked(view_->overwriteCSV());
    connect(overwrite_check_, &QCheckBox::clicked, this,
            &ListBoxViewConfigWidget::toggleUseOverwrite);
    vlayout->addWidget(overwrite_check_);

    export_button_ = new QPushButton("Export");
    connect(export_button_, SIGNAL(clicked(bool)), this, SLOT(exportSlot()));
    vlayout->addWidget(export_button_);

    vlayout->addStretch();

    QFont font_status;
    font_status.setItalic(true);

    status_label_ = new QLabel();
    status_label_->setFont(font_status);
    status_label_->setVisible(false);
    vlayout->addWidget(status_label_);

    update_button_ = new QPushButton("Reload");
    connect(update_button_, &QPushButton::clicked, this,
            &ListBoxViewConfigWidget::reloadRequestedSlot);
    update_button_->setDisabled(true);
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

void ListBoxViewConfigWidget::loadingStartedSlot()
{
    reload_needed_ = false;

    updateUpdateButton();
    setStatus("Loading Data", true);
}
