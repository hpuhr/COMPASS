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

#ifndef DBTABLECOLUMN_H_
#define DBTABLECOLUMN_H_

#include <string>
#include "configurable.h"
#include "property.h"

class DBInterface;
class DBTable;
class UnitSelectionWidget;

/**
 * @brief Database table column definition
 *
 * Holds some parameters which define a database table column
 */
class DBTableColumn : public Configurable
{
public:
    /// @brief Constructor
    DBTableColumn(const std::string &class_id, const std::string &instance_id, DBTable *table,
                  DBInterface& db_interface);
    /// @brief Destructor
    virtual ~DBTableColumn();

    /// @brief Sets the column name
    void name (const std::string &name);
    /// @brief Returns the column name
    const std::string &name() const { return name_; }

    const std::string &identifier () const { return identifier_; }

    /// @brief Sets the data type
    void type (const std::string &type) { type_=type; }
    /// @brief Returns the data type
    const std::string &type() const { return type_; }
    PropertyDataType propertyType () const;

    /// @brief Sets key flag
    void isKey (bool is_key) { is_key_=is_key;}
    /// @brief Returns key flag
    bool isKey () const { return is_key_; }

    void comment (const std::string &comment) { comment_ = comment; }
    const std::string &comment () const { return comment_; }

    bool hasDimension () const { return dimension_.size() > 0; }
    /// @brief Returns dimension contained in the column
    const std::string &dimension () const { return dimension_; }
    /// @brief Returns unit
    const std::string &unit () const { return unit_; }

    //  bool hasSpecialNull () const { return special_null_.size() > 0; }
    //  void specialNull (std::string special_null) { special_null_ = special_null; }
    //  const std::string &specialNull () const { return special_null_; }

    DBTable &table() const { return table_; }

    UnitSelectionWidget *unitWidget ();

    void createSubConfigurables () {}

    void updateOnDatabase(); // check what informations is present in the current db
    bool existsInDB () const { return exists_in_db_; }

    std::string numberConversionType() const;
    void numberConversionType(const std::string& number_conversion_type);

protected:
    DBTable &table_;
    DBInterface& db_interface_;

    /// Name of the column
    std::string name_;
    std::string identifier_;
    /// Data type
    std::string type_;
    /// Key flag
    bool is_key_;
    /// Data type
    std::string comment_;
    /// Unit dimension
    std::string dimension_;
    /// Unit
    std::string unit_;
    /// Special value signifying null value
    std::string special_null_;
    /// Special number conversion type
    std::string number_conversion_type_;

    UnitSelectionWidget* widget_ {nullptr};

    static std::map<std::string, PropertyDataType> db_types_2_data_types_;

    bool exists_in_db_ {false};
};

#endif /* DBTABLECOLUMN_H_ */
