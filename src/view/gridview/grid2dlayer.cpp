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

#include "grid2dlayer.h"
#include "grid2d.h"

#include "logger.h"

#include <iostream>

/*********************************************************************************
 * Grid2DLayer
 *********************************************************************************/

const std::string Grid2DLayer::TagName      = "name";
const std::string Grid2DLayer::TagNumCellsX = "num_cells_x";
const std::string Grid2DLayer::TagNumCellsY = "num_cells_y";
const std::string Grid2DLayer::TagData      = "data";
const std::string Grid2DLayer::TagDataRaw   = "data_raw";
const std::string Grid2DLayer::TagFlags     = "flags";
const std::string Grid2DLayer::TagRef       = "reference";

/**
*/
bool Grid2DLayer::hasData() const
{
    return (data.cols() > 0 && data.rows() > 0);
}

/**
*/
bool Grid2DLayer::hasFlags() const
{
    return (hasData() && flags.cols() == data.cols() && flags.rows() == data.rows());
}

/**
*/
boost::optional<std::pair<double, double>> Grid2DLayer::range() const
{
    double vmin = std::numeric_limits<double>::max();
    double vmax = std::numeric_limits<double>::min();

    const double* d = data.data();

    bool has_range = false;

    size_t n = (size_t)data.size();

    for (size_t i = 0; i < n; ++i)
    {
        if (d[ i ] == Grid2D::InvalidValue)
            continue;

        if (d[ i ] < vmin) vmin = d[ i ];
        if (d[ i ] > vmax) vmax = d[ i ];

        has_range = true;
    }

    if (!has_range)
        return {};

    return std::make_pair(vmin, vmax);
}

/**
*/
nlohmann::json Grid2DLayer::toJSON(bool binary) const
{
    nlohmann::json obj;

    unsigned int nx = (unsigned int)data.cols();
    unsigned int ny = (unsigned int)data.rows();

    obj[ TagName      ] = name;
    obj[ TagRef       ] = ref.toJSON();
    obj[ TagNumCellsX ] = nx;
    obj[ TagNumCellsY ] = ny;

    if (binary)
    {
        size_t n = data.cols() * data.rows();

        //write data binary
        QByteArray ba;

        //add array
        ba.append((const char*)data.data(), n * sizeof(double));

        //code base 64
        QString byte_str(ba.toBase64());

        obj[ TagDataRaw ] = byte_str.toStdString();
    }
    else
    {
        //write data as json array
        nlohmann::json jdata = nlohmann::json::array();

        for (unsigned int y = 0; y < ny; ++y)
        {
            nlohmann::json jrow = nlohmann::json::array();
            for (unsigned int x = 0; x < nx; ++x)
                jrow.push_back(data(y, x));

            jdata.push_back(jrow);
        }

        obj[ TagData ] = jdata;
    }

    return obj;
}

/**
*/
bool Grid2DLayer::fromJSON(const nlohmann::json& obj)
{
    *this = {};

    if (!obj.is_object() ||
        !obj.contains(TagName) || 
        !obj.contains(TagNumCellsX) ||
        !obj.contains(TagNumCellsY) ||
        (!obj.contains(TagData) && !obj.contains(TagDataRaw)) ||
        !obj.contains(TagRef))
        return false;

    name = obj[ TagName ];

    if (!ref.fromJSON(obj[ TagRef ]))
        return false;

    unsigned int nx = obj[ TagNumCellsX ];
    unsigned int ny = obj[ TagNumCellsY ];

    data.resize(ny, nx);
    data.setZero();

    bool read_raw = obj.contains(TagDataRaw);

    if (read_raw)
    {
        //read data binary
        const auto& jdata = obj[ TagDataRaw ];

        if (!jdata.is_string())
            return false;

        //read binary
        std::string str = jdata;

        QByteArray ba_base64(str.data(), str.size());
        QByteArray ba = QByteArray::fromBase64(ba_base64);

        const double* data_raw = (const double*)(ba.data());

        memcpy(data.data(), data_raw, data.cols() * data.rows() * sizeof(double));
    }
    else
    {
        //read data from json array
        const auto& jdata = obj[ TagData ];
        if (!jdata.is_array() || jdata.size() != ny)
            return false;

        for (unsigned int y = 0; y < ny; ++y)
        {
            const auto& jrow = jdata[ y ];
            if (!jrow.is_array() || jrow.size() != nx)
                return false;

            for (unsigned int x = 0; x < nx; ++x)
                data(y, x) = jrow[ x ];
        }
    }

    return true;
}

/*********************************************************************************
 * Grid2DLayers
 *********************************************************************************/

const std::string Grid2DLayers::TagLayers = "layers";

/**
*/
Grid2DLayers::Grid2DLayers() = default;

/**
*/
Grid2DLayers::~Grid2DLayers() = default;

/**
*/
void Grid2DLayers::clear()
{
    layers_.clear();
    layer_map_.clear();
}

/**
*/
void Grid2DLayers::addLayer(const std::string& name, 
                            const RasterReference& ref,
                            const Eigen::MatrixXd& data,
                            const Eigen::MatrixXi& flags)
{
    size_t idx = layers_.size();

    LayerPtr l(new Grid2DLayer);
    l->name  = name;
    l->ref   = ref;
    l->data  = data;
    l->flags = flags;

    layer_map_[ name ].push_back(idx);
    layers_.push_back(std::move(l));
}

