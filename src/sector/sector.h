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

#ifndef SECTOR_H
#define SECTOR_H

#include "json.hpp"

#include <QColor>
#include <QRectF>
#include <QImage>

#include <memory>

#include <boost/optional.hpp>

class DBInterface;
class EvaluationTargetPosition;

class OGRPolygon;

/**
 * Fast sector inside test via image descretization.
 * 
 * Generates an discrete image map representation of the input polygon and checks this 
 * representation for definitely inside, definitely outside and dubious points.
 * Dubious points reside near the polygon border and can be checked later on with a
 * more accurate geometrical inside test. 
 */
class SectorInsideTest
{
public:
    enum class CheckResult
    {
        Inside = 0,
        Outside,
        Border
    };

    SectorInsideTest();
    SectorInsideTest(const std::vector<std::pair<double,double>>& points, 
                     double xmin, 
                     double ymin, 
                     double xmax, 
                     double ymax);
    virtual ~SectorInsideTest();

    CheckResult isInside(double x, double y) const;

    bool isValid() const;

    const QImage& image() const { return img_; }

private:
    static const int ImageSize    = 1000; //maximum image map extent (data region aspect will be preserved)
    static const int ImgBorder    = 5;    //border added to the image map for safety
    static const int BorderRegion = 7;    //width of the polygon border region (= dubious region)

    void map2Image(double& xm, double& ym, double x, double y) const;

    void create(const std::vector<std::pair<double,double>>& points, 
                double xmin, 
                double ymin, 
                double xmax, 
                double ymax);

    QImage img_;
    double xmin_ = 0;
    double ymin_ = 0;
    double tx_   = 1;
    double ty_   = 1;
};

class Sector
{
public:
    // should be protected?
    Sector(unsigned int id, const std::string& name, const std::string& layer_name,
           bool exclude, QColor color, std::vector<std::pair<double,double>> points);
    Sector(unsigned int id, const std::string& name, const std::string& layer_name,
           const std::string& json_str);
    virtual ~Sector();

    unsigned int id() const;

    std::string name() const;
    void name(const std::string& name);

    std::string layerName() const;
    void layerName(const std::string& layer_name);

    bool exclude() const;
    void exclude(bool value);

    nlohmann::json jsonData() const;
    std::string jsonDataStr() const;

    unsigned int size () { return points_.size(); }

    const std::vector<std::pair<double, double>>& points() const;

    bool hasMinimumAltitude();
    double minimumAltitude();
    void minimumAltitude(double value);
    void removeMinimumAltitude();

    bool hasMaximumAltitude();
    double maximumAltitude();
    void maximumAltitude(double value);
    void removeMaximumAltitude();

    std::string colorStr();
    void colorStr(std::string value);
    void removeColorStr();

    void save();

    bool isInside(const EvaluationTargetPosition& pos, 
                  bool has_ground_bit, 
                  bool ground_bit_set) const;

    std::pair<double, double> getMinMaxLatitude() const;
    std::pair<double, double> getMinMaxLongitude() const;

    void createFastInsideTest() const;

protected:
    void createPolygon();

    unsigned int id_;
    std::string name_;
    std::string layer_name_;

    bool exclude_ {false};
    std::string color_str_;

    std::vector<std::pair<double,double>> points_;

    bool has_min_altitude_ {false};
    double min_altitude_{0.0};

    bool has_max_altitude_ {false};
    double max_altitude_{0.0};

    boost::optional<double> lat_min_;
    boost::optional<double> lat_max_;
    boost::optional<double> lon_min_;
    boost::optional<double> lon_max_;

    mutable boost::optional<SectorInsideTest> inside_test_;

    std::unique_ptr<OGRPolygon> ogr_polygon_;


};

#endif // SECTOR_H
