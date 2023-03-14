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

#include "sectorlayer.h"
#include "sector.h"
#include "evaluationtargetposition.h"
#include "logger.h"

#include <cassert>

using namespace std;

SectorLayer::SectorLayer(const std::string& name)
  : name_(name)
{
}

std::string SectorLayer::name() const
{
    return name_;
}

bool SectorLayer::hasSector (const std::string& name)
{
    auto iter = std::find_if(sectors_.begin(), sectors_.end(),
                             [&name](const shared_ptr<Sector>& x) { return x->name() == name;});

    return iter != sectors_.end();
}

void SectorLayer::addSector (std::shared_ptr<Sector> sector)
{
    assert (!hasSector(sector->name()));
    assert (sector->layerName() == name_);
    sectors_.push_back(sector);

    has_exclude_sector_ |= sector->exclude();
}

std::shared_ptr<Sector> SectorLayer::sector (const std::string& name)
{
    assert (hasSector(name));

    auto iter = std::find_if(sectors_.begin(), sectors_.end(),
                             [&name](const shared_ptr<Sector>& x) { return x->name() == name;});
    assert (iter != sectors_.end());

    return *iter;
}

void SectorLayer::removeSector (std::shared_ptr<Sector> sector)
{
    assert (hasSector(sector->name()));

    string name = sector->name();
    auto iter = std::find_if(sectors_.begin(), sectors_.end(),
                             [&name](const shared_ptr<Sector>& x) { return x->name() == name;});
    assert (iter != sectors_.end());

    sectors_.erase(iter);
    assert (!hasSector(sector->name()));
}

bool SectorLayer::isInside(const EvaluationTargetPosition& pos,
                           bool has_ground_bit, 
                           bool ground_bit_set)  const
{
    bool is_inside         = false;
    bool is_inside_exclude = false;

    // check if inside normal ones
    for (auto& sec_it : sectors_)
    {
        if (sec_it->exclude())
            continue;

        if (sec_it->isInside(pos, has_ground_bit, ground_bit_set))
        {
            logdbg << "SectorLayer " << name_ << ": isInside: true, has alt " << pos.has_altitude_
                   << " alt " << pos.altitude_ << " exclude " << sec_it->exclude();

            is_inside = true;
            break;
        }
    }

    if (!is_inside) // not inside normal sector
        return false;

    // is inside normal sector

    if (!has_exclude_sector_) // nothin more to check
        return true;

    // check if inside exclude ones
    for (auto& sec_it : sectors_)
    {
        if (!sec_it->exclude())
            continue;

        if (sec_it->isInside(pos, has_ground_bit, ground_bit_set))
        {
            logdbg << "SectorLayer " << name_ << ": isInside: true, has alt " << pos.has_altitude_
                   << " alt " << pos.altitude_ << " exclude " << sec_it->exclude();

            is_inside_exclude = true;
            break;
        }
    }

    return !is_inside_exclude; // true if in no exclude, false if in include
}

std::pair<double, double> SectorLayer::getMinMaxLatitude() const
{
    double min, max;
    double tmp_min, tmp_max;
    bool first = true;

    assert (sectors_.size());

    for (auto& sec_it : sectors_)
    {
        if (first)
        {
            tie (min, max) = sec_it->getMinMaxLatitude();
            first = false;
        }
        else
        {
            tie (tmp_min, tmp_max) = sec_it->getMinMaxLatitude();
            min = std::min(min, tmp_min);
            max = std::max(max, tmp_max);
        }
    }

    return {min, max};
}

std::pair<double, double> SectorLayer::getMinMaxLongitude() const
{
    double min, max;
    double tmp_min, tmp_max;
    bool first = true;

    assert (sectors_.size());

    for (auto& sec_it : sectors_)
    {
        if (first)
        {
            tie (min, max) = sec_it->getMinMaxLongitude();
            first = false;
        }
        else
        {
            tie (tmp_min, tmp_max) = sec_it->getMinMaxLongitude();
            min = std::min(min, tmp_min);
            max = std::max(max, tmp_max);
        }
    }

    return {min, max};
}
