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

#include <vector>
#include <map>

#include <QColor>

#include <boost/optional.hpp>

/**
*/
class ColorMap
{
public:
    ColorMap();
    virtual ~ColorMap();

    void set(const std::vector<QColor>& colors);
    void set(const QColor& color_min, 
             const QColor& color_max, 
             size_t steps);
    void set(const QColor& color_min, 
             const QColor& color_mid, 
             const QColor& color_max, 
             size_t steps);
    
    QColor sample(double t) const;

private:
    std::vector<QColor> colors_;
};
