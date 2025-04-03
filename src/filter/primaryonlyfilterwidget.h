
#pragma once

#include "dbfilterwidget.h"
#include "primaryonlyfilter.h"

/**
 */
class PrimaryOnlyFilterWidget : public DBFilterWidget
{
public:
    PrimaryOnlyFilterWidget(PrimaryOnlyFilter& filter);
    virtual ~PrimaryOnlyFilterWidget();

protected:
    PrimaryOnlyFilter& filter_;
};
