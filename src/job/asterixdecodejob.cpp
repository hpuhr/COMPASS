#include "asterixdecodejob.h"
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

#include "asteriximportertask.h"
#include "logger.h"

#include <jasterix/jasterix.h>

#include <QThread>

using namespace nlohmann;

ASTERIXDecodeJob::ASTERIXDecodeJob(ASTERIXImporterTask& task, const std::string& filename, const std::string& framing,
                                   bool test)
    : Job ("ASTERIXDecodeJob"), task_(task), filename_(filename), framing_(framing), test_(test)
{

}

void ASTERIXDecodeJob::run ()
{
    logdbg << "ASTERIXDecodeJob: run";

    started_ = true;

    using namespace std::placeholders;
    std::function<void(nlohmann::json&, size_t, size_t)> callback = std::bind(&ASTERIXDecodeJob::jasterix_callback,
                                                                              this, _1, _2, _3);
    try
    {
        if (framing_ == "")
            task_.jASTERIX()->decodeFile (filename_, callback);
        else
            task_.jASTERIX()->decodeFile (filename_, framing_, callback);
    }
    catch (std::exception& e)
    {
        logerr << "ASTERIXDecodeJob: run: decoding error '" << e.what() << "'";
        error_ = true;
        error_message_ = e.what();
    }

    done_ = true;

    loginf << "ASTERIXDecodeJob: run: done";
}

void ASTERIXDecodeJob::jasterix_callback(nlohmann::json& data, size_t num_frames, size_t num_records)
{
    if (error_)
        return;

    while (pause_) // block decoder until unpaused
    {
        QThread::sleep(1);
    }

    num_frames_ = num_frames;
    num_records_ = num_records;

    std::shared_ptr<json> moved_data {new json()};

    *moved_data = std::move(data);

    //loginf << "ASTERIXDecodeJob: jasterix_callback: got " << moved_data.size() << " records";

    emit decodedASTERIXSignal(moved_data);
}


size_t ASTERIXDecodeJob::numFrames() const
{
    return num_frames_;
}

size_t ASTERIXDecodeJob::numRecords() const
{
    return num_records_;
}

bool ASTERIXDecodeJob::error() const
{
    return error_;
}

std::string ASTERIXDecodeJob::errorMessage() const
{
    return error_message_;
}



