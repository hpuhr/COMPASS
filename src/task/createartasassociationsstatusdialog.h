#ifndef CREATEARTASASSOCIATIONSSTATUSDIALOG_H
#define CREATEARTASASSOCIATIONSSTATUSDIALOG_H

#include <QDialog>

#include "boost/date_time/posix_time/posix_time.hpp"

class QLabel;
class QPushButton;
class QGridLayout;
class CreateARTASAssociationsTask;

class CreateARTASAssociationsStatusDialog : public QDialog
{
    Q_OBJECT

signals:
    void closeSignal();

public slots:
    void okClickedSlot();

public:
    CreateARTASAssociationsStatusDialog(CreateARTASAssociationsTask& task, QWidget* parent=nullptr,
                                        Qt::WindowFlags f=0);

    void markStartTime ();
    void setDone ();

    void setDBODoneFlags(const std::map<std::string, bool> &dbo_done_flags);

    void setAssociationStatus(const std::string& status);

    void setMissingHashesAtBeginning(const size_t& missing_hashes_at_beginning);
    void setMissingHashes(const size_t& missing_hashes);
    void setFoundHashes(const size_t& found_hashes);
    void setDubiousAssociations(const size_t& dubious_associations);
    void setFoundDuplicates(const size_t& found_duplicates);

private:
    CreateARTASAssociationsTask& task_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;
    boost::posix_time::time_duration time_diff_;
    std::string elapsed_time_str_;

    std::map<std::string, bool> dbo_done_flags_;

    std::string association_status_ {"Waiting"};

    std::map<std::string, unsigned int> association_counts_;

    size_t missing_hashes_at_beginning_ {0};
    size_t missing_hashes_ {0};
    size_t found_hashes_ {0}; // dbo name -> cnt
    size_t dubious_associations_ {0};
    size_t found_duplicates_ {0};

    QLabel* time_label_ {nullptr};

    QGridLayout* dbo_done_grid_ {nullptr};

    QLabel* association_status_label_ {nullptr};
    QLabel* missing_hashes_at_beginning_label_ {nullptr};
    QLabel* missing_hashes_label_ {nullptr};
    QLabel* found_hashes_label_ {nullptr};
    QLabel* dubious_label_ {nullptr};
    QLabel* found_duplicates_label_ {nullptr};

    QGridLayout* dbo_associated_grid_ {nullptr};

    QPushButton* ok_button_ {nullptr};

    void updateTime ();
    void updateDBODoneGrid ();
    void updateDBOAssociatedGrid ();
};

#endif // CREATEARTASASSOCIATIONSSTATUSDIALOG_H
