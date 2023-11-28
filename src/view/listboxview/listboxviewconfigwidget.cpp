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
#include "listboxviewwidget.h"
#include "listboxview.h"
#include "listboxviewsetconfigwidget.h"

#include "logger.h"
#include "viewwidget.h"

#include <QCheckBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTabWidget>

using namespace Utils;
using namespace std;

ListBoxViewConfigWidget::ListBoxViewConfigWidget(ListBoxViewWidget* view_widget, QWidget* parent)
:   ViewConfigWidget(view_widget, parent)
{
    QTabWidget* tab_widget = new QTabWidget(this);
    tab_widget->setStyleSheet("QTabBar::tab { height: 42px; }");

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(0, 0, 0, 0);

    view_ = view_widget->getView();
    assert(view_);

    // config
    {
        QFont font_bold;
        font_bold.setBold(true);

        QWidget* cfg_widget = new QWidget();
        QVBoxLayout* cfg_layout = new QVBoxLayout();

        QLabel* set_label = new QLabel("Variable Lists");
        set_label->setFont(font_bold);
        cfg_layout->addWidget(set_label);

        set_config_widget_ = new ListBoxViewSetConfigWidget(view_->getDataSource(), this);
        set_config_widget_->updateFromDataSource();

        cfg_layout->addWidget(set_config_widget_);

        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        cfg_layout->addWidget(line);

        only_selected_check_ = new QCheckBox("Show Only Selected");
        only_selected_check_->setChecked(view_->showOnlySelected());
        connect(only_selected_check_, &QCheckBox::clicked, this, &ListBoxViewConfigWidget::toggleShowOnlySeletedSlot);
        cfg_layout->addWidget(only_selected_check_);

        presentation_check_ = new QCheckBox("Use Presentation");
        presentation_check_->setChecked(view_->usePresentation());
        connect(presentation_check_, &QCheckBox::clicked, this, &ListBoxViewConfigWidget::toggleUsePresentation);
        cfg_layout->addWidget(presentation_check_);

        //vlayout->addStretch();

        cfg_widget->setLayout(cfg_layout);
        tab_widget->addTab(cfg_widget, "Config");
    }

    vlayout->addWidget(tab_widget);

    export_button_ = new QPushButton("Export");
    connect(export_button_, SIGNAL(clicked(bool)), this, SLOT(exportSlot()));
    vlayout->addWidget(export_button_);

    setLayout(vlayout);
}

ListBoxViewConfigWidget::~ListBoxViewConfigWidget() = default;

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

void ListBoxViewConfigWidget::exportSlot()
{
    logdbg << "ListBoxViewConfigWidget: exportSlot";
    assert(export_button_);

    export_button_->setDisabled(true);
    emit exportSignal();
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

void ListBoxViewConfigWidget::configChanged()
{
    assert(view_);

    //update ui for var set
    set_config_widget_->updateFromDataSource();

    //other ui elements
    only_selected_check_->setChecked(view_->showOnlySelected());
    presentation_check_->setChecked(view_->usePresentation());
}
