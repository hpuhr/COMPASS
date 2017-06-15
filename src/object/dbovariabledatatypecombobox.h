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
 * DBOVariableDataTypeComboBox.h
 *
 *  Created on: Aug 29, 2012
 *      Author: sk
 */

#ifndef DBOVARIABLEDATATYPECOMBOBOX_H_
#define DBOVARIABLEDATATYPECOMBOBOX_H_

#include <QComboBox>
#include <stdexcept>

#include "dbovariable.h"
#include "global.h"
#include "logger.h"

/**
 *  @brief Property data type selection for a DBOVariable
 */
class DBOVariableDataTypeComboBox: public QComboBox
{
    Q_OBJECT

signals:
    /// @brief Emitted if type was changed
    void changedType();

public slots:
    /// @brief Sets the data type
    void changed ()
    {
        variable_->dataType(getType());
    }

public:
    /// @brief Constructor
    DBOVariableDataTypeComboBox(DBOVariable *variable, QWidget * parent = 0)
    : QComboBox(parent), variable_(variable)
    {
        const std::map<PropertyDataType,std::string> &datatypes2str = Property::dataTypes2Strings();
        for (auto it = datatypes2str.begin(); it != datatypes2str.end(); it++)
        {
            addItem (it->second.c_str());
        }

        int index = findText(QString(variable_->dataTypeString().c_str()));
        assert (index >= 0);
        setCurrentIndex (index);
        connect(this, SIGNAL( activated(const QString &) ), this, SIGNAL( changedType() ));
        connect(this, SIGNAL( activated(const QString &) ), this, SLOT( changed() ));

    }
    /// @brief Destructor
    virtual ~DBOVariableDataTypeComboBox() {}
    /// @brief Returns the currently selected data type
    PropertyDataType getType ()
    {
        return Property::asDataType(currentText().toStdString());
//        return ;
//        for (unsigned int cnt = 0; cnt < P_TYPE_SENTINEL; cnt++)
//        {
//            if (text.compare (PROPERTY_DATA_TYPE_STRINGS.at((PROPERTY_DATA_TYPE) cnt)) == 0)
//                return (PROPERTY_DATA_TYPE) cnt;
//        }
//        throw std::runtime_error ("DBOVariableDataTypeComboBox: getType: unknown type");
    }
    /// @brief Sets the currently selected data type
    void setType (PropertyDataType type)
    {
        logdbg << "UGA2 '" << Property::asString(type) << "'";
        int index = findText(QString(Property::asString(type).c_str()));
        assert (index >= 0);
        setCurrentIndex (index);
    }

protected:
    /// Used variable
    DBOVariable *variable_;
};

#endif /* DBOVARIABLEDATATYPECOMBOBOX_H_ */
