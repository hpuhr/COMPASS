/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ASTERIXEXTRACTRECORDSJOB_H
#define ASTERIXEXTRACTRECORDSJOB_H

#include "job.h"
#include "json.hpp"

class ASTERIXExtractRecordsJob : public Job
{
public:
    ASTERIXExtractRecordsJob(const std::string& framing, std::shared_ptr<nlohmann::json> data);

    virtual void run ();

    std::vector<nlohmann::json>& extractedRecords(); // to be moved out
    std::map<unsigned int, size_t> categoryCounts() const;

private:
    std::string framing_;
    std::shared_ptr<nlohmann::json> data_;
    std::vector <nlohmann::json> extracted_records_;

    std::map<unsigned int, size_t> category_counts_;

    void processRecord (unsigned int category, nlohmann::json& record);
};

#endif // ASTERIXEXTRACTRECORDSJOB_H
