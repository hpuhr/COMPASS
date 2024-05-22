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

#include "colormap.h"
#include "colorscales.h"

#include <QComboBox>

namespace
{
    /**
    */
    std::vector<double> interpFactors(size_t n)
    {
        if (n == 0)
            return {};
        if (n == 1)
            return { 0.0 };

        const size_t b = n - 1;

        std::vector<double> f(n);
        for (size_t i = 0; i < n; ++i)
            f[ i ] = (double)i / (double)b;

        return f;
    };

    /**
    */
    std::vector<double> valueFactors(size_t n)
    {
        if (n == 0)
            return {};
        if (n == 1)
            return { 0.0 };
        if (n == 2)
            return { 0.0, 1.0 };

        std::vector<double> f(n);
        f[ 0 ] = 0.0;

        const size_t b = n - 2;

        for (size_t i = 0; i < n - 1; ++i)
            f[ i + 1 ] = (double)i / (double)b;

        return f;
    };

    /**
    */
    size_t numColorsFromType(ColorMap::Type type, size_t n)
    {
        size_t num_colors = 0;
        if (type == ColorMap::Type::Linear)
            num_colors = n + 2;
        else if (type == ColorMap::Type::Discrete)
            num_colors = n;
        else if (type == ColorMap::Type::Binary)
            num_colors = 2;

        assert(num_colors >= 1);

        return num_colors;
    };

    /**
    */
    Eigen::Vector3d colorQt2Eigen(const QColor& color)
    {
        return Eigen::Vector3d(color.redF(), color.greenF(), color.blueF());
    }

    /**
    */
    QColor colorEigen2Qt(const Eigen::Vector3d& color)
    {
        return QColor(color[ 0 ] * 255, color[ 1 ] * 255, color[ 2 ] * 255);
    }

    /**
    */
    QColor mix(const QColor& color0, const QColor& color1, double t0, double t1)
    {
        return QColor(color0.red()   * t0 + color1.red()   * t1,
                      color0.green() * t0 + color1.green() * t1,
                      color0.blue()  * t0 + color1.blue()  * t1);
    }
}

/**
*/
ColorMap::ColorMap()
{
    //init special colors
    special_colors_.resize(NumSpecialColors);

    special_colors_[ SpecialColorSelected ] = Qt::darkYellow;
    special_colors_[ SpecialColorNull     ] = Qt::darkMagenta;
}

/**
*/
ColorMap::~ColorMap() = default;

/**
*/
size_t ColorMap::numColors() const
{
    return n_colors_;
}

/**
*/
const QColor& ColorMap::getColor(size_t idx) const
{
    return colors_.at(idx);
}

/**
*/
QColor ColorMap::sample(double t, ColorScale scale)
{
    auto c = colorscale::sample(t, scale);
    return QColor(c[ 0 ] * 255, c[ 1 ] * 255, c[ 2 ] * 255);
}

/**
*/
std::vector<QColor> ColorMap::sample(ColorScale scale, 
                                     size_t n)
{
    std::vector<QColor> colors(n);
    size_t i = 0;
    for (double t : interpFactors(n))
        colors[ i++ ] = sample(t, scale);

    return colors;
}

/**
*/
std::vector<QColor> ColorMap::sample(const std::vector<QColor>& samples, 
                                     size_t n)
{
    size_t n_samples = samples.size();
    assert(n_samples >= 2);

    std::vector<Eigen::Vector3d> samples_01(n_samples);
    for (size_t i = 0; i < n_samples; ++i)
        samples_01[ i ] = colorQt2Eigen(samples[ i ]);

    std::vector<QColor> colors(n);
    size_t i = 0;
    for (double t : interpFactors(n))
        colors[ i++ ] = colorEigen2Qt(colorscale::sampleCustom(t, samples_01));

    return colors;
}

/**
*/
std::vector<QColor> ColorMap::sample(const QColor& color_min, 
                                     const QColor& color_max, 
                                     size_t n)
{
    return sample({ color_min, color_max }, n);
}

/**
*/
std::vector<QColor> ColorMap::sample(const QColor& color_min, 
                                     const QColor& color_mid, 
                                     const QColor& color_max,
                                     size_t n)
{
    return sample({ color_min, color_mid, color_max }, n);
}

/**
*/
void ColorMap::generateScaleImage(QImage& img, 
                                  ColorScale scale, 
                                  int w, 
                                  int h)
{
    img = QImage();
    
    if (w < 1 || h < 1)
        return;

    img = QImage(w, h, QImage::Format_ARGB32);

    auto factors = interpFactors(w);
    assert(factors.size() == (size_t)w);

    //@TODO: optimize
    for (int x = 0; x < w; ++x)
    {
        const QColor c = sample(factors[ x ], scale);

        for (int y = 0; y < h; ++y)
            img.setPixelColor(x, y, c);
    }
}

