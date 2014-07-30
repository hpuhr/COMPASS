/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * DBOTypeComboBox.h
 *
 *  Created on: Aug 27, 2012
 *      Author: sk
 */

#ifndef DBOTYPECOMBOBOX_H_
#define DBOTYPECOMBOBOX_H_

#include <QComboBox>
#include "DBObjectManager.h"
#include <stdexcept>
#include <map>

#include "Global.h"

/**
 * @brief Selection widget for a DBOject type
 */
class DBOTypeComboBox : public QComboBox
{
    Q_OBJECT
signals:
    /// @brief Is emitted when type is changed
    void changedType();

public:
    /// @brief Constructor
    DBOTypeComboBox(bool show_non_existent = true, QWidget * parent = 0)
    {
        connect(this, SIGNAL( activated(const QString &) ), this, SIGNAL( changedType() ));

        int index=0;

        for (unsigned int cnt = 0; cnt <= DBO_SENSOR_INFORMATION; cnt++)
        {
            if (show_non_existent)
            {
                addItem (DB_OBJECT_TYPE_STRINGS.at((DB_OBJECT_TYPE) cnt).c_str());
                indexes_[(DB_OBJECT_TYPE)cnt] = index;
                index++;
            }
            else
            {
                if (DBObjectManager::getInstance().existsDBObject((DB_OBJECT_TYPE) cnt))
                {
                    addItem (DB_OBJECT_TYPE_STRINGS.at((DB_OBJECT_TYPE) cnt).c_str());
                    indexes_[(DB_OBJECT_TYPE)cnt] = index;
                    index++;
                }
            }
        }
    }
    /// @brief Desctructor
    virtual ~DBOTypeComboBox() {}

    /// @brief Returns current DBObject type
    DB_OBJECT_TYPE getType ()
    {
        std::string text = currentText().toStdString();
        for (unsigned int cnt = 0; cnt <= DBO_SENSOR_INFORMATION; cnt++)
        {
            if (text.compare (DB_OBJECT_TYPE_STRINGS.at((DB_OBJECT_TYPE) cnt)) == 0)
                return (DB_OBJECT_TYPE) cnt;
        }
        throw std::runtime_error ("DBOTypeComboBox: getType: unknown type");
    }

    /// @brief Sets current DBObject type
    void setType (DB_OBJECT_TYPE type)
    {
        assert (indexes_.find (type) != indexes_.end());

        setCurrentIndex (indexes_[type]);
    }

protected:
    std::map <DB_OBJECT_TYPE, int> indexes_; // type to index
};

#endif /* DBOBJECTEDITWIDGET_H_ */
