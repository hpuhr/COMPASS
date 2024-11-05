
#include "geotiff.h"

#include "rasterreference.h"

#include "logger.h"
#include "files.h"

#include <cassert>

#include <QImage>

#include <gdal.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>

#include <boost/filesystem.hpp>

/**
*/
bool GeoTIFFWriter::isValidGeoTIFF(const std::string& fn)
{
    if (!Utils::Files::fileExists(fn))
        return false;

    //try to open
    auto dataset = GDALOpen(fn.c_str(), GDALAccess::GA_ReadOnly);
    if (!dataset)
        return false;

    //try to get projection
    if (!GDALGetProjectionRef(dataset))
        return false;

    return true;
}

namespace helpers
{

std::string wktStringFromSRSName(const std::string& srs_name)
{
    char* srs_wkt = NULL;
    OGRSpatialReferenceH srs = OSRNewSpatialReference( NULL );

    auto srs_string_norm = QString::fromStdString(srs_name).toLower();

    if (srs_string_norm.startsWith("epsg:"))
    {
        //set from epsg
        auto parts = srs_string_norm.split(':');
        if (parts.size() == 2 && !parts[ 0 ].isEmpty() && !parts[ 1 ].isEmpty())
        {
            bool ok;
            int epsg = parts[ 1 ].toInt(&ok);

            if (ok)
                OSRImportFromEPSG(srs, epsg);
        }
    }
    else
    {
        //set from well-known system
        OSRSetWellKnownGeogCS(srs, srs_name.c_str());
    }
    OSRExportToWkt(srs, &srs_wkt);
    OSRDestroySpatialReference(srs);

    return std::string(srs_wkt);
}

}

