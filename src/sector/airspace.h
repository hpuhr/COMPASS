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

#include <memory>
#include <string>

class SectorLayer;
class EvaluationTargetPosition;

/**
*/
class AirSpace
{
public:
    enum class InsideCheckResult
    {
        OutOfAirspace,
        AltitudeOOR,
        Inside
    };

    AirSpace();
    virtual ~AirSpace();

    void clear();
    bool readJSON(const std::string& fn);

    const SectorLayer& layer() const;

    InsideCheckResult isInside(const EvaluationTargetPosition& pos,
                               bool has_ground_bit, 
                               bool ground_bit_set,
                               bool evaluation_only) const;
    void isInside(InsideCheckResult& result_gb,
                  InsideCheckResult& result_no_gb,
                  const EvaluationTargetPosition& pos,
                  bool evaluation_only) const;

    size_t numEvaluationSectors() const;

    static const std::string LayerName;

private:
    std::unique_ptr<SectorLayer> layer_;
};
