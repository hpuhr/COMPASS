#ifndef PRIMARYONLYFILTERWIDGET_H
#define PRIMARYONLYFILTERWIDGET_H

#include "dbfilterwidget.h"
#include "primaryonlyfilter.h"

class PrimaryOnlyFilterWidget : public DBFilterWidget
{
public:
    PrimaryOnlyFilterWidget(PrimaryOnlyFilter& filter);
    virtual ~PrimaryOnlyFilterWidget();

protected:
    PrimaryOnlyFilter& filter_;
};

#endif // PRIMARYONLYFILTERWIDGET_H
