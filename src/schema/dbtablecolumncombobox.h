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

#ifndef DBTABLECOLUMNCOMBOBOX_H_
#define DBTABLECOLUMNCOMBOBOX_H_

#include <QComboBox>
#include <stdexcept>

#include "global.h"

class DBOVariable;

/**
 * @brief Selection of a DBTableColumn
 */
class DBTableColumnComboBox : public QComboBox
{
    Q_OBJECT

  public:
    /// @brief Constructor
    DBTableColumnComboBox(const std::string& schema, const std::string& meta_table,
                          DBOVariable& variable, QWidget* parent = 0);
    /// @brief Destructor
    virtual ~DBTableColumnComboBox();

  private:
    /// Schema name
    std::string schema_;
    /// Meta table name
    std::string meta_table_;
    /// Variable
    DBOVariable& variable_;
};

#endif /* DBTABLECOLUMNCOMBOBOX_H_ */
