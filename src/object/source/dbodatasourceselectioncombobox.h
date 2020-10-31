/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBODATASOURCESELECTIONCOMBOBOX_H
#define DBODATASOURCESELECTIONCOMBOBOX_H

#include <QComboBox>

#include "compass.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbodatasource.h"
#include "global.h"

/**
 *  @brief Property data type selection for a DBOVariable
 */
class DBODataSourceSelectionComboBox : public QComboBox
{
    Q_OBJECT

  signals:
    /// @brief Emitted if changed
    void changedDataSourceSignal();

  public:
    /// @brief Constructor
    DBODataSourceSelectionComboBox(DBObject& object, QWidget* parent = 0)
        : QComboBox(parent), db_object_(object)
    {
        if (db_object_.hasDataSources())
        {
            const std::map<int, DBODataSource>& data_sources = db_object_.dataSources();

            for (auto& ds_it : data_sources)
            {
                if (ds_it.second.hasShortName())
                    addItem(ds_it.second.shortName().c_str());
                else
                    addItem(ds_it.second.name().c_str());
            }
        }
        connect(this, SIGNAL(activated(const QString&)), this, SIGNAL(changedDataSourceSignal()));
    }

    /// @brief Destructor
    virtual ~DBODataSourceSelectionComboBox() {}

    /// @brief Returns the currently selected data type
    std::string getDSName() { return currentText().toStdString(); }

    bool hasDataSource(const std::string& ds_name)
    {
        int index = findText(QString(ds_name.c_str()));
        return index >= 0;
    }

    /// @brief Sets the currently selected data type
    void setDataSource(const std::string& ds_name)
    {
        int index = findText(QString(ds_name.c_str()));
        assert(index >= 0);
        setCurrentIndex(index);
    }

  protected:
    DBObject& db_object_;
};

#endif  // DBODATASOURCESELECTIONCOMBOBOX_H
