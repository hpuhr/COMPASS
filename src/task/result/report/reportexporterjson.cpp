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

#include "task/result/report/reportexporterjson.h"
#include "task/result/report/section.h"
#include "task/result/report/sectioncontent.h"
#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/sectioncontenttable.h"
#include "task/result/report/sectioncontenttext.h"
#include "task/result/taskresult.h"

#include "files.h"

#include <fstream>

namespace ResultReport
{

/**
 */
ReportExporterJSON::ReportExporterJSON(const ReportExport* report_export,
                                       const std::string& export_fn,
                                       const std::string& export_resource_dir,
                                       bool write_files)
:   ReportExporter(report_export, export_fn, export_resource_dir)
,   write_files_  (write_files)
{
}

/**
 */
ReportExporterJSON::~ReportExporterJSON() = default;

/**
 */
Result ReportExporterJSON::initExport_impl(TaskResult& result)
{
    return Result::succeeded();
}

/**
 */
ResultT<nlohmann::json> ReportExporterJSON::finalizeExport_impl(TaskResult& result)
{
    if (!write_files_)
        return ResultT<nlohmann::json>::succeeded(json_data_);

    setStatus("Writing report file");

    std::ofstream file(exportFilename());
    if (!file.is_open())
        return ResultT<nlohmann::json>::failed("Could not create file '" + exportFilename() + "'");

    file << json_data_.dump(4);

    if (!file)
        return ResultT<nlohmann::json>::failed("Could not export report to '" + exportFilename() + "'");

    file.close();

    return ResultT<nlohmann::json>::succeeded(nlohmann::json());
}

/**
 */
Result ReportExporterJSON::exportSection_impl(Section& section)
{
    auto res_dir     = exportResourceDir();
    auto res_dir_ptr = exportCreatesResources() ? &res_dir : nullptr;

    //generate section json
    auto res_json = section.toJSONDocument(res_dir_ptr);
    if (!res_json.ok())
        return res_json;

    assert(res_json.result().is_object());

    nlohmann::json* jptr = nullptr;

    auto parent = section.parentSection();
    if (parent)
    {
        //section obtains parent => try to find json section of parent
        auto it = json_sections_.find(parent);
        if (it == json_sections_.end())
            return Result::failed("Section '" + section.name() + "' obtains no parent section in report");

        assert(it->second);
        assert(it->second->is_object());

        auto& parent_json = *it->second;

        assert(parent_json.contains(Section::FieldSubSections));
        auto& subsections = parent_json[ Section::FieldSubSections ];
        assert(subsections.is_array());

        //add to parents subsections
        subsections.push_back(res_json.result());

        jptr = &subsections.back();
    }
    else
    {
        //root section => init json data
        json_data_ = res_json.result();

        jptr = &json_data_;
    }

    assert(jptr);

    //remember my section's json object
    json_sections_[ &section ] = jptr;

    return Result::succeeded();
}

/**
 */
Result ReportExporterJSON::exportContentToJSON(SectionContent& content)
{
    //loginf << "ReportExporterJSON: exportContentToJSON";

    auto res_dir     = exportResourceDir();
    auto res_dir_ptr = exportCreatesResources() ? &res_dir : nullptr;

    //generate content json
    auto res_json = content.toJSONDocument(res_dir_ptr);
    if (!res_json.ok())
        return res_json;

    //contents might yield a single or multiple json content objects
    assert(res_json.result().is_object() || res_json.result().is_array());

    auto parent = content.parentSection();
    assert(parent);

    // try to find json section of parent
    auto it = json_sections_.find(parent);
    if (it == json_sections_.end())
        return Result::failed("Content '" + content.name() + "' obtains no parent section in report");

    assert(it->second);
    assert(it->second->is_object());

    auto& parent_json = *it->second;

    assert(parent_json.contains(Section::FieldDocContents));
    auto& contents = parent_json[ Section::FieldDocContents ];
    assert(contents.is_array());

    //add to parent section's contents
    if (res_json.result().is_object())
    {
        //loginf << "Adding content to parent " << parent->name();

        //add single json content
        contents.push_back(res_json.result());
    }
    else // array
    {
        //loginf << "Adding " << res_json.result().size() << " content(s) to parent " << parent->name();

        //add multiple json contents
        for (const auto& it : res_json.result())
        {
            assert(it.is_object());
            contents.push_back(it);
        }
    }

    return Result::succeeded();
}

/**
 */
Result ReportExporterJSON::exportFigure_impl(SectionContentFigure& figure)
{
    return exportContentToJSON(figure);
}

/**
 */
Result ReportExporterJSON::exportTable_impl(SectionContentTable& table)
{
    return exportContentToJSON(table);
}

/**
 */
Result ReportExporterJSON::exportText_impl(SectionContentText& text)
{
    return exportContentToJSON(text);
}

}
