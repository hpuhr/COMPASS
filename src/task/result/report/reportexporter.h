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

#include "json.hpp"

class TaskResult;

namespace ResultReport
{

class Section;
class SectionContent;

class ReportExport;

/**
 */
class ReportExporter
{
public:
    ReportExporter(ReportExport* report_export,
                   const std::string& export_fn,
                   const std::string& export_temp_dir);
    virtual ~ReportExporter();

    ResultT<nlohmann::json> exportReport(TaskResult& result,
                                         const std::string& section = "",
                                         const std::string& content = "");

    virtual ReportExportMode exportMode() const = 0;
    virtual ReportExportFlag exportFlag() const = 0;

protected:
    virtual ResultT<nlohmann::json> exportReport_impl(TaskResult& result,
                                                      Section* section,
                                                      const boost::optional<unsigned int>& content_id);

    virtual Result exportSection_impl(const Section& section) const = 0;
    virtual Result exportContent_impl(const SectionContent& content) const = 0;

    virtual bool exportCreatesFile() const { return false; }
    virtual bool exportCreatesTempFiles() const { return false; }
    virtual bool exportCreatesInMemoryData() const { return false; } 

private:
    Result visitSection(const Section& section) const;
    Result visitContent(const SectionContent& content) const;

    ReportExport* report_export_ = nullptr;
    std::string   export_fn_;
    std::string   export_temp_dir_;
};

} // namespace ResultReport