/**
*/
bool GeoTIFFWriter::writeGeoTIFF(const std::string& fn,
                                 const QImage& img,
                                 const RasterReference& ref,
                                 const std::string& warp_to_srs,
                                 size_t subsampling)
{
    if (img.isNull())
        return false;

    loginf << "GeoTIFFWriter: writeGeoTIFF:\n" << "   fn = " << fn << "\n"
                                               << "   srs = " << ref.srs 
                                               << " (x0 " << ref.img_origin_x 
                                               << ", y0 " << ref.img_origin_y 
                                               << ", dx " << ref.img_pixel_size_x 
                                               << ", dy " << ref.img_pixel_size_y 
                                               << ", is_nu " << ref.is_north_up << ")";
    //split image data into layers
    size_t w_in = (size_t)img.width();
    size_t h_in = (size_t)img.height();

    size_t w_out = (size_t)img.width()  * subsampling;
    size_t h_out = (size_t)img.height() * subsampling;
    size_t n_out = w_out * h_out;

    loginf << "GeoTIFFWriter: writeGeoTIFF: img w = " << w_in << ", h = " << h_in << ", depth = " << img.depth();
    loginf << "GeoTIFFWriter: writeGeoTIFF: layer w = " << w_out << ", h = " << h_out << ", subsampling = " << subsampling;

    std::string fn_in  = fn;
    std::string fn_out = fn;

    if (!warp_to_srs.empty())
    {
        boost::filesystem::path p(fn);

        std::string basename = p.stem().string();
        if (basename.empty())
            return false;

        fn_in = (p.parent_path() / (basename + "_in.tif")).string();
    }

    loginf << "GeoTIFFWriter: writeGeoTIFF: input filename = " << fn_in;

    GDALDatasetH dataset;

    bool zero_is_no_data = false;

    //create dataset
    {
        std::vector<unsigned char> channel_r(n_out, 0  );
        std::vector<unsigned char> channel_g(n_out, 0  );
        std::vector<unsigned char> channel_b(n_out, 0  );
        std::vector<unsigned char> channel_a(n_out, 255);

        const size_t chunk_size = subsampling * w_out;

        uint8_t r, g, b, a;
        size_t y_in, x_in;       // position in input image
        size_t x_block, y_block; // position in subblock
        size_t line_idx_out;     // output index of current subblock's origin line
        size_t idx_out;          // output index of current subblock's origin sample
        size_t line_idx_block;   // output index of current line in subblock
        size_t idx_block;        // output index of current sample in subblock

        //iterate over input image
        for (y_in = 0, line_idx_out = 0; y_in < h_in; ++y_in, line_idx_out += chunk_size)
        {
            //get input image's current line
            QRgb* line = (QRgb*)img.scanLine(y_in);

            for (x_in = 0, idx_out = line_idx_out; x_in < w_in; ++x_in, ++line, idx_out += subsampling)
            {
                //get input image's current sample
                a = qAlpha(*line);
                r = zero_is_no_data ? (a == 0 ? 0 : std::max(1, qRed  (*line))) : qRed  (*line);
                g = zero_is_no_data ? (a == 0 ? 0 : std::max(1, qGreen(*line))) : qGreen(*line);
                b = zero_is_no_data ? (a == 0 ? 0 : std::max(1, qBlue (*line))) : qBlue (*line);

                //write subsampling block to output channels
                for (y_block = 0, line_idx_block = idx_out; y_block < subsampling; ++y_block, line_idx_block += w_out)
                {
                    for (x_block = 0, idx_block = line_idx_block; x_block < subsampling; ++x_block, ++idx_block)
                    {
                        assert(idx_block <= n_out);
                        channel_r[idx_block] = r;
                        channel_g[idx_block] = g;
                        channel_b[idx_block] = b;
                        channel_a[idx_block] = a;
                    }
                }
            }
        }

        //create gtiff dataset in gdal virtual filesystem
        auto gtiff_driver = GDALGetDriverByName("GTiff");

        char **papszOptions = NULL;
        dataset = GDALCreate(gtiff_driver, fn_in.c_str(), w_out, h_out, 4, GDT_Byte, papszOptions);
    
        //write image data to raster bands
        auto rBand = GDALGetRasterBand(dataset, 1);
        auto gBand = GDALGetRasterBand(dataset, 2);
        auto bBand = GDALGetRasterBand(dataset, 3);
        auto aBand = GDALGetRasterBand(dataset, 4);

#if 0
        //comment: not working for oldos gdal
        if (zero_is_no_data)
        {
            GDALSetRasterNoDataValueAsUInt64(rBand, 0u);
            GDALSetRasterNoDataValueAsUInt64(gBand, 0u);
            GDALSetRasterNoDataValueAsUInt64(bBand, 0u);
            GDALSetRasterNoDataValueAsUInt64(aBand, 0u);
        }
#endif

        auto err_r = GDALRasterIO(rBand, GF_Write, 0, 0, w_out, h_out, channel_r.data(), w_out, h_out, GDT_Byte, 0, 0 );
        auto err_g = GDALRasterIO(gBand, GF_Write, 0, 0, w_out, h_out, channel_g.data(), w_out, h_out, GDT_Byte, 0, 0 );
        auto err_b = GDALRasterIO(bBand, GF_Write, 0, 0, w_out, h_out, channel_b.data(), w_out, h_out, GDT_Byte, 0, 0 );
        auto err_a = GDALRasterIO(aBand, GF_Write, 0, 0, w_out, h_out, channel_a.data(), w_out, h_out, GDT_Byte, 0, 0 );

        GDALSetRasterColorInterpretation(rBand, GDALColorInterp::GCI_RedBand);
        GDALSetRasterColorInterpretation(gBand, GDALColorInterp::GCI_GreenBand);
        GDALSetRasterColorInterpretation(bBand, GDALColorInterp::GCI_BlueBand);
        GDALSetRasterColorInterpretation(aBand, GDALColorInterp::GCI_AlphaBand);

        loginf << "GeoTIFFWriter: writeGeoTIFF: raster errors: " << err_r << ", " << err_g << ", " << err_b << ", " << err_a ;

        //set geo transform
        auto gt = ref.subsampled(subsampling).geoTransform();
        GDALSetGeoTransform(dataset, gt.data());

        loginf << "GeoTIFFWriter: writeGeoTIFF: geotransform = " << gt[ 0 ] << ", " << gt[ 1 ] << ", " << gt[ 2 ] << ", " << gt[ 3 ] << ", " << gt[ 4 ] << ", " << gt[ 5 ];

        //obtain and set wkt string of projection
        auto wkt_string = helpers::wktStringFromSRSName(ref.srs);
        GDALSetProjection(dataset, wkt_string.c_str());

        // auto dataset_tmp = GDALCreateCopy(gtiff_driver, "/home/mcphatty/grid.png", dataset, 0, NULL, NULL, NULL);
        // GDALClose(dataset_tmp);

        loginf << "GeoTIFFWriter: writeGeoTIFF: wkt = " << wkt_string;
    }

    //warp dataset to other srs
    if (!warp_to_srs.empty())
    {
        loginf << "GeoTIFFWriter: writeGeoTIFF: warping...\n\n" << GDALGetProjectionRef(dataset) << "\n =>\n" << warp_to_srs << "\n";

        //GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();

        auto dataset_warped = GDALAutoCreateWarpedVRT(dataset, 
                                                      GDALGetProjectionRef(dataset), 
                                                      warp_to_srs.c_str(), 
                                                      GDALResampleAlg::GRA_NearestNeighbour,
                                                      0.001,
                                                      0);

        //create a copy of the warped system using gtiff driver (not sure if this intermdiate step is needed)
        auto gtiff_driver  = GDALGetDriverByName("GTiff");
        auto dataset_layer = GDALCreateCopy(gtiff_driver, fn_out.c_str(), dataset_warped, 0, NULL, NULL, NULL);

        // auto dataset_tmp = GDALCreateCopy(gtiff_driver, "/home/mcphatty/grid_warped.png", dataset_warped, 0, NULL, NULL, NULL);
        // GDALClose(dataset_tmp);

        //close all datasets and delete unwarped file from (virtual) mem
        GDALClose(dataset_warped);
        GDALClose(dataset);
        GDALClose(dataset_layer);
        GDALDeleteDataset(gtiff_driver, fn_in.c_str());

        //GDALDestroyWarpOptions(psWarpOptions);

        loginf << "GeoTIFFWriter: writeGeoTIFF: warped";
    }
    else
    {
        GDALClose(dataset);
    }

    return true;
}

