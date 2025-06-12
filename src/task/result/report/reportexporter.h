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

#include "reportdefs.h"

#include "result.h"

#include <map>
#include <string>
#include <memory>

#include <boost/optional.hpp>

#include <QObject>

#include "json.hpp"

class TaskResult;

namespace ResultReport
{

class Section;
class SectionContent;
class SectionContentFigure;
class SectionContentTable;
class SectionContentText;

class ReportExport;

/**
 */
class ReportExporter : public QObject
{
    Q_OBJECT
public:
    ReportExporter(const ReportExport* report_export,
                   const std::string& export_fn,
                   const std::string& export_resource_dir,
                   bool interaction_mode);
    virtual ~ReportExporter();

    const ReportExportSettings& settings() const;

    ResultT<nlohmann::json> exportReport(TaskResult& result,
                                         const std::string& section = "",
                                         const std::string& content = "");

    virtual ReportExportMode exportMode() const = 0;

    size_t numSectionsTotal() const { return num_sections_total_; }
    size_t numSectionsExported() const { return num_sections_exported_; }
    const std::string& status() const { return status_; }
    bool isDone() const { return done_; }

    static const int TableMaxRows;
    static const int TableMaxColumns;

    static const std::string ResourceFolderScreenshots;
    static const std::string ResourceFolderTables;

    static const std::string ExportImageFormat;
    static const std::string ExportTableFormat;
    static const std::string ExportTextFormat;

    static std::string resourceSubDir(ResourceDir dir);

signals:
    void progressChanged();

protected:
    virtual ResultT<nlohmann::json> exportReport_impl(TaskResult& result,
                                                      Section* section,
                                                      const boost::optional<unsigned int>& content_id);
    virtual Result initExport_impl(TaskResult& result) = 0;
    virtual ResultT<nlohmann::json> finalizeExport_impl(TaskResult& result) = 0;

    virtual Result exportSection_impl(Section& section) = 0;
    virtual Result exportFigure_impl(SectionContentFigure& figure) = 0;
    virtual Result exportTable_impl(SectionContentTable& table) = 0;
    virtual Result exportText_impl(SectionContentText& text) = 0;

    virtual bool exportCreatesFile() const { return false; }
    virtual bool exportCreatesResources() const { return false; }
    virtual bool exportCreatesInMemoryData() const { return false; } 

    const std::string& exportFilename() const { return export_fn_; }
    const std::string& exportResourceDir() const { return export_resource_dir_; }
    bool hasInteraction() const { return interaction_mode_; }

    void setStatus(const std::string& status);

private:
    Result initExport(TaskResult& result);
    ResultT<nlohmann::json> finalizeExport(TaskResult& result);

    Result visitSection(Section& section);
    Result visitContent(SectionContent& content);

    Result exportFigure(SectionContentFigure& figure);
    Result exportTable(SectionContentTable& table);
    Result exportText(SectionContentText& text);

    const ReportExport* report_export_ = nullptr;
    std::string         export_fn_;
    std::string         export_resource_dir_;

    bool                interaction_mode_ = true;

    Section* current_content_section_ = nullptr;

    size_t      num_sections_total_    = 0;
    size_t      num_sections_exported_ = 0;
    bool        done_                  = false;
    std::string status_;
};

} // namespace ResultReport
