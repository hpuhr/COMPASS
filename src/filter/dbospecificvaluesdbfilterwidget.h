#ifndef DBOSPECIFICVALUESDBFILTERWIDGET_H
#define DBOSPECIFICVALUESDBFILTERWIDGET_H

#include <QMenu>

#include "dbospecificvaluesdbfilter.h"
#include "dbfilterwidget.h"

class DBOSpecificValuesDBFilterWidget : public DBFilterWidget
{
    Q_OBJECT
  protected slots:
    void toggleDataSource();
    void selectSensorsAll();
    void selectSensorsNone();
    void setSourcesInactive();

public:
    DBOSpecificValuesDBFilterWidget(DBOSpecificValuesDBFilter& filter, const std::string& class_id,
                                    const std::string& instance_id);
    virtual ~DBOSpecificValuesDBFilterWidget();

    virtual void update();

protected:
    DBOSpecificValuesDBFilter& filter_;
    std::string dbo_name_;

    void updateCheckboxesChecked();
    void updateCheckboxesDisabled();

    void createMenu(bool inactive_disabled);
};

#endif // DBOSPECIFICVALUESDBFILTERWIDGET_H
