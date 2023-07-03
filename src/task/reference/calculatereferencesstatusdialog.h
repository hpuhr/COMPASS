#ifndef CALCULATEREFERENCESSTATUSDIALOG_H
#define CALCULATEREFERENCESSTATUSDIALOG_H

#include "buffer.h"

#include <QDialog>
//#include <QMap>
//#include <QPair>
//#include <QList>

//#include "boost/date_time/posix_time/posix_time.hpp"

class QLabel;
class QPushButton;
class QGridLayout;
class CalculateReferencesTask;

struct PositionCountsMapStruct
{
    std::map<std::string, std::pair<unsigned int, unsigned int>> pos_map;
};

struct CalcInfoVectorStruct
{
    std::vector<std::pair<std::string, std::string>> info_vec;
};


class CalculateReferencesStatusDialog : public QDialog
{
    Q_OBJECT

signals:
    void closeSignal();

public:
    //typedef QMap<QString, QPair<unsigned int, unsigned int>> PositionCountsMap; // dbcont -> used, unused

    //typedef QList<QPair<QString, QString>> CalcInfoVector; // text pairs

public slots:
    void okClickedSlot();

    void setStatusSlot(const QString& status);
    void setLoadedCountsSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data);
    void setUsedPositionCountsSlot (PositionCountsMapStruct counts);
    void setCalculateInfoSlot(CalcInfoVectorStruct info);

public:
    CalculateReferencesStatusDialog(CalculateReferencesTask& task,
                                    QWidget* parent=nullptr, Qt::WindowFlags f=0);

    void markStartTime();
    void updateTime();
    void setDone(const QString& result = "");

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
    QLabel* result_label_{nullptr};

    QPushButton* ok_button_{nullptr};
};

Q_DECLARE_METATYPE(PositionCountsMapStruct);
Q_DECLARE_METATYPE(CalcInfoVectorStruct);
//Q_DECLARE_METATYPE(std::string);


#endif // CALCULATEREFERENCESSTATUSDIALOG_H
