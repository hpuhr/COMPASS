#ifndef POSTPROCESSTASKWIDGET_H
#define POSTPROCESSTASKWIDGET_H

#include <taskwidget.h>

class PostProcessTask;

class PostProcessTaskWidget : public TaskWidget
{
    Q_OBJECT

public slots:
    void expertModeChangedSlot ();

public:
    PostProcessTaskWidget(PostProcessTask& task, QWidget *parent=nullptr);

protected:
    PostProcessTask& task_;

    //QWidget* dbschema_manager_widget_ {nullptr};
};

#endif // POSTPROCESSTASKWIDGET_H
