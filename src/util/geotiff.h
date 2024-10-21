
#pragma once

#include <string>

struct RasterReference;

class QImage;

/**
*/
class GeoTIFFWriter
{
public:
    static bool writeGeoTIFF(const std::string& fn,
                             const QImage& img,
                             const RasterReference& ref,
                             size_t subsampling = 1,
                             const std::string& warp_to_srs = "");
    static bool warpGeoTIFF(const std::string& fn,
                            const std::string& fn_out,
                            const std::string& warp_to_srs);
};
