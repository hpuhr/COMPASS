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

#include "task/result/report/reportexporter.h"
#include "task/result/report/section.h"
#include "task/result/report/sectioncontent.h"
#include "task/result/taskresult.h"

#include "files.h"

namespace ResultReport
{

/**
 */
ReportExporter::ReportExporter(ReportExport* report_export,
                               const std::string& export_fn,
                               const std::string& export_temp_dir)
:   report_export_  (report_export  )
,   export_fn_      (export_fn      )
,   export_temp_dir_(export_temp_dir)
{
    assert(report_export_);
}

/**
 */
ReportExporter::~ReportExporter()
{
}

/**
 */
ResultT<nlohmann::json> ReportExporter::exportReport(TaskResult& result,
                                                     const std::string& section,
                                                     const std::string& content)
{
    //check stuff
    if (exportCreatesFile() && export_fn_.empty())
        return Result::failed("Filename not provided");
    if (exportCreatesTempFiles() && export_temp_dir_.empty())
        return Result::failed("Temporary directory not provided");
    if (exportCreatesTempFiles() && !Utils::Files::createMissingDirectories(export_temp_dir_))
        return Result::failed("Temporary directory could not be created");

    Section*                      section_ptr = nullptr;
    boost::optional<unsigned int> content_id;

    //retrieve initial section and content id
    if (!section.empty())
    {
        if (!result.report()->hasSection(section))
            return Result::failed("Section '" + section + "' not found in report");

        section_ptr = &result.report()->getSection(section);

        if (!content.empty())
        {
            if (!section_ptr->hasContent(content))
                return Result::failed("Content '" + content + "' not found in report section '" + section + "'");

            content_id = section_ptr->contentID(content);
        }
    }

    //export
    auto res = exportReport_impl(result, section_ptr, content_id);
    if (!res.ok())
        return res;

    //check on created in-mem data
    if (exportCreatesInMemoryData() && res.result().is_null())
        return Result::failed("Creating in-memory data failed");

    //check on created file
    if (exportCreatesFile() && !Utils::Files::fileExists(export_fn_))
        return Result::failed("Creating report file failed");

    return Result::succeeded();
}

/**
 */
ResultT<nlohmann::json> ReportExporter::exportReport_impl(TaskResult& result,
                                                          Section* section,
                                                          const boost::optional<unsigned int>& content_id)
{
    Section* start_section = section;
    if (!start_section)
        start_section = result.report()->rootSection().get();

    assert(start_section);

    if (content_id.has_value())
    {
        
    }

    return ResultT<nlohmann::json>::failed("");
}

/**
 */
Result ReportExporter::visitSection(const Section& section) const
{
    //@TODO: do stuff in base class?

    return exportSection_impl(section);
}

/**
 */
Result ReportExporter::visitContent(const SectionContent& content) const
{
    //@TODO: do stuff in base class?

    return exportContent_impl(content);
}

}
