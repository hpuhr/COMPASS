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
class DBOVariableDataTypeComboBox : public QComboBox
{
    Q_OBJECT

  signals:
    /// @brief Emitted if type was changed
    void changedType();

  public slots:
    /// @brief Sets the data type
    void changed() { variable_->dataType(getType()); }

  public:
    /// @brief Constructor
    DBOVariableDataTypeComboBox(DBOVariable& variable, QWidget* parent = 0)
        : QComboBox(parent), variable_(&variable)
    {
        const std::map<PropertyDataType, std::string>& datatypes2str =
            Property::dataTypes2Strings();
        for (auto it = datatypes2str.begin(); it != datatypes2str.end(); it++)
        {
            addItem(it->second.c_str());
        }

        update();

        connect(this, SIGNAL(activated(const QString&)), this, SIGNAL(changedType()));
        connect(this, SIGNAL(activated(const QString&)), this, SLOT(changed()));
    }
    /// @brief Destructor
    virtual ~DBOVariableDataTypeComboBox() {}

    /// @brief Returns the currently selected data type
    PropertyDataType getType() { return Property::asDataType(currentText().toStdString()); }

    /// @brief Sets the currently selected data type
    void setType(PropertyDataType type)
    {
        int index = findText(QString(Property::asString(type).c_str()));
        assert(index >= 0);
        setCurrentIndex(index);
    }

    void setVariable(DBOVariable& variable)
    {
        variable_ = &variable;

        update();
    }

  protected:
    /// Used variable
    DBOVariable* variable_{nullptr};

    void update()
    {
        int index = findText(QString(variable_->dataTypeString().c_str()));
        assert(index >= 0);
        setCurrentIndex(index);
    }
};

#endif /* DBOVARIABLEDATATYPECOMBOBOX_H_ */
