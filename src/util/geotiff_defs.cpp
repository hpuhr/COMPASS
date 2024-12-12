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

#include "geotiff_defs.h"
#include "rasterreference.h"

/**
*/
QRectF GeoTIFFInfo::roi() const
{
    if (!valid || img_w < 1 || img_h < 1)
        return QRectF();

    RasterReference ref;
    ref.set(geo_srs, geo_transform);

    return ref.getROI(img_w, img_h);
}

/**
 */
void RawRasterData::init(size_t nx, size_t ny, size_t subsampling)
{
    raster_x_size = nx * subsampling;
    raster_y_size = ny * subsampling;
    raster_n      = raster_x_size * raster_y_size;

    channel_r.assign(raster_n, 0  );
    channel_g.assign(raster_n, 0  );
    channel_b.assign(raster_n, 0  );
    channel_a.assign(raster_n, 255);
}
