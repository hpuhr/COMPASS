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

#include "colormap_defs.h"

#include <vector>

#include <QColor>

class QComboBox;
class QImage;

/**
*/
class ColorMap
{
public:
    typedef colorscale::ColorScale ColorScale;

    enum class Type
    {
        Linear = 0,
        Discrete,
        Binary
    };

    enum SpecialColor
    {
        SpecialColorSelected = 0,
        SpecialColorNull,

        NumSpecialColors
    };

    ColorMap();
    virtual ~ColorMap();

    static QColor sample(double t, ColorScale scale);
    static std::vector<QColor> sample(ColorScale scale, 
                                      size_t n);
    static std::vector<QColor> sample(const std::vector<QColor>& samples, 
                                      size_t n);
    static std::vector<QColor> sample(const QColor& color_min, 
                                      const QColor& color_max, 
                                      size_t n);
    static std::vector<QColor> sample(const QColor& color_min, 
                                      const QColor& color_mid, 
                                      const QColor& color_max,
                                      size_t n);

    static void generateScaleImage(QImage& img, 
                                   ColorScale scale, 
                                   int w, 
                                   int h);
    static QComboBox* generateScaleSelection(ColorScale scale_default = ColorScale::Green2Red);

    size_t numColors() const;
    const QColor& getColor(size_t idx) const;

    void create(const std::vector<QColor>& colors,
                Type type = Type::Linear);
    void create(const QColor& color_min,
                const QColor& color_max,
                size_t n,
                Type type = Type::Linear);
    void create(const QColor& color_min,
                const QColor& color_mid,
                const QColor& color_max,
                size_t n,
                Type type = Type::Linear);
    void create(ColorScale scale, 
                size_t n,
                Type type = Type::Linear);

    void setSpecialColor(SpecialColor type, const QColor& color);
    const QColor& specialColor(SpecialColor type) const;
    
    QColor sample(double t) const;
    QColor sampleValue(double v, 
                       double vmin, 
                       double vmax) const;
    QColor sampleValueSymm(double v, 
                           double vmax) const;

    std::vector<std::pair<QColor, std::string>> getDescription(double vmin, 
                                                               double vmax) const;
    
    template<typename T>
    double value2Double(const T& v)
    {
        return static_cast<double>(v);
    }

    template<typename T>
    T double2Value(double v)
    {
        return static_cast<T>(v);
    }

    template<typename T>
    double factorFromValue(const T& v, const T& vmin, const T& vmax)
    {
        const double vrange = value2Double<T>(vmax - vmin);
        if (vrange < 1e-12)
            return 0.0;

        return value2Double<T>(v - vmin) / vrange;
    }

    template<typename T>
    T valueFromFactor(double f, const T& vmin, const T& vmax)
    {
        const double v_dbl = value2Double<T>(vmin) + f * value2Double<T>(vmax - vmin);
        return double2Value<T>(v_dbl);
    }

private:
    size_t indexFromFactor(double t) const;

    Type type_ = Type::Linear;

    std::vector<QColor> colors_;
    std::vector<double> value_factors_;
    size_t              n_colors_ = 0;

    std::vector<QColor> special_colors_;
};
