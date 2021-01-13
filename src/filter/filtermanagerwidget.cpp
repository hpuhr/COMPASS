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

#include "filtermanagerwidget.h"

#include "dbfilter.h"
#include "dbfilterwidget.h"
#include "datasourcesfilter.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbschemawidget.h"
#include "dbtable.h"
#include "filtergeneratorwidget.h"
#include "filtermanager.h"
#include "global.h"
#include "logger.h"
#include "metadbtable.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QCheckBox>

FilterManagerWidget::FilterManagerWidget(FilterManager& filter_manager, QWidget* parent,
                                         Qt::WindowFlags f)
    : QFrame(parent),
      filter_manager_(filter_manager),
      filter_generator_widget_(nullptr),
      add_button_(nullptr)
{
    unsigned int frame_width = FRAME_SIZE;
    QFont font_bold;
    font_bold.setBold(true);

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(frame_width);

    QVBoxLayout* layout = new QVBoxLayout();

    QLabel* filter_label = new QLabel(tr("Filters"));
    filter_label->setFont(font_bold);
    layout->addWidget(filter_label);

    // use filters stuff
    filters_check_ = new QCheckBox("Use Filters");
    filters_check_->setChecked(filter_manager_.useFilters());
    connect(filters_check_, &QCheckBox::clicked, this, &FilterManagerWidget::toggleUseFilters);
    layout->addWidget(filters_check_);

    QHBoxLayout* filter_layout = new QHBoxLayout();

    QVBoxLayout* ds_filter_parent_layout = new QVBoxLayout();
    ds_filter_layout_ = new QVBoxLayout();

    ds_filter_parent_layout->addLayout(ds_filter_layout_);
    ds_filter_parent_layout->addStretch();

    filter_layout->addLayout(ds_filter_parent_layout);

    other_filter_layout_ = new QVBoxLayout();
    filter_layout->addLayout(other_filter_layout_);

    layout->addLayout(filter_layout);

    layout->addStretch();

    QHBoxLayout* button_layout = new QHBoxLayout();

    add_button_ = new QPushButton(tr("Add Filter"));
    connect(add_button_, SIGNAL(clicked()), this, SLOT(addFilterSlot()));
    button_layout->addWidget(add_button_);

    layout->addLayout(button_layout);

    setLayout(layout);

    setDisabled(true);
}

FilterManagerWidget::~FilterManagerWidget()
{
    if (filter_generator_widget_)
    {
        delete filter_generator_widget_;
        filter_generator_widget_ = nullptr;
    }
}

void FilterManagerWidget::toggleUseFilters()
{
    assert(filters_check_);

    bool checked = filters_check_->checkState() == Qt::Checked;
    logdbg << "FilterManagerWidget: toggleUseFilters: setting use limit to " << checked;
    filter_manager_.useFilters(checked);
}

void FilterManagerWidget::updateUseFilters ()
{
    assert (filters_check_);
    filters_check_->setChecked(filter_manager_.useFilters());
}


void FilterManagerWidget::addFilterSlot()
{
    assert(!filter_generator_widget_);

    filter_generator_widget_ = new FilterGeneratorWidget();
    connect(filter_generator_widget_, SIGNAL(filterWidgetAction(bool)), this,
            SLOT(filterWidgetActionSlot(bool)));
    filter_generator_widget_->show();
}

void FilterManagerWidget::updateFiltersSlot()
{
    assert(ds_filter_layout_);

    QLayoutItem* child;
    while ((child = ds_filter_layout_->takeAt(0)) != 0)
        ds_filter_layout_->removeItem(child);

    assert(other_filter_layout_);
    while ((child = other_filter_layout_->takeAt(0)) != 0)
        other_filter_layout_->removeItem(child);

    std::vector<DBFilter*>& filters = filter_manager_.filters();
    for (auto it : filters)
    {
        loginf << "FilterManagerWidget: updateFiltersSlot: filter " << it->getName();

        if (dynamic_cast<DataSourcesFilter*>(it))
            ds_filter_layout_->addWidget(it->widget());
        else
            other_filter_layout_->addWidget(it->widget());
    }
}

void FilterManagerWidget::filterWidgetActionSlot(bool result)
{
    assert(filter_generator_widget_);

    if (result)
        updateFiltersSlot();

    delete filter_generator_widget_;
    filter_generator_widget_ = nullptr;
}

void FilterManagerWidget::databaseOpenedSlot() { setDisabled(false); }
