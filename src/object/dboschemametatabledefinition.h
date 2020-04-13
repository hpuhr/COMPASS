#ifndef DBOSCHEMAMETATABLEDEFINITION_H
#define DBOSCHEMAMETATABLEDEFINITION_H

#include "configurable.h"

/**
 * @brief Definition of a meta table in a schema in a DBObject
 *
 * Simple storage class for a schema and a meta table, as strings. Used in a DBObject to save the
 * definitions into the configuration, and generate the pointers to the defined structures at from
 * them.
 */
class DBOSchemaMetaTableDefinition : public Configurable
{
  public:
    /// @brief Constructor, registers parameters
    DBOSchemaMetaTableDefinition(const std::string& class_id, const std::string& instance_id,
                                 Configurable* parent)
        : Configurable(class_id, instance_id, parent)
    {
        registerParameter("schema", &schema_, "");
        registerParameter("meta_table", &meta_table_, "");
    }
    /// @brief Destructor
    virtual ~DBOSchemaMetaTableDefinition() {}

    const std::string& schema() const { return schema_; }
    const std::string& metaTable() const { return meta_table_; }

  protected:
    /// DBSchema identifier
    std::string schema_;
    /// MetaDBTable identifier
    std::string meta_table_;
};

#endif  // DBOSCHEMAMETATABLEDEFINITION_H
