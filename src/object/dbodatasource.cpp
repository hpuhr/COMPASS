#include "dbodatasource.h"
#include "dbobject.h"
#include "dbodatasourcedefinitionwidget.h"

DBODataSourceDefinition::DBODataSourceDefinition(const std::string &class_id, const std::string &instance_id, DBObject* object)
    : Configurable (class_id, instance_id, object), object_(object)
{
    registerParameter ("schema", &schema_, "");
    registerParameter ("local_key", &local_key_, "");
    registerParameter ("meta_table", &meta_table_,  "");
    registerParameter ("foreign_key", &foreign_key_, "");
    registerParameter ("name_column", &name_column_, "");
}

DBODataSourceDefinition::~DBODataSourceDefinition()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

DBODataSourceDefinitionWidget* DBODataSourceDefinition::widget ()
{
    if (!widget_)
    {
        widget_ = new DBODataSourceDefinitionWidget (*object_, *this);
    }

    assert (widget_);
    return widget_;
}

void DBODataSourceDefinition::localKey(const std::string &local_key)
{
    loginf << "DBODataSourceDefinition: localKey: value " << local_key;
    local_key_ = local_key;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::metaTable(const std::string &meta_table)
{
    loginf << "DBODataSourceDefinition: metaTable: value " << meta_table;
    meta_table_ = meta_table;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::foreignKey(const std::string &foreign_key)
{
    loginf << "DBODataSourceDefinition: foreignKey: value " << foreign_key;
    foreign_key_ = foreign_key;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::nameColumn(const std::string &name_column)
{
    loginf << "DBODataSourceDefinition: nameColumn: value " << name_column;
    name_column_ = name_column;
    emit definitionChangedSignal();
}

DBODataSource::DBODataSource()
{
    
}
