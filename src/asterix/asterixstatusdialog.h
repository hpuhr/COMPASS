#ifndef ASTERIXSTATUSDIALOG_H
#define ASTERIXSTATUSDIALOG_H

#include <QDialog>

#include "boost/date_time/posix_time/posix_time.hpp"

class QLabel;
class QPushButton;
class QGridLayout;

class ASTERIXStatusDialog  : public QDialog
{
    Q_OBJECT

signals:
    void closeSignal();

public slots:
    void okClickedSlot();

public:
    explicit ASTERIXStatusDialog(const std::string& filename, bool test, QWidget* parent=nullptr, Qt::WindowFlags f=0);

    void markStartTime ();
    void setDone ();

    void addNumFrames (unsigned int cnt);
    void addNumRecords (unsigned int cnt);
    void addNumMapped (unsigned int cnt);
    void addNumNotMapped (unsigned int cnt);
    void addNumCreated (unsigned int cnt);
    void addNumInserted (const std::string& dbo_name, unsigned int cnt);

    void setCategoryCounts (const std::map<unsigned int, size_t>& counts);
    void addMappedCounts (const std::map<unsigned int, std::pair<size_t,size_t>>& counts);

private:
    std::string filename_;
    bool test_ {false};

    size_t num_frames_ {0};
    size_t num_records_ {0};
    size_t records_mapped_ {0};
    size_t records_not_mapped_ {0};
    size_t records_created_ {0};
    size_t records_inserted_ {0};

    std::map<unsigned int, size_t> category_read_counts_;
    std::map<unsigned int, std::pair<size_t,size_t>> category_mapped_counts_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;
    boost::posix_time::time_duration time_diff_;
    std::string elapsed_time_str_;

    QLabel* time_label_ {nullptr};

    QLabel* num_frames_label_ {nullptr};
    QLabel* num_records_label_ {nullptr};
    QLabel* num_records_rate_label_ {nullptr};
    QLabel* records_mapped_label_ {nullptr};
    QLabel* records_not_mapped_label_ {nullptr};
    QLabel* records_created_label_ {nullptr};
    QLabel* records_inserted_label_ {nullptr};
    QLabel* records_inserted_rate_label_ {nullptr};

    std::map<std::string, size_t> dbo_inserted_counts_;

    QGridLayout* cat_counters_grid_ {nullptr};
    QGridLayout* dbo_counters_grid_ {nullptr};
    QPushButton* ok_button_ {nullptr};

    void updateCategoryGrid ();
    void updateDBObjectGrid ();
    void updateTime ();
};

#endif // ASTERIXSTATUSDIALOG_H
