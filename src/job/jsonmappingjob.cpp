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

#include "jsonmappingjob.h"

#include "buffer.h"
#include "dbcontent/dbcontent.h"
#include "json.h"
#include "jsonobjectparser.h"
#include "logger.h"

#include <exception>

using namespace std;
using namespace Utils;
using namespace nlohmann;

JSONMappingJob::JSONMappingJob(std::unique_ptr<nlohmann::json> data,
                               const std::vector<std::string>& data_record_keys,
                               const std::map<std::string, JSONObjectParser>& parsers)
    : Job("JSONMappingJob"),
      data_(std::move(data)),
      data_record_keys_(data_record_keys),
      parsers_(parsers)
{
    logdbg << "JSONMappingJob: ctor";
}

JSONMappingJob::~JSONMappingJob()
{
    loginf << "JSONMappingJob: dtor";
    assert (done_);
}

void JSONMappingJob::run()
{
    logdbg << "JSONMappingJob: run";

    started_ = true;

    for (auto& parser_it : parsers_)
    {
        if (!parser_it.second.active())
            continue;

        if (!buffers_.count(parser_it.second.dbObject().name()))
            buffers_[parser_it.second.dbObject().name()] = parser_it.second.getNewBuffer();
        else
            parser_it.second.appendVariablesToBuffer(
                *buffers_.at(parser_it.second.dbObject().name()));
    }

    auto process_lambda = [this](nlohmann::json& record) {
        //loginf << "UGA '" << record.dump(4) << "'";

        unsigned int category{0};
        bool has_cat = record.contains("category");

        if (has_cat)
            category = record.at("category");

        bool parsed{false};
        bool parsed_any{false};

        for (auto& map_it : parsers_)
        {
            if (!map_it.second.active())
                continue;

            logdbg << "JSONMappingJob: run: mapping json: obj " << map_it.second.dbObject().name();
            std::shared_ptr<Buffer>& buffer = buffers_.at(map_it.second.dbObject().name());
            assert(buffer);
            try
            {
                logdbg << "JSONMappingJob: run: obj " << map_it.second.dbObject().name() << " parsing JSON";

                parsed = map_it.second.parseJSON(record, *buffer);

                if (parsed)
                {
                    logdbg << "JSONMappingJob: run: obj " << map_it.second.dbObject().name() << " transforming buffer";
                    map_it.second.transformBuffer(*buffer, buffer->size() - 1);
                }

                logdbg << "JSONMappingJob: run: obj " << map_it.second.dbObject().name() << " done";

                parsed_any |= parsed;
            }
            catch (exception& e)
            {
                logerr << "JSONMappingJob: run: caught exception '" << e.what() << "' in \n'"
                       << record.dump(4) << "' parser " << map_it.second.dbObject().name();

                ++num_errors_;

                continue;
            }
        }

        if (parsed_any)
        {
            if (has_cat)
                category_mapped_counts_[category].first += 1;
            ++num_mapped_;
        }
        else
        {
            if (has_cat)
                category_mapped_counts_[category].second += 1;
            ++num_not_mapped_;
        }
    };

    if (obsolete_)
    {
        done_ = true;
        return;
    }
    assert(data_);
    logdbg << "JSONMappingJob: run: applying JSON function";
    JSON::applyFunctionToValues(*data_.get(), data_record_keys_, data_record_keys_.begin(),
                                process_lambda, false);

    if (obsolete_)
    {
        done_ = true;
        return;
    }

    std::map<std::string, std::shared_ptr<Buffer>> not_empty_buffers;

    logdbg << "JSONMappingJob: run: counting buffer sizes";
    for (auto& buf_it : buffers_)
    {
        if (buf_it.second && buf_it.second->size())
        {
            num_created_ += buf_it.second->size();
            not_empty_buffers[buf_it.first] = buf_it.second;
        }
    }
    buffers_ = not_empty_buffers;  // cleaner

    done_ = true;
    data_ = nullptr;

    logdbg << "JSONMappingJob: run: done: mapped " << num_created_ << " skipped "
           << num_not_mapped_;
}

size_t JSONMappingJob::numMapped() const { return num_mapped_; }

size_t JSONMappingJob::numNotMapped() const { return num_not_mapped_; }

size_t JSONMappingJob::numCreated() const { return num_created_; }

std::map<unsigned int, std::pair<size_t, size_t>> JSONMappingJob::categoryMappedCounts() const
{
    return category_mapped_counts_;
}

size_t JSONMappingJob::numErrors() const
{
    return num_errors_;
}
