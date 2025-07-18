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

#include "reportdefs.h"

namespace ResultReport
{

const QColor Colors::TextRed    = QColor(220,20,60);
const QColor Colors::TextOrange = QColor(255,140,0);
const QColor Colors::TextGreen  = QColor(0,128,0);
const QColor Colors::TextGray   = Qt::darkGray;

const QColor Colors::BGRed    = QColor(240,128,128);
const QColor Colors::BGOrange = QColor(255,165,0);
const QColor Colors::BGGreen  = QColor(144,238,144);
const QColor Colors::BGGray   = Qt::lightGray;
const QColor Colors::BGYellow = QColor(255,255,153);

/**
 */
ReportExportMode reportExportModeFromString(const std::string& str)
{
    if (str == "JSON")
        return ReportExportMode::JSONFile;
    else if (str == "JSONBlob")
        return ReportExportMode::JSONBlob;
    else if (str == "Latex")
        return ReportExportMode::Latex;
    else if (str == "PDF")
        return ReportExportMode::LatexPDF;

    return ReportExportMode::LatexPDF;
}

/**
 */
std::string reportExportMode2String(ReportExportMode mode)
{
    switch(mode)
    {
        case ReportExportMode::JSONFile:
            return "JSON";
        case ReportExportMode::JSONBlob:
            return "JSONBlob";
        case ReportExportMode::Latex:
            return "Latex";
        case ReportExportMode::LatexPDF:
            return "PDF";
    }
    return "";
}

/**
 */
std::string reportExportMode2Extension(ReportExportMode mode)
{
    switch(mode)
    {
        case ReportExportMode::JSONFile:
        case ReportExportMode::JSONBlob:
            return ".json";
        case ReportExportMode::Latex:
            return ".tex";
        case ReportExportMode::LatexPDF:
            return ".pdf";
    }
    return "";
}

/**
 */
std::string reportExportMode2Folder(ReportExportMode mode)
{
    switch(mode)
    {
        case ReportExportMode::JSONFile:
        case ReportExportMode::JSONBlob:
            return "json";
        case ReportExportMode::Latex:
        case ReportExportMode::LatexPDF:
            return "tex";
    }
    return "";
}

}
