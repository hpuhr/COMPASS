
#pragma once

#include <string>
#include <vector>

#include <QRectF>

struct RasterReference;

class QImage;
class QSize;

/**
*/
struct GeoTiffInfo
{
    QRectF roi() const;

    bool                valid = false;
    int                 img_w = -1;
    int                 img_h = -1;
    std::vector<double> geo_transform;
    std::string         geo_srs;
};

/**
*/
class GeoTIFFWriter
{
public:
    static GeoTiffInfo getInfo(const std::string& fn, bool vmem = false);
    static bool isValidGeoTIFF(const std::string& fn);

    static bool writeGeoTIFF(const std::string& fn,
                             const QImage& img,
                             const RasterReference& ref,
                             const std::string& warp_to_srs = "",
                             size_t subsampling = 1);
    static bool warpGeoTIFF(const std::string& fn,
                            const std::string& fn_out,
                            const std::string& warp_to_srs,
                            size_t subsampling = 1);

    static std::string wktStringFromSRSName(const std::string& srs_name);
}; 
