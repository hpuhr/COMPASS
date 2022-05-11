#ifndef DBCONTENTLABELDSWIDGET_H
#define DBCONTENTLABELDSWIDGET_H

#include "dbcontentlabelgenerator.h"

#include <QWidget>
#include <QIcon>

class DBContentLabelGenerator;

//class QListWidget;
class QPushButton;
class QGridLayout;

class DBContentLabelDSWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void updateListSlot();
    void sourceClickedSlot();
    void changeDirectionSlot();
    void selectDirectionSlot();

public:
    DBContentLabelDSWidget(DBContentLabelGenerator& label_generator, QWidget* parent = 0,
                               Qt::WindowFlags f = 0);
    virtual ~DBContentLabelDSWidget();

protected:
    DBContentLabelGenerator& label_generator_;

    //QListWidget* list_widget_{nullptr};

    QIcon arrow_lu_;
    QIcon arrow_ru_;
    QIcon arrow_ld_;
    QIcon arrow_rd_;

    QGridLayout* ds_grid_{nullptr};

    std::map<unsigned int, QPushButton*> direction_buttons_;

    QIcon& iconForDirection(LabelDirection direction);
};

#endif // DBCONTENTLABELDSWIDGET_H
