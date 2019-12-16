#ifndef POSTPROCESSTASKWIDGET_H
#define POSTPROCESSTASKWIDGET_H


#include <QWidget>

class PostProcessTask;

class PostProcessTaskWidget : public QWidget
{
    Q_OBJECT
//public slots:
//    void runSlot ();

public:
    PostProcessTaskWidget(PostProcessTask& task, QWidget *parent=nullptr);

protected:
    PostProcessTask& task_;

    //QWidget* dbschema_manager_widget_ {nullptr};
};

#endif // POSTPROCESSTASKWIDGET_H
