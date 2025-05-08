#include "excludedtimewindowsfilterwidget.h"

#include <QLabel>

ExcludedTimeWindowsFilterWidget::ExcludedTimeWindowsFilterWidget(ExcludedTimeWindowsFilter& filter)
    : DBFilterWidget(filter), filter_(filter)
{
    tw_widget_ = new TimeWindowCollectionWidget(filter_.timeWindows());

    int insert_row = child_layout_->rowCount();

    auto lwidget = new QLabel("Time Windows");
    lwidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    tw_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    child_layout_->addWidget(lwidget, insert_row, 0);
    child_layout_->addWidget(tw_widget_ , insert_row, 1);
}

ExcludedTimeWindowsFilterWidget::~ExcludedTimeWindowsFilterWidget()
{
    tw_widget_ = nullptr;
}

void ExcludedTimeWindowsFilterWidget::update()
{
    DBFilterWidget::update();

    tw_widget_->refreshList();
}
