#ifndef JSONDATAMAPPINGWIDGET_H
#define JSONDATAMAPPINGWIDGET_H

#include <QWidget>

class JSONDataMapping;

class JSONDataMappingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JSONDataMappingWidget(JSONDataMapping& mapping, QWidget *parent = nullptr);

    void setMapping (JSONDataMapping& mapping);

private:
    JSONDataMapping* mapping_ {nullptr};
};

#endif // JSONDATAMAPPINGWIDGET_H
