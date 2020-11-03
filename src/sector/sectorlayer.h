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

#ifndef SECTORLAYER_H
#define SECTORLAYER_H

#include <string>
#include <vector>
#include <memory>

class Sector;
class EvaluationTargetPosition;

class SectorLayer
{
public:
    SectorLayer(const std::string& name);

    std::string name() const;

    bool hasSector (const std::string& name);
    void addSector (std::shared_ptr<Sector> sector);
    std::shared_ptr<Sector> sector (const std::string& name);
    void removeSector (std::shared_ptr<Sector> sector);

    unsigned int size () { return sectors_.size(); };

    std::vector<std::shared_ptr<Sector>>& sectors() { return sectors_; }

    bool isInside(const EvaluationTargetPosition& pos) const;

    std::pair<double, double> getMinMaxLatitude() const;
    std::pair<double, double> getMinMaxLongitude() const;

protected:
    const std::string name_;

    std::vector<std::shared_ptr<Sector>> sectors_;
};

#endif // SECTORLAYER_H
