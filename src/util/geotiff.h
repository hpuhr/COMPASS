
#pragma once

#include <string>

struct RasterReference;

class QImage;

/**
*/
class GeoTIFFWriter
{
public:
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
};
