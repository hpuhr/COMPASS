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

namespace ResultReport
{
    enum CellStyle
    {
        CellStyleTextBold        = 1 << 0,
        CellStyleTextItalic      = 1 << 1,

        CellStyleTextColorRed    = 1 << 10,
        CellStyleTextColorOrange = 1 << 11,
        CellStyleTextColorGreen  = 1 << 12,
        CellStyleTextColorGray   = 1 << 13,

        CellStyleBGColorRed      = 1 << 20,
        CellStyleBGColorOrange   = 1 << 21,
        CellStyleBGColorGreen    = 1 << 22,
        CellStyleBGColorGray     = 1 << 23
    };
}
