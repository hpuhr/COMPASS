#ifndef DBDATASOURCEWIDGET_H
#define DBDATASOURCEWIDGET_H

#include <QWidget>

#include <map>

class QCheckBox;
class QLabel;
class QPushButton;
class QGridLayout;
class QVBoxLayout;

class DataSourceManager;

namespace dbContent
{

class DBDataSource;

class DBDataSourceWidget : public QWidget
{
    Q_OBJECT

public slots:
    void loadingChangedSlot();

    void lineButtonClickedSlot();

public:
    explicit DBDataSourceWidget(DBDataSource& src, QWidget *parent = nullptr);

    void updateContent();

    unsigned int getLabelMinWidth();
    void updateLabelMinWidth(unsigned int width);

protected:
    DBDataSource& src_;
    DataSourceManager& ds_man_;

    QVBoxLayout* main_layout_ {nullptr};
    QGridLayout* grid_layout_ {nullptr};

    QCheckBox* load_check_ {nullptr};

    std::map<std::string, QPushButton*> line_buttons_; // LX -> button, has all 4 lines, can hide
    std::map<std::string, QLabel*> content_labels_; // cont -> label
    std::map<std::string, QLabel*> loaded_cnt_labels_; // cont -> loaded count label
    std::map<std::string, QLabel*> total_cnt_labels_; // cont -> total count label

    //bool last_net_lines_shown_ {false};
    bool last_show_counts_ {false}; // indicates if counts where added to layout last time

    bool needsRecreate();
    void recreateWidgets();
    QWidget* createLinesWidget();
    void updateWidgets(); // updates all widgets to be stored in layout
};

}
#endif // DBDATASOURCEWIDGET_H
