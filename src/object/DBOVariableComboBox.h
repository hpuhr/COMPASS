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
 * DBOVariableComboBox.h
 *
 *  Created on: Aug 29, 2012
 *      Author: sk
 */

#ifndef DBOVariableComboBox_H_
#define DBOVariableComboBox_H_

#include <QComboBox>
#include <stdexcept>

#include "Global.h"

class DBOVariable;

/**
 * @brief Allows selection of one DBOVariable from a given DBObject
 */
class DBOVariableComboBox : public QComboBox
{
    Q_OBJECT

public slots:
    /// @brief Is called when variable is changed
    void changed ();

public:
    /// @brief Constructor
    DBOVariableComboBox(const std::string &dbo_type, DBOVariable *variable, QWidget * parent = 0);
    /// @brief Destructor
    virtual ~DBOVariableComboBox();

private:
    /// Given DBObject type
    std::string dbo_type_;
    /// Selected DBOVariable
    DBOVariable *variable_;
};


#endif /* DBOVariableComboBox_H_ */
