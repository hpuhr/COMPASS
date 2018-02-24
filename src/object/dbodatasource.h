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

    bool hasShortNameColumn () const { return short_name_column_.size() > 0; }
    std::string shortNameColumn() const;
    void shortNameColumn(const std::string &short_name_column);

    bool hasSacColumn () const { return sac_column_.size() > 0; }
    std::string sacColumn() const;
    void sacColumn(const std::string &sac_column);

    bool hasSicColumn () const { return sic_column_.size() > 0; }
    std::string sicColumn() const;
    void sicColumn(const std::string &sic_column);

    bool hasAltitudeColumn () const { return altitude_column_.size() > 0; }
    std::string altitudeColumn() const;
    void altitudeColumn(const std::string &altitude_column);

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
    std::string short_name_column_;
    std::string name_column_;
    std::string sac_column_;
    std::string sic_column_;
    std::string latitude_column_;
    std::string longitude_column_;
    std::string altitude_column_;

    DBODataSourceDefinitionWidget* widget_{nullptr};
};

Q_DECLARE_METATYPE(DBODataSourceDefinition*)

class DBODataSource
{
public:
    DBODataSource(unsigned int id, const std::string& name);
    virtual ~DBODataSource();

    unsigned int id() const;

    const std::string &name() const;

    bool hasShortName() const;
    void shortName(const std::string &short_name);
    const std::string &shortName() const;

    bool hasSac() const;
    void sac(unsigned char sac);
    unsigned char sac() const;

    bool hasSic() const;
    void sic(unsigned char sic);
    unsigned char sic() const;

    bool hasLatitude() const;
    void latitude(double latitiude);
    double latitude() const;

    bool hasLongitude() const;
    void longitude(double longitude_);
    double longitude() const;

    bool hasAltitude() const;
    void altitude(double altitude);
    double altitude() const;

    void finalize ();

    bool isFinalized () { return finalized_; } // returns false if projection can not be made because of error

    // azimuth degrees, range & altitude in meters
    void calculateSystemCoordinates (double azimuth, double slant_range, double altitude, bool has_altitude,
                                     double &sys_x, double &sys_y);

protected:
    unsigned int id_{0};

    std::string name_;

    bool has_short_name_{false};
    std::string short_name_;

    bool has_sac_;
    unsigned char sac_;

    bool has_sic_{false};
    unsigned char sic_{0};

    bool has_latitude_{false};
    double latitude_{0}; //degrees

    bool has_longitude_{false};
    double longitude_{0}; // degrees

    bool has_altitude_{false};
    double altitude_{0};  // meter above msl

    bool finalized_ {false};

    double system_x_;
    double system_y_;

//    double local_trans_x_;
//    double local_trans_y_;
};

#endif // DBODATASOURCE_H
