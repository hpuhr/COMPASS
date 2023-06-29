#ifndef CALCULATEREFERENCESSTATUSDIALOG_H
#define CALCULATEREFERENCESSTATUSDIALOG_H

#include "buffer.h"

#include <QDialog>



//#include "boost/date_time/posix_time/posix_time.hpp"

class QLabel;
class QPushButton;
class QGridLayout;
class CalculateReferencesTask;

class CalculateReferencesStatusDialog : public QDialog
{
    Q_OBJECT

signals:
    void closeSignal();

public:
    typedef std::map<std::string, std::pair<unsigned int, unsigned int>> PositionCountsMap; // dbcont -> used, unused

    typedef std::vector<std::pair<std::string, std::string>> CalcInfoVector; // text pairs


public slots:
    void okClickedSlot();

    void setStatusSlot(const std::string& status);
    void setLoadedCountsSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data);
    void setUsedPositionCountsSlot (CalculateReferencesStatusDialog::PositionCountsMap counts);
    void setCalculateInfoSlot(CalculateReferencesStatusDialog::CalcInfoVector info);

public:
    CalculateReferencesStatusDialog(CalculateReferencesTask& task,
                                    QWidget* parent=nullptr, Qt::WindowFlags f=0);

    void markStartTime();
    void updateTime();
    void setDone();

private:
    CalculateReferencesTask& task_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;
    boost::posix_time::time_duration time_diff_;
    std::string elapsed_time_str_;

    std::string status_{"Waiting"};
    bool done_ {false};

    QLabel* time_label_{nullptr};

    QGridLayout* dbcont_loaded_grid_ {nullptr};

    QGridLayout* pos_used_grid_ {nullptr};

    QGridLayout* calc_info_grid_ {nullptr};

    QLabel* status_label_{nullptr};

    QPushButton* ok_button_{nullptr};
};

Q_DECLARE_METATYPE(CalculateReferencesStatusDialog::PositionCountsMap);
Q_DECLARE_METATYPE(CalculateReferencesStatusDialog::CalcInfoVector);

#endif // CALCULATEREFERENCESSTATUSDIALOG_H
