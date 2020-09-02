#ifndef CREATEASSOCIATIONSTASKWIDGET_H
#define CREATEASSOCIATIONSTASKWIDGET_H

#include <taskwidget.h>

class CreateAssociationsTask;

class CreateAssociationsTaskWidget : public TaskWidget
{
    Q_OBJECT

public slots:
    void expertModeChangedSlot();

public:
    CreateAssociationsTaskWidget(CreateAssociationsTask& task, QWidget* parent = 0,
                                 Qt::WindowFlags f = 0);

    virtual ~CreateAssociationsTaskWidget();

protected:
    CreateAssociationsTask& task_;
};

#endif // CREATEASSOCIATIONSTASKWIDGET_H
