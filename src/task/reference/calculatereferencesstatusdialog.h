#ifndef CALCULATEREFERENCESSTATUSDIALOG_H
#define CALCULATEREFERENCESSTATUSDIALOG_H

#include <QDialog>


#include "boost/date_time/posix_time/posix_time.hpp"

class QLabel;
class QPushButton;
class QGridLayout;
class CalculateReferencesTask;

class CalculateReferencesStatusDialog : public QDialog
{
    Q_OBJECT

signals:
    void closeSignal();

public slots:
    void okClickedSlot();

public:
    CalculateReferencesStatusDialog(CalculateReferencesTask& task,
                                    QWidget* parent=nullptr, Qt::WindowFlags f=0);

    void markStartTime();
    void updateTime();
    void setDone();

    void setStatus(const std::string& status);

private:
    CalculateReferencesTask& task_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;
    boost::posix_time::time_duration time_diff_;
    std::string elapsed_time_str_;

    std::string status_{"Waiting"};
    bool done_ {false};

    QLabel* time_label_{nullptr};

    QLabel* status_label_{nullptr};

    QPushButton* ok_button_{nullptr};
};

#endif // CALCULATEREFERENCESSTATUSDIALOG_H
