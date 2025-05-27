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

#include "task/result/report/reportexport.h"
#include "task/result/report/reportexporter.h"
#include "task/result/report/reportexporterjson.h"
#include "task/result/report/reportexporterlatex.h"

namespace ResultReport
{

/**
 */
ReportExport::ReportExport()
{
}

/**
 */
ReportExport::~ReportExport()
{
}

/**
 */
Result ReportExport::exportReport(TaskResult& result,
                                  ReportExportMode mode,
                                  const std::string& fn,
                                  const std::string& resource_dir)
{
    //create exporter
    auto exporter = createExporter(mode, fn, resource_dir);
    if (!exporter)
        return Result::failed("Exporter could not be created for export type ''");

    //export using exporter
    return exporter->exportReport(result);
}

/**
 */
std::unique_ptr<ReportExporter> ReportExport::createExporter(ReportExportMode mode,
                                                             const std::string& fn,
                                                             const std::string& resource_dir) const
{
    if (mode == ReportExportMode::JSONFile)
    {
        return std::unique_ptr<ReportExporter>(new ReportExporterJSONFile(this, fn, resource_dir));
    }
    else if (mode == ReportExportMode::JSONBlob)
    {
        return std::unique_ptr<ReportExporter>(new ReportExporterJSONBlob(this, fn, resource_dir));
    }
    else if (mode == ReportExportMode::Latex)
    {
        return std::unique_ptr<ReportExporter>(new ReportExporterLatexFiles(this, fn, resource_dir));
    }
    else if (mode == ReportExportMode::LatexPDF)
    {
        return std::unique_ptr<ReportExporter>(new ReportExporterLatexPDF(this, fn, resource_dir));
    }

    return std::unique_ptr<ReportExporter>();
}

}
