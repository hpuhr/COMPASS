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

#include <memory>
//#include <QWidget>

//#include "geomap.h"
#include "dboeditdatasourceactionoptions.h"

class DBObject;
// class DBODataSourceWidget;
// class QGridLayout;

class DBODataSource
{
  public:
    DBODataSource(DBObject& object, unsigned int id, const std::string& name);
    DBODataSource() = default;

    // copy from dbds, everything but id
    DBODataSource& operator=(StoredDBODataSource& other);
    // DBODataSource& operator=(DBODataSource&& other);

    // comparison
    bool operator==(const StoredDBODataSource& other) const;
    bool operator!=(const StoredDBODataSource& other) const { return !(*this == other); }

    virtual ~DBODataSource();

    const std::string dboName() const;

    unsigned int id() const;
    // void id(unsigned int id);

    const std::string& name() const;
    void name(const std::string& name);

    bool hasShortName() const;
    void removeShortName();
    void shortName(const std::string& short_name);
    const std::string& shortName() const;

    bool hasSac() const;
    void removeSac();
    void sac(unsigned char sac);
    unsigned char sac() const;

    bool hasSic() const;
    void removeSic();
    void sic(unsigned char sic);
    unsigned char sic() const;

    bool hasLatitude() const;
    void removeLatitude();
    void latitude(double latitiude);
    double latitude() const;

    bool hasLongitude() const;
    void removeLongitude();
    void longitude(double longitude_);
    double longitude() const;

    bool hasAltitude() const;
    void removeAltitude();
    void altitude(double altitude);
    double altitude() const;

    DBObject& object();
    void updateInDatabase();  // not called automatically in setters

  protected:
    DBObject* object_;
    unsigned int id_{0};

    std::string name_;

    bool has_short_name_{false};
    std::string short_name_;

    bool has_sac_{false};
    unsigned char sac_{0};

    bool has_sic_{false};
    unsigned char sic_{0};

    bool has_latitude_{false};
    double latitude_{0};  // degrees

    bool has_longitude_{false};
    double longitude_{0};  // degrees

    bool has_altitude_{false};
    double altitude_{0};  // meter above msl

    //    t_CPos grs_pos_;
    //    t_GPos geo_pos_;
    //    t_Mapping_Info mapping_info_;
};

#endif  // DBODATASOURCE_H
