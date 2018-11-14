#ifndef DBOEDITDATASOURCEACTIONOPTIONSWIDGET_H
#define DBOEDITDATASOURCEACTIONOPTIONSWIDGET_H

#include "dboeditdatasourceactionoptions.h"


#include <QWidget>


class DBOEditDataSourceActionOptions;
class QCheckBox;
class QComboBox;

class DBOEditDataSourceActionOptionsWidget : public QWidget
{
    Q_OBJECT

public slots:
    void changedPerformSlot ();
    void changedActionSlot (int index);

public:
    DBOEditDataSourceActionOptionsWidget(DBOEditDataSourceActionOptions& options,
                                         QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~DBOEditDataSourceActionOptionsWidget() {}

    void update ();

private:
    DBOEditDataSourceActionOptions* options_;

    QCheckBox* perform_check_ {nullptr};
    QComboBox* action_box_ {nullptr};
};

#endif // DBOEDITDATASOURCEACTIONOPTIONSWIDGET_H
