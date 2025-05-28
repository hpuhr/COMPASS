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

#include <limits>

#include <QColor>

namespace ResultReport
{
    enum CellStyle
    {
        CellStyleCheckable         = 1 << 0,  // cell obtains an icon coded into the cells bool value
        CellStyleIcon              = 1 << 1,  // cell is checkable, its check state coded into the cells string value

        CellStyleTextBold          = 1 << 5,  // cell obtains bold text
        CellStyleTextItalic        = 1 << 6,  // cell obtains italic text
        CellStyleTextStrikeOut     = 1 << 7,  // cell text is strike-out

        CellStyleTextColorRed      = 1 << 10,
        CellStyleTextColorOrange   = 1 << 11,
        CellStyleTextColorGreen    = 1 << 12,
        CellStyleTextColorGray     = 1 << 13,

        CellStyleBGColorRed        = 1 << 20,
        CellStyleBGColorOrange     = 1 << 21,
        CellStyleBGColorGreen      = 1 << 22,
        CellStyleBGColorGray       = 1 << 23,
        CellStyleBGColorYellow     = 1 << 24
    };

    /**
     */
    class Colors
    {
    public:
        static const QColor TextRed;
        static const QColor TextOrange;
        static const QColor TextGreen;
        static const QColor TextGray;

        static const QColor BGRed;
        static const QColor BGOrange;
        static const QColor BGGreen;
        static const QColor BGGray;
        static const QColor BGYellow;
    };

    enum class ReportExportMode
    {
        JSONFile = 0,
        JSONBlob,
        Latex,
        LatexPDF
    };

    enum class ResourceDir
    {
        Root = 0,
        Screenshots,
        Tables
    };

    struct ReportExportSettings
    {
        std::string author;
        std::string abstract;

        unsigned int latex_table_max_columns = 500;
        unsigned int latex_table_max_width   = 24;

        bool open_created_file = true;
    };
}
