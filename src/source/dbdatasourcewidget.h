#ifndef DBDATASOURCEWIDGET_H
#define DBDATASOURCEWIDGET_H

#include <QWidget>

#include <map>

class QCheckBox;
class QLabel;

class DataSourceManager;

namespace dbContent
{

class DBDataSource;

class DBDataSourceWidget : public QWidget
{
    Q_OBJECT

public slots:
    void loadingChangedSlot();

public:
    explicit DBDataSourceWidget(DBDataSource& src, QWidget *parent = nullptr);

    void update();

protected:
    DBDataSource& src_;
    DataSourceManager& ds_man_;

    QCheckBox* load_check_ {nullptr};

    std::map<std::string, QLabel*> content_labels_; // cont -> label
    std::map<std::string, QLabel*> loaded_cnt_labels_; // cont -> loaded count label
    std::map<std::string, QLabel*> total_cnt_labels_; // cont -> total count label

};

}
#endif // DBDATASOURCEWIDGET_H
