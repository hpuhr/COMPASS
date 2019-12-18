#ifndef MYSQLDBIMPORTTASKWIDGET_H
#define MYSQLDBIMPORTTASKWIDGET_H

#include <taskwidget.h>

class MySQLDBImportTask;

class QPushButton;
class QListWidget;

class MySQLDBImportTaskWidget : public TaskWidget
{
    Q_OBJECT

public slots:
    void addFileSlot ();
    void deleteFileSlot ();
    void selectedFileSlot ();
    void updateFileListSlot ();

    void expertModeChangedSlot ();

public:
    MySQLDBImportTaskWidget(MySQLDBImportTask& task, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~MySQLDBImportTaskWidget();

protected:
    MySQLDBImportTask& task_;

    QListWidget* file_list_ {nullptr};
    QPushButton* add_file_button_ {nullptr};
    QPushButton* delete_file_button_ {nullptr};
};

#endif // MYSQLDBIMPORTTASKWIDGET_H
