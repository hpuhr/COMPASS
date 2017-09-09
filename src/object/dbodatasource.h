#ifndef DBODATASOURCE_H
#define DBODATASOURCE_H


#include "configurable.h"

/**
 * @brief Definition of a data source for a DBObject
 */
class DBODataSourceDefinition : public Configurable
{
public:
    /// @brief Constructor, registers parameters
    DBODataSourceDefinition(const std::string &class_id, const std::string &instance_id, Configurable *parent);
    virtual ~DBODataSourceDefinition();

    const std::string& schema () const { return schema_; }
    const std::string& localKey () const { return local_key_; }
    const std::string& metaTableName () const { return meta_table_; }
    const std::string& foreignKey () const { return foreign_key_; }
    const std::string& nameColumn () const { return name_column_; }

protected:
    /// DBSchema identifier
    std::string schema_;
    /// Identifier for key in main table
    std::string local_key_;
    /// Identifier for meta table with data sources
    std::string meta_table_;
    /// Identifier for key in meta table with data sources
    std::string foreign_key_;
    /// Identifier for sensor name column in meta table with data sources
    std::string name_column_;
};

class DBODataSource
{
public:
    DBODataSource();
};

#endif // DBODATASOURCE_H
