#pragma once

#include "dbfilterwidget.h"
#include "excludedtimewindowsfilter.h"
#include "timewindowcollectionwidget.h"

class ExcludedTimeWindowsFilterWidget : public DBFilterWidget
{
    Q_OBJECT

public:
    ExcludedTimeWindowsFilterWidget(ExcludedTimeWindowsFilter& filter);
    virtual ~ExcludedTimeWindowsFilterWidget();

    virtual void update();

public:
    ExcludedTimeWindowsFilter& filter_;
    TimeWindowCollectionWidget* tw_widget_{nullptr};
};

