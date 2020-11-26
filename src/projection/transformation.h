#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <memory>

class OGRSpatialReference;
class OGRCoordinateTransformation;

class Transformation
{
public:
    Transformation();
    Transformation (const Transformation& a); // only called during creation, slightly hacky
    virtual ~Transformation();


    std::tuple<bool, double, double> distanceCart (double lat1, double long1, double lat2, double long2);
    // ok, dist x, dist y
    std::tuple<bool, double, double> wgsAddCartOffset (double lat1, double long1, double x_pos2, double y_pos2);
    // ok, lat, long

protected:
    static bool in_appimage_;
    static const double max_wgs_dist_;

    std::unique_ptr<OGRSpatialReference> wgs84_;
    std::unique_ptr<OGRSpatialReference> local_;
    std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart_;
    std::unique_ptr<OGRCoordinateTransformation> ogr_cart2geo_;

    bool has_pos1_ {false};
    double lat1_;
    double long1_;

    void updateIfRequired(double lat1, double long1);
};

class FixedTransformation
{
public:
    FixedTransformation(double lat1, double long1);
    virtual ~FixedTransformation();

    std::tuple<bool, double, double> distanceCart (double lat2, double long2);
    // ok, dist x, dist y
    std::tuple<bool, double, double> wgsAddCartOffset (double x_pos2, double y_pos2);
    // ok, lat, long

protected:
    static bool in_appimage_;
    static const double max_wgs_dist_;

    std::unique_ptr<OGRSpatialReference> wgs84_;
    std::unique_ptr<OGRSpatialReference> local_;
    std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart_;
    std::unique_ptr<OGRCoordinateTransformation> ogr_cart2geo_;
};

#endif // TRANSFORMATION_H
