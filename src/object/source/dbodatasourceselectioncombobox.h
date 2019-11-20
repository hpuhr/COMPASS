#ifndef DBODATASOURCESELECTIONCOMBOBOX_H
#define DBODATASOURCESELECTIONCOMBOBOX_H

#include <QComboBox>

#include "atsdb.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "dbodatasource.h"
#include "global.h"

/**
 *  @brief Property data type selection for a DBOVariable
 */
class DBODataSourceSelectionComboBox: public QComboBox
{
    Q_OBJECT

signals:
    /// @brief Emitted if changed
    void changedDataSourceSignal();

public:
    /// @brief Constructor
    DBODataSourceSelectionComboBox(DBObject& object, QWidget* parent=0)
    : QComboBox(parent), db_object_ (object)
    {
        if (db_object_.hasDataSources())
        {
            const std::map<int, DBODataSource>& data_sources = db_object_.dataSources();

            for (auto& ds_it : data_sources)
            {
                if (ds_it.second.hasShortName())
                    addItem (ds_it.second.shortName().c_str());
                else
                    addItem (ds_it.second.name().c_str());
            }
        }
        connect(this, SIGNAL(activated(const QString &)), this, SIGNAL(changedDataSourceSignal()));
    }

    /// @brief Destructor
    virtual ~DBODataSourceSelectionComboBox() {}

    /// @brief Returns the currently selected data type
    std::string getDSName ()
    {
        return currentText().toStdString();
    }

    bool hasDataSource (const std::string &ds_name)
    {
        int index = findText(QString(ds_name.c_str()));
        return index >= 0;
    }

    /// @brief Sets the currently selected data type
    void setDataSource (const std::string &ds_name)
    {
        int index = findText(QString(ds_name.c_str()));
        assert (index >= 0);
        setCurrentIndex (index);
    }

protected:
    DBObject& db_object_;
};

#endif // DBODATASOURCESELECTIONCOMBOBOX_H
