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

namespace
{
    /**
    */
    void interpolateColors(std::vector<QColor>& colors, 
                           const QColor& col0, 
                           const QColor& col1, 
                           size_t steps, 
                           size_t offs)
    {
        double dc = 1.0 / steps;

        for (size_t i = offs; i <= steps; ++i)
        {
            const double t  = i * dc;
            const double t0 = (1.0 - t);
            const double t1 = t;
            colors.push_back(QColor(col0.red()   * t0 + col1.red()   * t1,
                                    col0.green() * t0 + col1.green() * t1,
                                    col0.blue()  * t0 + col1.blue()  * t1));
        }
    };
}

/**
*/
ColorMap::ColorMap() = default;

/**
*/
ColorMap::~ColorMap() = default;

/**
*/
void ColorMap::set(const std::vector<QColor>& colors)
{
    colors_ = colors;
}

/**
*/
void ColorMap::set(const QColor& color_min, 
                   const QColor& color_max, 
                   size_t steps)
{
    colors_.clear();
    colors_.reserve(steps);

    interpolateColors(colors_, color_min, color_max, steps, 0);
}

/**
*/
void ColorMap::set(const QColor& color_min, 
                   const QColor& color_mid, 
                   const QColor& color_max, 
                   size_t steps)
{
    //make steps even and >= 2
    steps = std::max(steps % 2 == 0 ? steps : steps + 1, (size_t)2);

    colors_.clear();
    colors_.reserve(steps);

    const size_t steps_mid = steps / 2;

    interpolateColors(colors_, color_min, color_mid, steps_mid, 0);
    interpolateColors(colors_, color_mid, color_max, steps_mid, 1);
}

/**
*/
QColor ColorMap::sample(double t) const
{
    size_t idx = std::min(colors_.size() - 1, (size_t)(colors_.size() * std::min(1.0, std::max(0.0, t))));
    return colors_.at(idx);
}
