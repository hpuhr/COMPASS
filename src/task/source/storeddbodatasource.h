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

#ifndef STOREDDBODATASOURCE_H
#define STOREDDBODATASOURCE_H

#include <QWidget>
#include <memory>
#include <string>

#include "configurable.h"

class ManageDataSourcesTask;
class DBODataSource;
// class StoredDBODataSourceWidget;

class StoredDBODataSource : public Configurable
{
  public:
    StoredDBODataSource(const std::string& class_id, const std::string& instance_id,
                        ManageDataSourcesTask& task);
    StoredDBODataSource() = default;
    virtual ~StoredDBODataSource();

    // copy from dbds, everything but id
    StoredDBODataSource& operator=(DBODataSource& other);
    /// @brief Move constructor
    StoredDBODataSource& operator=(StoredDBODataSource&& other);

    // comparison
    bool operator==(const DBODataSource& other) const;
    bool operator!=(const DBODataSource& other) const { return !(*this == other); }

    unsigned int id() const;

    void name(const std::string& name);
    const std::string& name() const;

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

    // psr
    bool hasPrimaryAzimuthStdDev() const;
    void removePrimaryAzimuthStdDev();
    void primaryAzimuthStdDev(double value);
    double primaryAzimuthStdDev() const;

    bool hasPrimaryRangeStdDev() const;
    void removePrimaryRangeStdDev();
    void primaryRangeStdDev(double value);
    double primaryRangeStdDev() const;

    bool hasPrimaryRangeMin() const;
    void removePrimaryRangeMin();
    void primaryRangeMin(int value);
    int primaryRangeMin() const;

    bool hasPrimaryRangeMax() const;
    void removePrimaryRangeMax();
    void primaryRangeMax(int value);
    int primaryRangeMax() const;

    //ssr
    bool hasSecondaryAzimuthStdDev() const;
    void removeSecondaryAzimuthStdDev();
    void secondaryAzimuthStdDev(double value);
    double secondaryAzimuthStdDev() const;

    bool hasSecondaryRangeStdDev() const;
    void removeSecondaryRangeStdDev();
    void secondaryRangeStdDev(double value);
    double secondaryRangeStdDev() const;

    bool hasSecondaryRangeMin() const;
    void removeSecondaryRangeMin();
    void secondaryRangeMin(int value);
    int secondaryRangeMin() const;

    bool hasSecondaryRangeMax() const;
    void removeSecondaryRangeMax();
    void secondaryRangeMax(int value);
    int secondaryRangeMax() const;

    // mode s
    bool hasModeSAzimuthStdDev() const;
    void removeModeSAzimuthStdDev();
    void modeSAzimuthStdDev(double value);
    double modeSAzimuthStdDev() const;

    bool hasModeSRangeStdDev() const;
    void removeModeSRangeStdDev();
    void modeSRangeStdDev(double value);
    double modeSRangeStdDev() const;

    bool hasModeSRangeMin() const;
    void removeModeSRangeMin();
    void modeSRangeMin(int value);
    int modeSRangeMin() const;

    bool hasModeSRangeMax() const;
    void removeModeSRangeMax();
    void modeSRangeMax(int value);
    int modeSRangeMax() const;

    std::string dboName() const;

    nlohmann::json getAsJSON();
    void setFromJSON(nlohmann::json& j);

  private:
    std::string dbo_name_;
    unsigned int id_{0};
    std::string name_;
    bool has_short_name_{false};
    std::string short_name_;
    bool has_sac_{false};
    unsigned int sac_{0};
    bool has_sic_{false};
    unsigned int sic_{0};
    bool has_latitude_{false};
    double latitude_{0};  // degrees
    bool has_longitude_{false};
    double longitude_{0};  // degrees
    bool has_altitude_{false};
    double altitude_{0};  // meter above msl

    // radar specific stuff

    // pri azm bias
    // pri range bias
    // pri range gain

    // pri azm standard dev
    bool has_primary_azimuth_stddev_{false};
    double primary_azimuth_stddev_ {0};  // degrees
    // pri range standard dev
    bool has_primary_range_stddev_{false};
    double primary_range_stddev_ {0};  // meters

    bool has_primary_ir_min_ {false};
    int primary_ir_min_ {0};  // nm

    bool has_primary_ir_max_ {false};
    int primary_ir_max_ {0};  // nm

    // ssr azm bias
    // ssr range bias
    // ssr range gain

    // ssr azm standard dev
    bool has_secondary_azimuth_stddev_{false};
    double secondary_azimuth_stddev_ {0};  // degrees
    // ssr range standard dev
    bool has_secondary_range_stddev_{false};
    double secondary_range_stddev_ {0};  // meters

    bool has_secondary_ir_min_ {false};
    int secondary_ir_min_ {0};  // nm

    bool has_secondary_ir_max_ {false};
    int secondary_ir_max_ {0};  // nm

    // mode s azm bias
    // mode s range bias
    // mode s range gain

    // mode s azm standard dev
    bool has_mode_s_azimuth_stddev_{false};
    double mode_s_azimuth_stddev_ {0};  // degrees
    // mode s range standard dev
    bool has_mode_s_range_stddev_{false};
    double mode_s_range_stddev_ {0};  // meters

    bool has_mode_s_ir_min_ {false};
    int mode_s_ir_min_ {0};  // nm

    bool has_mode_s_ir_max_ {false};
    int mode_s_ir_max_ {0};  // nm

  protected:
    virtual void checkSubConfigurables() {}
};

#endif  // STOREDDBODATASOURCE_H
