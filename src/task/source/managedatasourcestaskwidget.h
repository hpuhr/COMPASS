#ifndef MANAGEDATASOURCESTASKWIDGET_H
#define MANAGEDATASOURCESTASKWIDGET_H

#include <taskwidget.h>

class ManageDataSourcesTask;

class QTabWidget;

class ManageDataSourcesTaskWidget : public TaskWidget
{
    Q_OBJECT

public slots:
    void expertModeChangedSlot ();

    void exportConfigDataSourcesSlot ();
    void clearConfigDataSourcesSlot ();
    void importConfigDataSourcesSlot ();

    void dbItemChangedSlot ();

public:
    ManageDataSourcesTaskWidget(ManageDataSourcesTask& task, QWidget *parent=nullptr);

protected:
    ManageDataSourcesTask& task_;

    QTabWidget* tab_widget_ {nullptr};
};

#endif // MANAGEDATASOURCESTASKWIDGET_H
