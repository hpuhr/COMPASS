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

#pragma once

#include "configurable.h"
#include "singleton.h"
#include "buffer.h"

class ProjectionManagerWidget;
class Projection;
class OGRProjection;

/**
 * @brief Singleton for coordinate projection handling
 *
 * Currently handles projection from world coordinates to Cartesian coordinates using the WGS-84
 * method.
 */
class ProjectionManager : public Singleton, public Configurable
{
protected:
    /// @brief Constructor
    ProjectionManager();

public:
    /// @brief Desctructor
    virtual ~ProjectionManager();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    /// @brief Projects cartesian coordinate to geo-coordinate in WGS-84, returns false on error
    // bool sdlGRS2Geo (t_CPos grs_pos, t_GPos& geo_pos);

    ProjectionManagerWidget* widget();
    void deleteWidget();

    static ProjectionManager& instance()
    {
        static ProjectionManager instance;
        return instance;
    }
    std::string currentProjectionName() const;
    void currentProjectionName(const std::string& name);

    bool hasProjection(const std::string& name);
    Projection& projection(const std::string& name);
    bool hasCurrentProjection();
    Projection& currentProjection();

    std::map<std::string, std::unique_ptr<Projection>>& projections();

    OGRProjection& ogrProjection();

    // in place calculation, returns transformation errors count
    unsigned int doRadarPlotPositionCalculations (std::map<std::string, std::shared_ptr<Buffer>> buffers);
    // returns transformation errors count, update buffers
    std::pair<unsigned int, std::map<std::string, std::shared_ptr<Buffer>>>
      doUpdateRadarPlotPositionCalculations (std::map<std::string, std::shared_ptr<Buffer>> buffers);

    static const std::string RS2G_NAME;
    static const std::string OGR_NAME;

protected:

    std::string current_projection_name_;

    std::unique_ptr<ProjectionManagerWidget> widget_;

    std::map<std::string, std::unique_ptr<Projection>> projections_;

    virtual void checkSubConfigurables();

    unsigned int calculateRadarPlotPositions (std:: string dbcontent_name, std::shared_ptr<Buffer> buffer,
                                              NullableVector<double>& target_latitudes_vec,
                                              NullableVector<double>& target_longitudes_vec);
};

