#ifndef CREATEARTASASSOCIATIONSTASKWIDGET_H
#define CREATEARTASASSOCIATIONSTASKWIDGET_H


#include <QWidget>

class CreateARTASAssociationsTask;
class QPushButton;

class CreateARTASAssociationsTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void runSlot ();
    void runDoneSlot ();

public:

    CreateARTASAssociationsTaskWidget(CreateARTASAssociationsTask& task, QWidget* parent=0,
                                      Qt::WindowFlags f=0);

    virtual ~CreateARTASAssociationsTaskWidget();

    void update ();

protected:
    CreateARTASAssociationsTask& task_;

    QPushButton* calc_button_ {nullptr};
};

#endif // CREATEARTASASSOCIATIONSTASKWIDGET_H
