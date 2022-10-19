#include "trackertracknumberfilterwidget.h"
#include "timeconv.h"
#include "datasourcemanager.h"
#include "compass.h"
#include "logger.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QDateTimeEdit>

using namespace std;
using namespace Utils;

TrackerTrackNumberFilterWidget::TrackerTrackNumberFilterWidget(TrackerTrackNumberFilter& filter)
    : DBFilterWidget(filter), filter_(filter)
{
    main_layout_ = new QFormLayout();

    child_layout_->addLayout(main_layout_);

    update();
}

TrackerTrackNumberFilterWidget::~TrackerTrackNumberFilterWidget()
{
}

void TrackerTrackNumberFilterWidget::update()
{
    loginf << "TrackerTrackNumberFilterWidget: update";

    assert (main_layout_);

    DBFilterWidget::update();

    QLayoutItem* child;
    while ((child = main_layout_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    // ds_id -> values
    std::map<unsigned int, std::string> active_values = filter_.getActiveTrackerTrackNums();
    std::string ds_name;

    for (auto& val_it : active_values)
    {
        assert (ds_man.hasDBDataSource(val_it.first));
        ds_name = ds_man.dbDataSource(val_it.first).name();

        QLineEdit* value_edit = new QLineEdit(val_it.second.c_str());
        value_edit->setProperty("ds_id", val_it.first);
        connect(value_edit, &QLineEdit::textEdited, this, &TrackerTrackNumberFilterWidget::valueEditedSlot);
        main_layout_->addRow((ds_name + " Track Number IN").c_str(), value_edit);
    }
}


void TrackerTrackNumberFilterWidget::valueEditedSlot(const QString& value)
{
    QLineEdit* value_edit = dynamic_cast<QLineEdit*> (sender());
    assert (value_edit);

    unsigned int ds_id = value_edit->property("ds_id").toUInt();

    loginf << "TrackerTrackNumberFilterWidget: valueEditedSlot: ds_id " << ds_id
           << " value '" << value.toStdString() << "'";

    filter_.setTrackerTrackNum(ds_id, value.toStdString());
}

//void TrackerTrackNumberFilterWidget::minDateTimeChanged(const QDateTime& datetime)
//{
//    if (update_active_)
//        return;

//    loginf << "TrackerTrackNumberFilterWidget: minDateTimeChanged: value "
//           << datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString();

//    filter_.minValue(Time::fromString(datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString()), false);
//}

//void TrackerTrackNumberFilterWidget::maxDateTimeChanged(const QDateTime& datetime)
//{
//    if (update_active_)
//        return;

//    loginf << "TrackerTrackNumberFilterWidget: maxDateTimeChanged: value "
//           << datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString();

//    filter_.maxValue(Time::fromString(datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString()), false);
//}
