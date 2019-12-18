#ifndef TASKWIDGET_H
#define TASKWIDGET_H

#include <QWidget>

class TaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    virtual void expertModeChangedSlot ()=0;

public:
    TaskWidget (QWidget* parent=0, Qt::WindowFlags f=0)
        : QWidget(parent, f)
    {
        setContentsMargins(0, 0, 0, 0);
    }
};

#endif // TASKWIDGET_H
