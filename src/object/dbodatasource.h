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
    virtual ~DBODataSource();

    void id(unsigned int id);
    unsigned int id() const;

    void name(const std::string &name);
    const std::string &name() const;

    void shortName(const std::string &short_name);
    const std::string &shortName() const;

    void sac(unsigned char sac);
    unsigned char sac() const;

    void sic(unsigned char sic);
    unsigned char sic() const;

    void altitude(double altitude);
    double altitude() const;

    void latitude(double latitiude);
    double latitude() const;

    void longitude(double longitude_);
    double longitude() const;

    //void finalize ();

    // azimuth degrees, range & altitude in meters
    //void calculateSystemCoordinates (double azimuth, double slant_range, double altitude, bool has_altitude, double &sys_x, double &sys_y);

protected:
    unsigned int id_;

    std::string name_;
    std::string short_name_;

    unsigned char sac_;
    unsigned char sic_;

    double latitude_; //degrees
    double longitude_; // degrees
    double altitude_;  // meter above msl

    //bool finalized_;


//    double system_x_;
//    double system_y_;

//    double local_trans_x_;
//    double local_trans_y_;

//    double deg2rad_;
};

#endif // DBODATASOURCE_H
