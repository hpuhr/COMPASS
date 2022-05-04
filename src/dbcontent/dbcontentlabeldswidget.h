#ifndef DBCONTENTLABELDSWIDGET_H
#define DBCONTENTLABELDSWIDGET_H

#include <QWidget>

class DBContentLabelGenerator;

class QListWidget;
class QListWidgetItem;

class DBContentLabelDSWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void updateListSlot();
    void itemClickedSlot(QListWidgetItem* item);

public:
    DBContentLabelDSWidget(DBContentLabelGenerator& label_generator, QWidget* parent = 0,
                               Qt::WindowFlags f = 0);
    virtual ~DBContentLabelDSWidget();

protected:
    DBContentLabelGenerator& label_generator_;

    QListWidget* list_widget_{nullptr};
};

#endif // DBCONTENTLABELDSWIDGET_H
