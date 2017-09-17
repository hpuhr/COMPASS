#ifndef DBODATASOURCE_H
#define DBODATASOURCE_H

#include <QObject>

#include "configurable.h"

class DBObject;
class DBODataSourceDefinitionWidget;

/**
 * @brief Definition of a data source for a DBObject
 */
class DBODataSourceDefinition : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void definitionChangedSignal();

public:
    /// @brief Constructor, registers parameters
    DBODataSourceDefinition(const std::string &class_id, const std::string &instance_id, DBObject* object);
    virtual ~DBODataSourceDefinition();

    const std::string& schema () const { return schema_; }

    const std::string& localKey () const { return local_key_; }
    void localKey(const std::string &local_key);
    const std::string& metaTableName () const { return meta_table_; }
    void metaTable(const std::string &meta_table);
    const std::string& foreignKey () const { return foreign_key_; }
    void foreignKey(const std::string &foreign_key);
    const std::string& nameColumn () const { return name_column_; }
    void nameColumn(const std::string &name_column);

    DBODataSourceDefinitionWidget* widget ();

    bool hasLatitudeColumn () const { return latitude_column_.size() > 0; }
    std::string latitudeColumn() const;
    void latitudeColumn(const std::string &latitude_column);

    bool hasLongitudeColumn () const { return longitude_column_.size() > 0; }
    std::string longitudeColumn() const;
    void longitudeColumn(const std::string &longitude_column);

protected:
    DBObject* object_{nullptr};

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
    std::string latitude_column_;
    std::string longitude_column_;

    DBODataSourceDefinitionWidget* widget_{nullptr};
};

class DBODataSource
{
public:
    DBODataSource();
};

#endif // DBODATASOURCE_H
