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

#include "json.h"

#include <string>
#include <vector>

#include <QRectF>

/**
*/
struct RasterReference
{
    RasterReference() {};

    void set(const std::string& srs_str,
             const QRectF& roi,
             int w,
             int h,
             bool srs_is_north_up)
    {
        srs              = srs_str;
        img_origin_x     = roi.left();
        img_origin_y     = srs_is_north_up ? roi.bottom() : roi.top();
        img_pixel_size_x = roi.width()  / w;
        img_pixel_size_y = roi.height() / h;
        is_north_up      = srs_is_north_up;
    }

    QRectF getROI(int w, int h) const
    {
        double roi_w = w * img_pixel_size_x;
        double roi_h = h * img_pixel_size_y;
        double roi_x = img_origin_x;
        double roi_y = is_north_up ? img_origin_y - roi_h : img_origin_y;

        return QRectF(roi_x, roi_y, roi_w, roi_h);
    }

    std::vector<double> geoTransform(int offsx = 0, int offsy = 0) const
    {
        const double dy = is_north_up ? -img_pixel_size_y : img_pixel_size_y;

        return { img_origin_x + offsx * img_pixel_size_x, 
                 img_pixel_size_x, 
                 0, 
                 img_origin_y + offsy * dy, 
                 0, 
                 dy };
    }

    RasterReference subsampled(int samples) const
    {
        RasterReference r = *this;
        if (samples <= 1)
            return r;
        
        r.img_pixel_size_x /= samples;
        r.img_pixel_size_y /= samples;

        return r;
    }

    bool fromJSON(const nlohmann::json& json_ref)
    {
        *this = {};

        if (!json_ref.contains("srs")              ||
            !json_ref.contains("img_origin_x")     ||
            !json_ref.contains("img_origin_y")     ||
            !json_ref.contains("img_pixel_size_x") ||
            !json_ref.contains("img_pixel_size_y"))
            return false;

        srs              = json_ref[ "srs" ];
        img_origin_x     = json_ref[ "img_origin_x"   ];
        img_origin_y     = json_ref[ "img_origin_y"    ];
        img_pixel_size_x = json_ref[ "img_pixel_size_x" ];
        img_pixel_size_y = json_ref[ "img_pixel_size_y" ];

        is_north_up = true;
        if (json_ref.contains("is_north_up"))
            is_north_up = json_ref[ "is_north_up" ];

        return true;
    }

    nlohmann::json toJSON() const
    {
        nlohmann::json j;
        j[ "srs"              ] = srs;
        j[ "img_origin_x"     ] = img_origin_x;
        j[ "img_origin_y"     ] = img_origin_y;
        j[ "img_pixel_size_x" ] = img_pixel_size_x;
        j[ "img_pixel_size_y" ] = img_pixel_size_y;
        j[ "is_north_up"      ] = is_north_up;

        return j;
    }

    std::string srs;                  // srs name, either a well-known system identifier (e.g. "wgs84") or an epsg code (e.g. "epsg:3857")
    double      img_origin_x;         // x-coordinate of the left-upper corner of the image
    double      img_origin_y;         // y-coordinate of the left-upper corner of the image
    double      img_pixel_size_x;     // width of a pixel in srs units
    double      img_pixel_size_y;     // height of a pixel in srs units
    bool        is_north_up = true;   // is the srs north-up (most likely)
};
