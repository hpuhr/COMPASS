#ifndef TRACKERTRACKNUMBERFILTERWIDGET_H
#define TRACKERTRACKNUMBERFILTERWIDGET_H


#include "dbfilterwidget.h"
#include "trackertracknumberfilter.h"


class QFormLayout;

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

    QFormLayout* main_layout_ {nullptr};
};



#endif // TRACKERTRACKNUMBERFILTERWIDGET_H
