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
#include "task/result/report/reportexport.h"
#include "task/result/report/section.h"
#include "task/result/report/sectioncontent.h"
#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/sectioncontenttable.h"
#include "task/result/report/sectioncontenttext.h"

#include "task/result/taskresult.h"

#include "files.h"
#include "global.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include "geographicview.h"
#endif

namespace ResultReport
{

const int ReportExporter::TableMaxRows    = 0;
const int ReportExporter::TableMaxColumns = 0;

const std::string ReportExporter::ResourceFolderScreenshots = "screenshots";
const std::string ReportExporter::ResourceFolderTables      = "tables";

const std::string ReportExporter::ExportImageFormat = ".jpg";
const std::string ReportExporter::ExportTableFormat = ".json";
const std::string ReportExporter::ExportTextFormat  = ".txt";

/**
 */
ReportExporter::ReportExporter(const ReportExport* report_export,
                               const std::string& export_fn,
                               const std::string& export_resource_dir)
:   report_export_      (report_export      )
,   export_fn_          (export_fn          )
,   export_resource_dir_(export_resource_dir)
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
const ReportExportSettings& ReportExporter::settings() const
{
    return report_export_->settings();
}

/**
 */
std::string ReportExporter::resourceSubDir(ResourceDir dir)
{
    switch (dir)
    {
        case ResourceDir::Root:
            return "";
        case ResourceDir::Screenshots:
            return ResourceFolderScreenshots;
        case ResourceDir::Tables:
            return ResourceFolderTables;
    }

    return "";
}

/**
 */
Result ReportExporter::initExport(TaskResult& result)
{
    return initExport_impl(result);
}

/**
 */
ResultT<nlohmann::json> ReportExporter::finalizeExport(TaskResult& result)
{
    return finalizeExport_impl(result);
}

/**
 */
ResultT<nlohmann::json> ReportExporter::exportReport(TaskResult& result,
                                                     const std::string& section,
                                                     const std::string& content)
{
    loginf << "ReportExporter: exportReport: Exporting result '" << result.name() << "'";

    num_sections_exported_ = 0;
    done_                  = false;

    ResultT<nlohmann::json> res_final;

    try
    {
        //check stuff
        if (exportCreatesFile() && export_fn_.empty())
            return Result::failed("Filename not provided");
        if (exportCreatesResources() && export_resource_dir_.empty())
            return Result::failed("Resource directory not provided");
        if (exportCreatesResources() && Utils::Files::directoryExists(export_resource_dir_))
        {
            Utils::Files::deleteFolder(export_resource_dir_);
            if (Utils::Files::directoryExists(export_resource_dir_))
                return Result::failed("Existing report resources could not be removed");
        }
        if (exportCreatesResources() && !Utils::Files::createMissingDirectories(export_resource_dir_))
            return Result::failed("Resource directory could not be created");

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

        if (!section_ptr)
            section_ptr = result.report()->rootSection().get();

        //check if section is deactivated for export
        if (!section_ptr->exportEnabled(exportMode()))
            return Result::failed("Parent section '" + section_ptr->name() + "' is disabled for export");

        assert(section_ptr);

        //pre-compute total number of exported sections
        auto func = [ this ] (const Section& s)
        {
            return s.exportEnabled(this->exportMode());
        };
        num_sections_total_ = section_ptr->numSections(func);

        //for immediate rendering of geographic view during image generation
#if USE_EXPERIMENTAL_SOURCE == true
        GeographicView::instant_display_ = true;
#endif

        //export
        res_final = exportReport_impl(result, section_ptr, content_id);
        if (!res_final.ok())
            return res_final;

#if USE_EXPERIMENTAL_SOURCE == true
        GeographicView::instant_display_ = false;
#endif

        //check on created in-mem data
        if (exportCreatesInMemoryData() && res_final.result().is_null())
            return Result::failed("Creating in-memory data failed");

        //check on created file
        if (exportCreatesFile() && !Utils::Files::fileExists(export_fn_))
            return Result::failed("Creating report file failed");
    }
    catch (const std::exception& ex)
    {
        return Result::failed("Exporting report failed: " + std::string(ex.what())); 
    }

    loginf << "ReportExporter: exportReport: Exporting result '" << result.name() << "' succeeded";

    done_ = true;
    emit progressChanged();

    return res_final;
}

/**
 */
ResultT<nlohmann::json> ReportExporter::exportReport_impl(TaskResult& result,
                                                          Section* section,
                                                          const boost::optional<unsigned int>& content_id)
{
    assert(section);

    auto res = initExport(result);
    if (!res.ok())
        return res;

    loginf << "ReportExporter: exportReport_impl: start section = " << section->name();

    //visit start section
    res = visitSection(*section);
    if (!res.ok())
        return res;

    return finalizeExport(result);
}

/**
 */
Result ReportExporter::visitSection(Section& section)
{
    //skip section?
    if (!section.exportEnabled(exportMode()))
    {
        loginf << "ReportExporter: visitSection: Skipping, section '" << section.id() << "' disabled for export";
        return Result::succeeded();
    }

    //loginf << "Exporting section '" << section.id() << "'";

    setStatus("Exporting section '" + section.name() + "'");

    //export section first
    auto res = exportSection_impl(section);
    if (!res.ok())
        return res;

    current_content_section_ = &section;

    //then its contents
    auto contents = section.sectionContent();
    for (auto& c : contents)
    {
        if (!c)
            return Result::failed("Contents could not be loaded in section '" + section.name() + "'");

        res = visitContent(*c);
        if (!res.ok())
            return res;
    }

    current_content_section_ = nullptr;

    ++num_sections_exported_;
    emit progressChanged();

    //then its subsections
    for (const auto& sec_it : section.subSections(false))
    {
        res = visitSection(*sec_it);
        if (!res.ok())
            return res;
    }

    return Result::succeeded();
}

/**
 */
Result ReportExporter::visitContent(SectionContent& content)
{
    //skip content?
    if (!content.exportEnabled(exportMode()))
    {
        loginf << "ReportExporter: visitContent: Skipping, content '" << content.id() << "' disabled for export";
        return Result::succeeded();
    }

    if (content.isLocked())
    {
        loginf << "ReportExporter: visitContent: Skipping, content '" << content.id() << "' locked";
        return Result::succeeded();
    }

    //loginf << "Exporting content '" << content.id() << "'";

    //load content?
    if (content.isOnDemand() && !content.loadOnDemandIfNeeded())
        return Result::failed("Content '" + content.name() + "' could not be loaded on demand");

    //check which content
    if (content.contentType() == SectionContent::ContentType::Figure)
    {
        auto c = dynamic_cast<SectionContentFigure*>(&content);
        if (c) 
            return exportFigure(*c);
    }
    else if (content.contentType() == SectionContent::ContentType::Table)
    {
        auto c = dynamic_cast<SectionContentTable*>(&content);
        if (c) 
            return exportTable(*c);
    }
    else if (content.contentType() == SectionContent::ContentType::Text)
    {
        auto c = dynamic_cast<SectionContentText*>(&content);
        if (c) 
            return exportText(*c);
    }

    return Result::failed("Content '" + content.name() + "' is of illegal type");
}

/**
 */
Result ReportExporter::exportFigure(SectionContentFigure& figure)
{
    return exportFigure_impl(figure);
}

/**
 */
Result ReportExporter::exportTable(SectionContentTable& table)
{
    return exportTable_impl(table);
}

/**
 */
Result ReportExporter::exportText(SectionContentText& text)
{
    return exportText_impl(text);
}

/**
 */
void ReportExporter::setStatus(const std::string& status)
{
    status_ = status;

    emit progressChanged();
}

}
