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

class TaskResult;

namespace ResultReport
{

class Section;
class SectionContent;

class ReportExporter;

/**
 */
class ReportExport
{
public:
    ReportExport();
    virtual ~ReportExport();

    Result exportReport(TaskResult& result,
                        ReportExportMode mode,
                        const std::string& fn,
                        const std::string& resource_dir);

    ReportExportSettings& settings() { return settings_; }
    const ReportExportSettings& settings() const { return settings_; }

private:
    std::unique_ptr<ReportExporter> createExporter(ReportExportMode mode,
                                                   const std::string& fn,
                                                   const std::string& resource_dir) const;
    ReportExportSettings settings_;
};

} // namespace ResultReport
