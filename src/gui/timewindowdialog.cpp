#include "timewindowdialog.h"
#include "timeconv.h"
#include "logger.h"

#include <QDialogButtonBox>

// const std::string time_format{"yyyy-MM-dd HH:mm:sss"};
// const std::string time_format_long{"yyyy-MM-dd HH:mm:sss.zzz"};

using namespace Utils;

TimeWindowDialog::TimeWindowDialog(
    QWidget* parent, const boost::posix_time::ptime& begin, const boost::posix_time::ptime& end)
    : QDialog(parent)
{
    loginf << "TimeWindowDialog: ctor: begin " << Time::toString(begin) << " end " << Time::toString(end);

    begin_edit_ = new QDateTimeEdit(this);
    end_edit_ = new QDateTimeEdit(this);

    begin_edit_->setDisplayFormat(Time::QT_DATETIME_FORMAT_SHORT.c_str());
    end_edit_->setDisplayFormat(Time::QT_DATETIME_FORMAT_SHORT.c_str());

    if (!begin.is_not_a_date_time())
        begin_edit_->setDateTime(Time::qtFrom(begin, false));
    if (!end.is_not_a_date_time())
        end_edit_->setDateTime(Time::qtFrom(end, false));

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(begin_edit_);
    layout->addWidget(end_edit_);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

boost::posix_time::ptime TimeWindowDialog::begin() const
{
    return boost::posix_time::time_from_string(begin_edit_->dateTime().toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString());
}

boost::posix_time::ptime TimeWindowDialog::end() const
{
    return boost::posix_time::time_from_string(end_edit_->dateTime().toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString());
}