/**
*/
bool GeoTIFFWriter::warpGeoTIFF(const std::string& fn,
                                const std::string& fn_out,
                                const std::string& warp_to_srs,
                                size_t subsampling)
{
    auto dataset = GDALOpen(fn.c_str(), GDALAccess::GA_ReadOnly);
    if (!dataset)
        return false;

    //@TODO: subsampling

    loginf << "GeoTIFFWriter: warpGeoTIFF: fn = " << fn;
    loginf << "GeoTIFFWriter: warpGeoTIFF: fn_out = " << fn_out;
    loginf << "GeoTIFFWriter: warpGeoTIFF: warping...\n\n" << GDALGetProjectionRef(dataset) << "\n =>\n" << warp_to_srs << "\n";

    //GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();

    auto dataset_warped = GDALAutoCreateWarpedVRT(dataset, 
                                                  GDALGetProjectionRef(dataset), 
                                                  warp_to_srs.c_str(), 
                                                  GDALResampleAlg::GRA_NearestNeighbour,
                                                  0.001,
                                                  0);
    if (!dataset_warped)
        return false;

    //create a copy of the warped system using gtiff driver (not sure if this intermdiate step is needed)
    auto gtiff_driver  = GDALGetDriverByName("GTiff");
    auto dataset_layer = GDALCreateCopy(gtiff_driver, fn_out.c_str(), dataset_warped, 0, NULL, NULL, NULL);

    //close all datasets and delete unwarped file from (virtual) mem
    GDALClose(dataset_warped);
    GDALClose(dataset);
    GDALClose(dataset_layer);

    //GDALDestroyWarpOptions(psWarpOptions);

    loginf << "GeoTIFFWriter: warpGeoTIFF: warped";

    return true;
}
