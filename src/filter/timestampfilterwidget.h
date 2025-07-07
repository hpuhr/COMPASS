
#pragma once

#include "dbfilterwidget.h"
#include "timestampfilter.h"

class QDateTimeEdit;

/**
 */
class TimestampFilterWidget : public DBFilterWidget
{
    Q_OBJECT

public slots:
    void minDateTimeChanged(const QDateTime& datetime);
    void maxDateTimeChanged(const QDateTime& datetime);

public:
    TimestampFilterWidget(TimestampFilter& filter);
    virtual ~TimestampFilterWidget();

    virtual void update();

protected:
    TimestampFilter& filter_;

    QDateTimeEdit* min_edit_ {nullptr};
    QDateTimeEdit* max_edit_ {nullptr};

    bool update_active_ {false};
};
