#pragma once

#include <QDialog>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <boost/date_time/posix_time/posix_time.hpp>

class TimeWindowDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimeWindowDialog(QWidget* parent = nullptr,
                              const boost::posix_time::ptime& begin = boost::posix_time::not_a_date_time,
                              const boost::posix_time::ptime& end = boost::posix_time::not_a_date_time);

    boost::posix_time::ptime begin() const;
    boost::posix_time::ptime end() const;

private:
    QDateTimeEdit* begin_edit_;
    QDateTimeEdit* end_edit_;
};
