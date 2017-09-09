#include "dbodatasource.h"

DBODataSourceDefinition::DBODataSourceDefinition(const std::string &class_id, const std::string &instance_id, Configurable *parent)
    : Configurable (class_id, instance_id, parent)
{
    registerParameter ("schema", &schema_, "");
    registerParameter ("local_key", &local_key_, "");
    registerParameter ("meta_table", &meta_table_,  "");
    registerParameter ("foreign_key", &foreign_key_, "");
    registerParameter ("name_column", &name_column_, "");
}

DBODataSourceDefinition::~DBODataSourceDefinition()
{

}

DBODataSource::DBODataSource()
{

}
