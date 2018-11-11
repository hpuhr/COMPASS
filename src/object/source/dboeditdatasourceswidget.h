#ifndef DBOEDITDATASOURCESWIDGET_H
#define DBOEDITDATASOURCESWIDGET_H

#include <QWidget>

class DBObject;

class DBOEditDataSourcesWidget : public QWidget
{
public:
    DBOEditDataSourcesWidget(DBObject* object, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~DBOEditDataSourcesWidget() {}

    void update ();

private:
    /// @brief DBObject to be managed
    DBObject* object_ {nullptr};
};

#endif // DBOEDITDATASOURCESWIDGET_H
