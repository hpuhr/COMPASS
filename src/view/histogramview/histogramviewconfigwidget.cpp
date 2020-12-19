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

#include "histogramviewconfigwidget.h"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "dbobjectmanager.h"
#include "dbovariableorderedsetwidget.h"
#include "histogramview.h"
#include "histogramviewdatasource.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;

HistogramViewConfigWidget::HistogramViewConfigWidget(HistogramView* view, QWidget* parent)
    : QWidget(parent), view_(view)
{
    QVBoxLayout* vlayout = new QVBoxLayout;

    assert(view_);

//    variable_set_widget_ = view_->getDataSource()->getSet()->widget();
//    connect(view_->getDataSource()->getSet(), &DBOVariableOrderedSet::variableAddedChangedSignal,
//            this, &HistogramViewConfigWidget::reloadWantedSlot);
//    vlayout->addWidget(variable_set_widget_);

//    only_selected_check_ = new QCheckBox("Show Only Selected");
//    only_selected_check_->setChecked(view_->showOnlySelected());
//    connect(only_selected_check_, &QCheckBox::clicked, this,
//            &HistogramViewConfigWidget::toggleShowOnlySeletedSlot);
//    vlayout->addWidget(only_selected_check_);

//    presentation_check_ = new QCheckBox("Use Presentation");
//    presentation_check_->setChecked(view_->usePresentation());
//    connect(presentation_check_, &QCheckBox::clicked, this,
//            &HistogramViewConfigWidget::toggleUsePresentation);
//    vlayout->addWidget(presentation_check_);

//    associations_check_ = new QCheckBox("Show Associations");
//    associations_check_->setChecked(view_->showAssociations());
//    connect(associations_check_, &QCheckBox::clicked, this,
//            &HistogramViewConfigWidget::showAssociationsSlot);
//    if (!view_->canShowAssociations())
//        associations_check_->setDisabled(true);
//    vlayout->addWidget(associations_check_);

//    vlayout->addStretch();

//    overwrite_check_ = new QCheckBox("Overwrite Exported File");
//    overwrite_check_->setChecked(view_->overwriteCSV());
//    connect(overwrite_check_, &QCheckBox::clicked, this,
//            &HistogramViewConfigWidget::toggleUseOverwrite);
//    vlayout->addWidget(overwrite_check_);

    export_button_ = new QPushButton("Export");
    connect(export_button_, SIGNAL(clicked(bool)), this, SLOT(exportSlot()));
    vlayout->addWidget(export_button_);

    vlayout->addStretch();

    update_button_ = new QPushButton("Reload");
    connect(update_button_, &QPushButton::clicked, this,
            &HistogramViewConfigWidget::reloadRequestedSlot);
    update_button_->setDisabled(true);
    vlayout->addWidget(update_button_);

    setLayout(vlayout);
}

HistogramViewConfigWidget::~HistogramViewConfigWidget() {}

//void HistogramViewConfigWidget::toggleShowOnlySeletedSlot()
//{
//    assert(only_selected_check_);
//    bool checked = only_selected_check_->checkState() == Qt::Checked;
//    loginf << "HistogramViewConfigWidget: toggleShowOnlySeletedSlot: setting to " << checked;
//    view_->showOnlySelected(checked);
//}

//void HistogramViewConfigWidget::toggleUsePresentation()
//{
//    assert(presentation_check_);
//    bool checked = presentation_check_->checkState() == Qt::Checked;
//    logdbg << "HistogramViewConfigWidget: toggleUsePresentation: setting use presentation to "
//           << checked;
//    view_->usePresentation(checked);
//}

//void HistogramViewConfigWidget::toggleUseOverwrite()
//{
//    assert(overwrite_check_);
//    bool checked = overwrite_check_->checkState() == Qt::Checked;
//    logdbg << "HistogramViewConfigWidget: toggleUseOverwrite: setting overwrite to " << checked;
//    view_->overwriteCSV(checked);
//}

//void HistogramViewConfigWidget::showAssociationsSlot()
//{
//    assert(associations_check_);
//    bool checked = associations_check_->checkState() == Qt::Checked;
//    logdbg << "HistogramViewConfigWidget: showAssociationsSlot: setting to " << checked;
//    view_->showAssociations(checked);
//}

void HistogramViewConfigWidget::exportSlot()
{
    logdbg << "HistogramViewConfigWidget: exportSlot";
    //assert(overwrite_check_);
    assert(export_button_);

    export_button_->setDisabled(true);
    //emit exportSignal(overwrite_check_->checkState() == Qt::Checked);
}

void HistogramViewConfigWidget::exportDoneSlot(bool cancelled)
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

void HistogramViewConfigWidget::reloadWantedSlot()
{
    reload_needed_ = true;
    updateUpdateButton();
}

void HistogramViewConfigWidget::reloadRequestedSlot()
{
    assert(reload_needed_);
    emit reloadRequestedSignal();
    reload_needed_ = false;

    updateUpdateButton();
}

void HistogramViewConfigWidget::updateUpdateButton()
{
    assert(update_button_);
    update_button_->setEnabled(reload_needed_);
}

void HistogramViewConfigWidget::loadingStartedSlot()
{
    reload_needed_ = false;

    updateUpdateButton();
}
