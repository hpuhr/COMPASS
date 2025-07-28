#pragma once

#include "compass.h"
#include "datasourcemanager.h"
#include "source/dbdatasource.h"

#include <QComboBox>

namespace dbContent
{

class DBDataSourceComboBox : public QComboBox
{
    Q_OBJECT

  signals:
    void changedSource();

  public:
    DBDataSourceComboBox(QWidget* parent = nullptr)
        : QComboBox(parent)
    {
        updateBox();

        connect(this, SIGNAL(activated(const QString&)), this, SIGNAL(changedSource()));
    }
    virtual ~DBDataSourceComboBox() {}

    void showDSTypeOnly(const std::string& only_dstype_name)
    {
        only_dstype_name_ = only_dstype_name;
        show_dstype_only_ = true;

        updateBox();
    }

    void disableShowDSTypeOnly()
    {
        only_dstype_name_ = "";
        show_dstype_only_ = false;
    }

    void showDBContentOnly(const std::string& only_dbcontent_name)
    {
        only_dbcontent_name_ = only_dbcontent_name;
        show_dbcontent_only_ = true;

        updateBox();
    }

    void disableShowDBOOnly()
    {
        only_dbcontent_name_ = "";
        show_dbcontent_only_ = false;
    }

    std::string getDSName() { return currentText().toStdString(); }

    bool hasDSName(const std::string& ds_name)
    {
        int index = findText(QString(ds_name.c_str()));
        return index >= 0;
    }

    void setDSName(const std::string& ds_name)
    {
        int index = findText(QString(ds_name.c_str()));
        assert(index >= 0);
        setCurrentIndex(index);
    }

  protected:
    bool show_dstype_only_{false};
    std::string only_dstype_name_;

    bool show_dbcontent_only_{false};
    std::string only_dbcontent_name_;

    void updateBox()
    {
        clear();

        for (auto& ds_it : COMPASS::instance().dataSourceManager().dbDataSources())
        {
            if (show_dstype_only_ && ds_it->dsType() != only_dstype_name_)
                continue;

            if (show_dbcontent_only_ && !ds_it->hasNumInserted(only_dbcontent_name_))
                continue;

            addItem(ds_it->name().c_str());
        }

        setCurrentIndex(0);
    }
};

}