/**
*/
QComboBox* ColorMap::generateScaleSelection(ColorScale scale_default)
{
    const int IconW = 100;
    const int IconH = 6;

    QComboBox* combo = new QComboBox;
    combo->setIconSize(QSize(IconW, IconH));

    std::map<QString, std::pair<QImage, ColorScale>> entries;

    for (int i = 0; i < (int)ColorScale::NumColorScales; ++i)
    {
        ColorScale scale = (ColorScale)i;
        QString    name  = QString::fromStdString(colorscale::colorScale2String(scale));

        QImage img;
        generateScaleImage(img, scale, IconW, IconH);

        entries[ name ] = std::make_pair(img, scale);
    }

    int idx = -1;
    for (const auto& e : entries)
    {
        if (e.second.second == scale_default)
            idx = combo->count();

        combo->addItem(QIcon(QPixmap::fromImage(e.second.first)), e.first, QVariant((int)e.second.second));
    }

    if (idx >= 0)
        combo->setCurrentIndex(idx);

    return combo;
}

/**
*/
void ColorMap::create(const std::vector<QColor>& colors,
                      Type type)
{
    
    assert(type != Type::Binary   || colors.size() == 2);
    assert(type != Type::Discrete || colors.size() >= 1);
    assert(type != Type::Linear   || colors.size() >= 3);

    type_ = type;

    colors_        = colors;
    value_factors_ = valueFactors(colors_.size());
    n_colors_      = colors_.size();
}

/**
*/
void ColorMap::create(const QColor& color_min, 
                      const QColor& color_max, 
                      size_t n,
                      Type type)
{
    assert(n >= 1);

    auto colors = ColorMap::sample(color_min, color_max, numColorsFromType(type, n));
    create(colors, type);
}

/**
*/
void ColorMap::create(const QColor& color_min, 
                      const QColor& color_mid, 
                      const QColor& color_max, 
                      size_t n,
                      Type type)
{
    assert(n >= 1);
    
    auto colors = ColorMap::sample(color_min, color_mid, color_max, numColorsFromType(type, n));
    create(colors, type);
}

/**
*/
void ColorMap::create(ColorScale scale,
                      size_t n,
                      Type type)
{
    assert(n >= 1);

    auto colors = ColorMap::sample(scale, numColorsFromType(type, n));
    create(colors, type);
}

/**
*/
void ColorMap::setSpecialColor(SpecialColor type, const QColor& color)
{
    special_colors_.at(type) = color;
}

/**
*/
const QColor& ColorMap::specialColor(SpecialColor type) const
{
    return special_colors_.at(type);
}

/**
 * Mapping of factors to a color index / value interval.
 * 
 * Example - Binary colormap:
 *     t <  0.5 ... color0
 *     t >= 0.5 ... color1
 * 
 * Example - Linear colormap: (e.g. n = 5 => yields 7 colors, 5 inner intervals, and 6 value factors)
 * 
 *     colors_        = [ color0, color1, color2, color3, color4, color5, color6 ]
 *     value_factors_ = [ 0.0, 0.0, 0.2, 0.4, 0.6, 0.8, 1.0 ]
 * 
 *     t < 0.0        ... color0
 *     0.0 <= t < 0.2 ... color1
 *     0.2 <= t < 0.4 ... color2
 *     0.4 <= t < 0.6 ... color3
 *     0.6 <= t < 0.8 ... color4
 *     0.8 <= t < 1.0 ... color5
 *     t >= 1.0       ... color6
 * 
 *     => yielding idx: value_factors_[ idx ] <= t < value_factors_[ idx + 1 ]
 * 
 */
size_t ColorMap::indexFromFactor(double t) const
{
    //!discrete colormaps cannot be accessed by this method!
    assert (type_ != Type::Discrete);

    //handle binary case separately
    if (type_ == Type::Binary)
    {
        assert(n_colors_ == 2);
        return (t < 0.5 ? 0 : 1);
    }

    //linear colormap
    assert(type_ == Type::Linear);
    assert(n_colors_ >= 3);

    //handle maximum colors
    if (t < 0.0)
        return 0;
    if (t >= 1.0)
        return n_colors_ - 1;

    //t in [0, 1)
    const size_t n_inner = n_colors_ - 2;
    const size_t idx     = (size_t)std::floor(t * n_inner) + 1;

    assert(idx <= n_colors_ - 2);

    return idx;
}

/**
*/
QColor ColorMap::sample(double t) const
{
    size_t idx = indexFromFactor(t);

    return colors_.at(idx);
}

/**
*/
QColor ColorMap::sampleValue(double v, 
                             double vmin, 
                             double vmax) const
{
    assert(vmin <= vmax);

    const double vrange = vmax - vmin;
    if (vrange < 1e-12)
        return colors_.front();
    
    const double t = (v - vmin) / vrange;
    
    return sample(t);
}

/**
*/
QColor ColorMap::sampleValueSymm(double v, 
                                 double vmax) const
{
    const double vmax_abs = std::fabs(vmax);
    return sampleValue(v, -vmax_abs, vmax_abs);
}

/**
*/
std::vector<std::pair<QColor, std::string>> ColorMap::getDescription(double vmin, 
                                                                     double vmax) const
{
    std::vector<std::pair<QColor, std::string>> descr;

    descr.emplace_back(colors_.front(), "< " + std::to_string(vmin));

    const double vrange = vmax - vmin;

    for (size_t i = 1; i < n_colors_ - 1; ++i)
    {
        const double v0 = vmin + value_factors_[ i     ] * vrange;
        const double v1 = vmin + value_factors_[ i + 1 ] * vrange;
        descr.emplace_back(colors_[ i ], ">= " + std::to_string(v0) + " < " + std::to_string(v1));
    }

    descr.emplace_back(colors_.back(), ">= " + std::to_string(vmax));

    return descr;
}
