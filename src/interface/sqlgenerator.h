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

#ifndef SQLGENERATOR_H_
#define SQLGENERATOR_H_

#include <memory>

#include "dbcontent/variable/variableset.h"

class Buffer;
class DBCommand;
class DBCommandList;
class DBInterface;
class DBContent;

class SQLGenerator
{
public:
    SQLGenerator(DBInterface& db_interface);
    virtual ~SQLGenerator();

    std::string getCreateTableStatement(const DBContent& object);
    std::string insertDBUpdateStringBind(std::shared_ptr<Buffer> buffer, std::string table_name);
    std::string createDBUpdateStringBind(std::shared_ptr<Buffer> buffer,
                                         const std::string& key_col_name, std::string table_name);

    std::shared_ptr<DBCommand> getSelectCommand(
            const DBContent& object, dbContent::VariableSet read_list,
            std::vector<std::string> extra_from_parts, const std::string& filter,
            bool use_order = false, dbContent::Variable* order_variable = nullptr,
            bool use_order_ascending = false, const std::string& limit = "");

    //    std::shared_ptr<DBCommand> getSelectCommand(const DBContent& object,
    //                                                const std::vector<std::string>& columns,
    //                                                bool distinct = false);
    std::shared_ptr<DBCommand> getDataSourcesSelectCommand();

    //std::shared_ptr<DBCommand> getDistinctDataSourcesSelectCommand(DBContent& object);
    std::shared_ptr<DBCommand> getMaxUIntValueCommand(const std::string& table_name,
                                                      const std::string& col_name);
    std::shared_ptr<DBCommand> getADSBInfoCommand(DBContent& adsb_obj);

//    std::string getCreateAssociationTableStatement(const std::string& table_name);
//    std::shared_ptr<DBCommand> getSelectAssociationsCommand(const std::string& table_name);

    std::string getCountStatement(const std::string& table);

    //std::string getTableMinMaxCreateStatement();
    std::string getTablePropertiesCreateStatement();
    std::string getTableDataSourcesCreateStatement();
    std::string getTableSectorsCreateStatement();
    std::string getTableViewPointsCreateStatement();
    std::string getTableTargetsCreateStatement();
    std::string getDeleteStatement (const std::string& table, const std::string& filter);

    std::string getInsertTargetStatement(unsigned int utn, const std::string& info);

    std::string getInsertPropertyStatement(const std::string& id, const std::string& value);
    std::string getSelectPropertyStatement(const std::string& id);
    std::string getSelectAllPropertiesStatement();

    //    std::string getInsertMinMaxStatement(const std::string& variable_name,
    //                                         const std::string& object_name, const std::string& min,
    //                                         const std::string& max);
    //    std::string getSelectMinMaxStatement(const std::string& variable_name,
    //                                         const std::string& object_name);
    //    std::string getSelectMinMaxStatement();

    std::string getSelectNullCount (const std::string& table_name, const std::vector<std::string> columns);

    std::string getInsertViewPointStatement(const unsigned int id, const std::string& json);
    std::string getSelectAllViewPointsStatement();

    std::string getReplaceSectorStatement(const unsigned int id, const std::string& name,
                                          const std::string& layer_name, const std::string& json);
    std::string getSelectAllSectorsStatement();
    std::string getSelectAllTargetsStatement();

    std::shared_ptr<DBCommand> getTableSelectMinMaxNormalStatement(const DBContent& object);

protected:
    DBInterface& db_interface_;

    //std::string table_minmax_create_statement_;
    std::string table_properties_create_statement_;
    std::string table_data_sources_create_statement_;
    std::string table_sectors_create_statement_;
    std::string table_view_points_create_statement_;
    std::string table_targets_create_statement_;

    //    std::string subTablesWhereClause(const DBTable& table,
    //                                     const std::vector<std::string>& used_tables);
    //    std::string subTableKeyClause(const DBTable& table, const std::string& sub_table_name);
};

#endif /* SQLGENERATOR_H_ */
