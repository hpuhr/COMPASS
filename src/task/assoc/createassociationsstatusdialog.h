#ifndef CREATEASSOCIATIONSSTATUSDIALOG_H
#define CREATEASSOCIATIONSSTATUSDIALOG_H

#include <QDialog>

#include "boost/date_time/posix_time/posix_time.hpp"

class QLabel;
class QPushButton;
class QGridLayout;
class CreateAssociationsTask;

class CreateAssociationsStatusDialog : public QDialog
{
    Q_OBJECT

signals:
    void closeSignal();

public slots:
    void okClickedSlot();

public:
    CreateAssociationsStatusDialog(CreateAssociationsTask& task,
                                   QWidget* parent=nullptr, Qt::WindowFlags f=0);

    void markStartTime();
    void setDone();

    void setDBODoneFlags(const std::map<std::string, bool>& dbo_done_flags);

    void setAssociationStatus(const std::string& status);

private:
    CreateAssociationsTask& task_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;
    boost::posix_time::time_duration time_diff_;
    std::string elapsed_time_str_;

    std::map<std::string, bool> dbo_done_flags_;

    std::string association_status_{"Waiting"};

    std::map<std::string, unsigned int> association_counts_;

    QLabel* time_label_{nullptr};

    QGridLayout* dbo_done_grid_{nullptr};

    QLabel* association_status_label_{nullptr};

    QGridLayout* dbo_associated_grid_{nullptr};

    QPushButton* ok_button_{nullptr};

    void updateTime();
    void updateDBODoneGrid();
    void updateDBOAssociatedGrid();
};

#endif // CREATEASSOCIATIONSSTATUSDIALOG_H
