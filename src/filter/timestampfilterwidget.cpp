#include "timestampfilterwidget.h"
#include "timeconv.h"
#include "logger.h"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QDateTimeEdit>

using namespace std;
using namespace Utils;

TimestampFilterWidget::TimestampFilterWidget(TimestampFilter& filter)
    : DBFilterWidget(filter), filter_(filter)
{
    QFormLayout* layout = new QFormLayout();

    min_edit_ = new QDateTimeEdit(QDateTime::currentDateTime());
    min_edit_->setDisplayFormat(Time::QT_DATETIME_FORMAT.c_str());
    connect(min_edit_, &QDateTimeEdit::dateTimeChanged, this, &TimestampFilterWidget::minDateTimeChanged);
    layout->addRow("Timestamp >=", min_edit_);

    max_edit_ = new QDateTimeEdit(QDateTime::currentDateTime());
    max_edit_->setDisplayFormat(Time::QT_DATETIME_FORMAT.c_str());
    connect(max_edit_, &QDateTimeEdit::dateTimeChanged, this, &TimestampFilterWidget::maxDateTimeChanged);
    layout->addRow("Timestamp <=", max_edit_);

    child_layout_->addLayout(layout);

    update();
}

TimestampFilterWidget::~TimestampFilterWidget()
{
}

void TimestampFilterWidget::update()
{
    loginf << "TimestampFilterWidget: update";

    update_active_ = true;

    DBFilterWidget::update();

    assert (min_edit_);
    assert (max_edit_);

    min_edit_->setDateTime(QDateTime::fromString(Time::toString(filter_.minValue()).c_str(),
                                                 Time::QT_DATETIME_FORMAT.c_str()));
    max_edit_->setDateTime(QDateTime::fromString(Time::toString(filter_.maxValue()).c_str(),
                                                 Time::QT_DATETIME_FORMAT.c_str()));

    update_active_ = false;
}


void TimestampFilterWidget::minDateTimeChanged(const QDateTime& datetime)
{
    if (update_active_)
        return;

    loginf << "TimestampFilterWidget: minDateTimeChanged: value "
           << datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString();

    filter_.minValue(Time::fromString(datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString()), false);
}

void TimestampFilterWidget::maxDateTimeChanged(const QDateTime& datetime)
{
    if (update_active_)
        return;

    loginf << "TimestampFilterWidget: maxDateTimeChanged: value "
           << datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString();

    filter_.maxValue(Time::fromString(datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString()), false);
}