/**
*/
void Grid2DLayers::addLayer(LayerPtr&& layer)
{
    assert(layer);

    size_t idx = layers_.size();

    layer_map_[ layer->name ].push_back(idx);
    layers_.push_back(std::move(layer));
}

/**
*/
size_t Grid2DLayers::numLayers() const
{
    return layers_.size();
}

/**
*/
const Grid2DLayers::Layers& Grid2DLayers::layers() const
{
    return layers_;
}

/**
*/
const Grid2DLayer& Grid2DLayers::layer(size_t idx) const
{
    const auto& l = layers_.at(idx);
    assert(l);

    return *l;
}

/**
*/
const Grid2DLayer& Grid2DLayers::layer(const std::string& name) const
{
    assert(layer_map_.count(name));
    
    const auto& indices = layer_map_.at(name);
    assert(!indices.empty());

    const auto& l = layers_.at(indices[ 0 ]);
    assert(l);

    return *l;
}

/**
*/
std::vector<const Grid2DLayer*> Grid2DLayers::layers(const std::string& name) const
{
    assert(layer_map_.count(name));
    
    const auto& indices = layer_map_.at(name);

    std::vector<const Grid2DLayer*> lyrs;
    lyrs.reserve(indices.size());

    for (int idx : indices)
        lyrs.push_back(layers_.at(idx).get());

    return lyrs;
}

/**
*/
bool Grid2DLayers::hasLayer(const std::string& name) const
{
    return (layer_map_.count(name) > 0 && layer_map_.at(name).size() > 0);
}

/**
*/
nlohmann::json Grid2DLayers::toJSON(bool binary) const
{
    nlohmann::json obj;

    nlohmann::json jlayers = nlohmann::json::array();

    for (const auto& l : layers_)
        jlayers.push_back(l->toJSON(binary));

    obj[ TagLayers ] = jlayers;

    return obj;
}

/**
*/
bool Grid2DLayers::fromJSON(const nlohmann::json& obj)
{
    clear();

    if (!obj.is_object() || !obj.contains(TagLayers))
        return false;

    const auto& jlayers = obj[ TagLayers ];
    if (!jlayers.is_array())
        return false;

    for (const auto& jl : jlayers)
    {
        LayerPtr l(new Grid2DLayer);
        if (l->fromJSON(jl))
            return false;

        addLayer(std::move(l));
    }

    return true;
}

/*********************************************************************************
 * Grid2DLayerRenderer
 *********************************************************************************/

/**
*/
std::pair<QImage,RasterReference> Grid2DLayerRenderer::render(const Grid2DLayer& layer,
                                                              const Grid2DRenderSettings& settings)
{
    if (!layer.hasData())
        return {};

    const auto& data  = layer.data;
    const auto& flags = layer.flags;

    bool has_flags = layer.hasFlags();

    //create empty (transparent) image
    int pixels_per_cell = std::max(1, settings.pixels_per_cell);

    size_t nrows  = layer.data.rows();
    size_t ncols  = layer.data.cols();

    size_t nrows_out = nrows * pixels_per_cell;
    size_t ncols_out = ncols * pixels_per_cell;

    QImage img(ncols_out, nrows_out, QImage::Format_ARGB32);
        img.fill(QColor(0, 0, 0, 0));

    //create reference corrected for oversampling
    RasterReference ref = layer.ref;

    ref.img_pixel_size_x /= pixels_per_cell;
    ref.img_pixel_size_y /= pixels_per_cell;

    //determine range
    auto range = layer.range();

    double vmin, vmax;

    if (settings.min_value.has_value() && settings.max_value.has_value())
    {
        vmin = settings.min_value.value();
        vmax = settings.max_value.value();
    }
    else if (range.has_value())
    {
        vmin = range.value().first;
        vmax = range.value().second;
    }
    else
    {
        //no range => cannot render
        return std::make_pair(img, ref);
    }

    //std::cout << "vmin: " << vmin << ", vmax: " << vmax << std::endl;

    double vrange = vmax - vmin;
    
    auto getColor = [ & ] (size_t cx, size_t cy)
    {
        //handle cell selection
        if (settings.show_selected && has_flags && (flags(cy, cx) & grid2d::CellFlags::CellSelected))
            return settings.color_map.specialColor(ColorMap::SpecialColorSelected);

        //flip y-index if north is up
        if (layer.ref.is_north_up)
            cy = nrows - 1 - cy;

        if (data(cy, cx) == Grid2D::InvalidValue)
            return QColor(0, 0, 0, 0);

        double t = (data(cy, cx) - vmin) / vrange;

        return settings.color_map.sample(t);
    };

    for (size_t y = 0, y0 = 0; y < nrows; ++y, y0 += pixels_per_cell)
    {
        for (size_t x = 0, x0 = 0; x < ncols; ++x, x0 += pixels_per_cell)
        {
            QColor col = getColor(x, y);

            for (int y2 = 0; y2 < pixels_per_cell; ++y2)
                for (int x2 = 0; x2 < pixels_per_cell; ++x2)
                    img.setPixelColor(x0 + x2, y0 + y2, col);
        }
    }

    return std::make_pair(img, ref);
}
