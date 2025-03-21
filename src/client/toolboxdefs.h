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

#include <utility>

namespace toolbox
{

enum class ScreenRatio
{
    Ratio_Half = 0,
    Ratio_Third,
    Ratio_Quarter,
    Ratio_TwoThirds,
    Ratio_ThreeQuarter
};

inline std::pair<int, int> toParts(ScreenRatio screen_ratio) 
{
    switch(screen_ratio)
    {
        case ScreenRatio::Ratio_Half:
            return std::pair<int, int>(1, 1);
        case ScreenRatio::Ratio_Third:
            return std::pair<int, int>(1, 2);
        case ScreenRatio::Ratio_Quarter:
            return std::pair<int, int>(1, 3);
        case ScreenRatio::Ratio_TwoThirds:
            return std::pair<int, int>(2, 1);
        case ScreenRatio::Ratio_ThreeQuarter:
            return std::pair<int, int>(3, 1);
    }
    return std::pair<int, int>(1, 1);
}

}
