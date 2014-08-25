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

/*
 * AirspaceSector.cpp
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#include "AirspaceSector.h"
#include "GeographicPoint.h"
#include "Configuration.h"
#include "AirspaceSectorManager.h"

#include <algorithm>
#include "String.h"

using namespace Utils::String;

AirspaceSector::AirspaceSector(std::string class_id, std::string instance_id, Configurable *parent)
 : Configurable(class_id, instance_id, parent)
{
    registerParameter("name", &name_, "");
    registerParameter("own_volume", &has_own_volume_, false);
    registerParameter("own_height_min", &own_height_min_, -1.0);
    registerParameter("own_height_max", &own_height_max_, -1.0);
    registerParameter ("used_for_checking", &used_for_checking_, false);

    if (name_.size() == 0)
    {
        logerr << "AirspaceSector: constructor: name is null, setting to instance id '" << instance_id << "'";
        name_=instance_id;
    }
    misnomer_.setAltitudeMinMax(own_height_min_, own_height_max_);

    createSubConfigurables ();

    update();
}

AirspaceSector::~AirspaceSector()
{
    std::vector<AirspaceSector *>::iterator it;
    for (it = sub_sectors_.begin(); it != sub_sectors_.end(); it++)
        delete *it;
    sub_sectors_.clear();

    clearPoints ();
}

void AirspaceSector::checkSubConfigurables ()
{

}

void AirspaceSector::generateSubConfigurable (std::string class_id, std::string instance_id)
{
    if (class_id.compare ("AirspaceSector") == 0)
    {
        AirspaceSector *sector = new AirspaceSector (class_id, instance_id, this);
        //assert (sub_sectors_.find (sector->getName()) == sub_sectors_.end());
        sub_sectors_.push_back(sector);
        //loginf << "AirspaceSector: generateSubConfigurable: name " << name_ << " has new sub sector " << sector->getName();
    }
    else if (class_id.compare ("GeographicPoint") == 0)
    {
        GeographicPoint *point = new GeographicPoint (class_id, instance_id, this);
        assert (own_points_config_.find(point->getIndex()) == own_points_config_.end());
        own_points_config_[point->getIndex()] = point;
        //own_points_config_.push_back (point);
        //own_points_.push_back (std::pair<double, double> (point->getLatitude(), point->getLongitude()));
        //misnomer_.addPoint(point->getLatitude(), point->getLongitude());
    }
    else
        throw std::runtime_error ("AirspaceSectorManager: generateSubConfigurable: unknown class_id "+class_id );
}

void AirspaceSector::update ()
{
    misnomer_.clearPoints();
    own_points_.clear();

    Vector2 tmp;

    std::map <unsigned int, GeographicPoint *>::iterator it;

    for (it = own_points_config_.begin(); it != own_points_config_.end(); it++)
    {
        //own_points_.push_back (std::pair<double, double> (it->second->getLatitude(), it->second->getLongitude()));

        tmp.x_ = it->second->getLatitude();
        tmp.y_ = it->second->getLongitude();

        own_points_.push_back (tmp);
        misnomer_.addPoint(it->second->getLatitude(), it->second->getLongitude());
    }

}

void AirspaceSector::addPoint (double latitude, double longitude)
{
    has_own_volume_=true;
    Configuration &configuration = addNewSubConfiguration ("GeographicPoint");
    configuration.addParameterDouble ("latitude", latitude);
    configuration.addParameterDouble ("longitude", longitude);
    configuration.addParameterUnsignedInt ("index", own_points_config_.size());
    generateSubConfigurable(configuration.getClassId(), configuration.getInstanceId());

    update();
}

void AirspaceSector::addPoints (std::vector <Vector2> points)
{
    has_own_volume_=true;

    std::vector <Vector2>::iterator it;

    for (it = points.begin(); it != points.end(); it++)
    {
        Configuration &configuration = addNewSubConfiguration ("GeographicPoint");
        configuration.addParameterDouble ("latitude", it->x_);
        configuration.addParameterDouble ("longitude", it->y_);
        configuration.addParameterUnsignedInt ("index", own_points_config_.size());
        generateSubConfigurable(configuration.getClassId(), configuration.getInstanceId());
    }
    update();
}

void AirspaceSector::addPoints (std::string list)
{
    assert (has_own_volume_);

    list.erase (std::remove(list.begin(), list.end(), ' '), list.end()); //remove blanks

    std::vector<std::string>line_splits =  split(list, '\n');

    std::vector<std::string>::iterator it;
    std::vector<std::string>::iterator it2;

    std::string latitude_str, longitude_str;

    for (it = line_splits.begin(); it != line_splits.end(); it++)
    {
        std::vector<std::string>number_splits =  split(*it, ',');

        loginf << "AirspaceSector: addPoints: split '" << *it << "' into " << number_splits.size() << " pieces";

        if (number_splits.size() % 2 == 1)
        {
            logerr << "AirspaceSector: addPoints: parsing error at '" << *it << "'";
            return;
        }

        for (it2 = number_splits.begin(); it2 != number_splits.end(); it2 += 2)
        {
            latitude_str = *it2;
            longitude_str = *(it2+1);
            addPoint (doubleFromString(latitude_str), doubleFromString(longitude_str));
        }
    }

    update();
}

void AirspaceSector::clearPoints ()
{
    std::map <unsigned int, GeographicPoint *>::iterator it;

    for (it = own_points_config_.begin(); it != own_points_config_.end(); it++)
    {
        delete it->second;
    }
    own_points_config_.clear();
    own_points_.clear();
    misnomer_.clearPoints();
}

AirspaceSector *AirspaceSector::addNewSubSector (std::string name)
{
    Configuration &configuration = addNewSubConfiguration ("AirspaceSector");
    configuration.addParameterString ("name", name);
    generateSubConfigurable(configuration.getClassId(), configuration.getInstanceId());
    return sub_sectors_.back();
}

void AirspaceSector::removeSubSector (AirspaceSector *sector)
{
    assert (find (sub_sectors_.begin(), sub_sectors_.end(), sector) != sub_sectors_.end());
    sub_sectors_.erase(find (sub_sectors_.begin(), sub_sectors_.end(), sector));
}

void AirspaceSector::addAllVolumeSectors (std::vector<AirspaceSector *>& sectors)
{
    if (has_own_volume_)
    {
        assert (find (sectors.begin(), sectors.end(), this) == sectors.end());
        sectors.push_back(this);
    }

    std::vector<AirspaceSector *>::iterator it;
    for (it = sub_sectors_.begin(); it != sub_sectors_.end(); it++)
        (*it)->addAllVolumeSectors(sectors);
}

std::vector <Vector2>& AirspaceSector::getOwnPoints ()
{
    assert (has_own_volume_);

    return own_points_;
}

void AirspaceSector::setHeightMin (double value)
{
    own_height_min_=value;
    misnomer_.setAltitudeMinMax(own_height_min_, own_height_max_);
}

void AirspaceSector::setHeightMax (double value)
{
    own_height_max_=value;
    misnomer_.setAltitudeMinMax(own_height_min_, own_height_max_);
}

bool AirspaceSector::isPointInside (double latitude, double longitude, double height_ft, bool debug)
{
    if (!misnomer_.isFinalized())
        misnomer_.finalize(name_);

    return misnomer_.inside(latitude, longitude, height_ft, debug);
}

bool AirspaceSector::isPointInside (double latitude, double longitude, bool debug)
{
    if (!misnomer_.isFinalized())
        misnomer_.finalize(name_);

    return misnomer_.inside(latitude, longitude, debug);
}

void AirspaceSector::setName (std::string name)
{
    name_ = name;

    AirspaceSectorManager::getInstance().rebuildSectorNames();
}

std::vector <Vector2> AirspaceSector::getPointsBetween (double p1_lat, double p1_long, double p2_lat, double p2_long)
{
    loginf << "AirspaceSector: getPointsBetween: name " << name_;
    Vector2 start_point;
    start_point.x_=p1_lat;
    start_point.y_=p1_long;

    Vector2 stop_point;
    stop_point.x_=p2_lat;
    stop_point.y_=p2_long;

    int start_cnt;
    double start_distance;

    double tmp_distance;
    Vector2 tmp;

    loginf << "AirspaceSector::getPointsBetween: searching for start point";

    int cnt=0;
    int size = own_points_.size();
    int max_cnt=own_points_.size()-1;

    for (cnt=0; cnt < size; cnt++)
    {
        tmp_distance = sqrt(pow(start_point.x_ - own_points_.at(cnt).x_,2)+pow(start_point.y_ - own_points_.at(cnt).y_,2));

        if (cnt == 0)
        {
            start_cnt = cnt;
            start_distance = tmp_distance;
        }
        else
        {
            if (tmp_distance < start_distance)
            {
                start_cnt = cnt;
                start_distance = tmp_distance;
            }
        }
    }

    int stop_cnt;
    double stop_distance;

    loginf << "AirspaceSector::getPointsBetween: searching for stop point";
    cnt=0;

    for (cnt=0; cnt < size; cnt++)
    {
        tmp_distance = sqrt(pow(stop_point.x_ - own_points_.at(cnt).x_,2)+pow(stop_point.y_ - own_points_.at(cnt).y_,2));

        if (cnt == 0)
        {
            stop_cnt = cnt;
            stop_distance = tmp_distance;
        }
        else
        {
            if (tmp_distance < stop_distance)
            {
                stop_cnt = cnt;
                stop_distance = tmp_distance;
            }
        }
    }
    loginf << "AirspaceSector::getPointsBetween: assembling result, start cnt " << start_cnt << " stop cnt " << stop_cnt << " size " << size;
    std::vector <Vector2> result;

    int distance_pos = stop_cnt-start_cnt;

    if (distance_pos < 0)
        distance_pos += own_points_.size();

    int distance_neg = start_cnt-stop_cnt;

    if (distance_neg < 0)
        distance_neg += own_points_.size();

    cnt = start_cnt;

    if (distance_pos <= distance_neg)
    {
        loginf << "AirspaceSector::getPointsBetween: pos " << distance_pos;

        while (cnt != stop_cnt)
        {
            loginf << "AirspaceSector::getPointsBetween: pos cnt " << cnt << " " << own_points_.at(cnt).x_ << ", " << own_points_.at(cnt).y_;
            result.push_back(own_points_.at(cnt));

            cnt++;

            if (cnt > max_cnt)
                cnt = 0;
        }
    }
    else
    {
        loginf << "AirspaceSector::getPointsBetween: neg " << distance_neg;

        while (cnt != stop_cnt)
        {
            loginf << "AirspaceSector::getPointsBetween: neg cnt " << cnt << " " << own_points_.at(cnt).x_ << ", " << own_points_.at(cnt).y_;
            result.push_back(own_points_.at(cnt));

            cnt--;

            if (cnt == -1)
                cnt = max_cnt;
        }
    }

    loginf << "AirspaceSector::getPointsBetween: done with " << result.size() << " points";

    return result;
}

//double AirspaceSector::distanceFromLineSegmentToPoint( const Vector2 v, const Vector2 w, const Vector2 p, Vector2 * const q )
//{
//    const float distSq = v.DistanceToSquared( w ); // i.e. |w-v|^2 ... avoid a sqrt
//    if ( distSq == 0.0 )
//    {
//        // v == w case
//        (*q) = v;
//
//        return v.DistanceTo( p );
//    }
//
//    // consider the line extending the segment, parameterized as v + t (w - v)
//    // we find projection of point p onto the line
//    // it falls where t = [(p-v) . (w-v)] / |w-v|^2
//
//    const float t = ( p - v ).DotProduct( w - v ) / distSq;
//    if ( t < 0.0 )
//    {
//        // beyond the v end of the segment
//        (*q) = v;
//
//        //return v.DistanceTo( p );
//        return NAN;
//    }
//    else if ( t > 1.0 )
//    {
//        // beyond the w end of the segment
//        (*q) = w;
//
//        return NAN;
//        //return w.DistanceTo( p );
//    }
//
//    // projection falls on the segment
//    const Vector2 projection = v + ( ( w - v ) * t );
//
//    (*q) = projection;
//
//    return p.DistanceTo( projection );
//}

//double AirspaceSector::distanceFromLineSegmentToPoint( double segmentX1, float segmentY1, float segmentX2, float segmentY2, float pX, float pY, float *qX, float *qY )
//{
//    Vector2 q;
//
//    float distance = DistanceFromLineSegmentToPoint( Vector2( segmentX1, segmentY1 ), Vector2( segmentX2, segmentY2 ), Vector2( pX, pY ), &q );
//
//    (*qX) = q.x_;
//    (*qY) = q.y_;
//
//    return distance;
//}
