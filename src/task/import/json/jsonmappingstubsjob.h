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

#include <memory>
#include <vector>

#include "job.h"
#include "json_fwd.hpp"

class JSONObjectParser;
class Buffer;

class JSONMappingStubsJob : public Job
{
  public:
    JSONMappingStubsJob(std::unique_ptr<nlohmann::json> data,
                        const std::vector<std::string>& data_record_keys,
                        std::map<std::string, JSONObjectParser>& parsers);
    // json obj moved, mappings referenced
    virtual ~JSONMappingStubsJob();

  protected:
    void run_impl() override;

  private:
    std::unique_ptr<nlohmann::json> data_;
    const std::vector<std::string> data_record_keys_;
    std::map<std::string, JSONObjectParser>& parsers_;
};

