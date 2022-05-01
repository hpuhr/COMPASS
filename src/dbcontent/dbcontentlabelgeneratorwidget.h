#ifndef DBCONTENTLABELGENERATORWIDGET_H
#define DBCONTENTLABELGENERATORWIDGET_H

#include <QWidget>

class DBContentLabelGenerator;

class DBContentLabelGeneratorWidget : public QWidget
{
    Q_OBJECT

public slots:
    void autoLabelChangedSlot(bool checked);
    void lodChangedSlot(const QString& text);

public:
    DBContentLabelGeneratorWidget(DBContentLabelGenerator& label_generator);
    virtual ~DBContentLabelGeneratorWidget();

protected:
    DBContentLabelGenerator& label_generator_;
};

#endif // DBCONTENTLABELGENERATORWIDGET_H
