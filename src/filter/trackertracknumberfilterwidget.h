
#pragma once

#include "dbfilterwidget.h"
#include "trackertracknumberfilter.h"

/**
 */
class TrackerTrackNumberFilterWidget : public DBFilterWidget
{
    Q_OBJECT

public slots:
    void valueEditedSlot(const QString& value);

public:
    TrackerTrackNumberFilterWidget(TrackerTrackNumberFilter& filter);
    virtual ~TrackerTrackNumberFilterWidget();

    virtual void update();

protected:
    TrackerTrackNumberFilter& filter_;
};
