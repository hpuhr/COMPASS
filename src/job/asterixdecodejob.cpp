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
#include "stringconv.h"
#include "logger.h"

#include <jasterix/jasterix.h>

#include <memory>

#include <QThread>

using namespace nlohmann;
using namespace Utils;

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
    std::function<void(nlohmann::json&, size_t, size_t, size_t)> callback =
            std::bind(&ASTERIXDecodeJob::jasterix_callback, this, _1, _2, _3, _4);
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

void ASTERIXDecodeJob::jasterix_callback(nlohmann::json& data, size_t num_frames, size_t num_records, size_t num_errors)
{
    if (error_)
        return;

    assert (!extracted_records_);
    extracted_records_ = std::make_shared<std::vector <nlohmann::json>> ();
    //extracted_records_.reset(new std::vector <nlohmann::json>());

    num_frames_ = num_frames;
    num_records_ = num_records;
    num_errors_ = num_errors;

    loginf << "ASTERIXDecodeJob: jasterix_callback: num errors " << num_errors_;

    unsigned int category;

    if (framing_ == "")
    {
        assert (data.find("data_blocks") != data.end());

        for (json& data_block : data.at("data_blocks"))
        {
            category = data_block.at("category");

            assert (data_block.find("content") != data_block.end());

            if (data_block.at("content").find("records") != data_block.at("content").end())
            {
                if (category_counts_.count(category) == 0)
                    category_counts_[category] = 0;

                for (json& record : data_block.at("content").at("records"))
                    processRecord (category, record);
            }
        }
    }
    else
    {
        assert (data.find("frames") != data.end());
        assert (data.at("frames").is_array());

        for (json& frame : data.at("frames"))
        {
            assert (frame.find("content") != frame.end());
            assert (frame.at("content").is_object());
            assert (frame.at("content").find("data_blocks") != frame.at("content").end());
            assert (frame.at("content").at("data_blocks").is_array());

            for (json& data_block : frame.at("content").at("data_blocks"))
            {
                category = data_block.at("category");

                assert (data_block.find("content") != data_block.end());

                if (data_block.at("content").find("records") != data_block.at("content").end())
                {
                    if (category_counts_.count(category) == 0)
                        category_counts_[category] = 0;

                    for (json& record : data_block.at("content").at("records"))
                        processRecord (category, record);
                }
            }
        }
    }

    while (pause_) // block decoder until unpaused
    {
        QThread::msleep(1);
    }

    emit decodedASTERIXSignal(extracted_records_);
    extracted_records_ = nullptr;

    data.clear();
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

void ASTERIXDecodeJob::processRecord (unsigned int category, nlohmann::json& record)
{
    record["category"] = category;

    int sac {-1};
    int sic {-1};

    if (record.find("010") != record.end())
    {
        sac = record.at("010").at("SAC");
        sic = record.at("010").at("SIC");
        record["ds_id"] =  sac*256 + sic;
    }

    if (category == 1) // CAT001 coversion hack
    {
        if (record.find("090") != record.end())
            if (record.at("090").find("Flight Level") != record.at("090").end())
            {
                double flight_level = record.at("090").at("Flight Level"); // is mapped in ft
                record.at("090").at("Flight Level") = flight_level* 1e-2;  // ft to fl
            }

        // "141":  "Truncated Time of Day": 221.4296875 mapped to "140.Time-of-Day"

        if (record.find("141") != record.end()
                && record.at("141").find("Truncated Time of Day") != record.at("141").end())
        {
            if (sac > -1 && sic > -1 ) // bingo
            {
                std::pair<unsigned int, unsigned int> sac_sic ({sac, sic});

                if (cat002_last_tod_period_.count(sac_sic) > 0)
                {
                    double tod = record.at("141").at("Truncated Time of Day");
                    //double tod = record.at("140").at("Time-of-Day");
                    tod += cat002_last_tod_period_.at(sac_sic);

//                    loginf << "corrected " << String::timeStringFromDouble(record.at("140").at("Time-of-Day"))
//                           << " to " << String::timeStringFromDouble(tod)
//                           << " last update " << cat002_last_tod_period_.at(sac_sic);

                    record["140"]["Time-of-Day"] = tod;
                }
                else
                {
                    loginf << "ASTERIXDecodeJob: processRecord: removing truncated tod "
                           << String::timeStringFromDouble(record.at("141").at("Truncated Time of Day"))
                           << " since to CAT002 from sensor "<< sac << "/" << sic << " is not present";
                    record["140"]["Time-of-Day"] = nullptr;
                }

//                loginf << "UGA " << String::timeStringFromDouble(record.at("140").at("Time-of-Day"))
//                       << " sac " << sac << " sic " << sic << " cnt " << cat002_last_tod_period_.count(sac_sic);

//                assert (record.at("140").at("Time-of-Day") > 3600.0);
            }
            else
            {
                loginf << "ASTERIXDecodeJob: processRecord: skipping cat001 report without sac/sic";
                record["140"]["Time-of-Day"] = nullptr;
            }
        }
        else
        {
            if (sac > -1 && sic > -1 )
            {
                std::pair<unsigned int, unsigned int> sac_sic ({sac, sic});

                if (cat002_last_tod_.count(sac_sic) > 0)
                {
                    record["140"]["Time-of-Day"] = cat002_last_tod_.at(sac_sic); // set tod, better than nothing
                }
                else
                    logdbg << "ASTERIXDecodeJob: processRecord: skipping cat001 report without truncated time of day"
                           << " or last cat002 time";
            }
            else
                logdbg << "ASTERIXDecodeJob: processRecord: skipping cat001 report without truncated time of day"
                       << " or sac/sic";
        }
    }
    else if (category == 2) // save last tods
    {
        //"030": "Time of Day": 33501.4140625

        if (record.find("030") != record.end())
        {
            if (sac > -1 && sic > -1) // bingo
            {
                //std::pair<unsigned int, unsigned int> sac_sic ({sac, sic});
                double cat002_last_tod = record.at("030").at("Time of Day");
                double cat002_last_tod_period = 512.0 * ((int)(cat002_last_tod / 512));
                cat002_last_tod_period_ [std::make_pair(sac, sic)] = cat002_last_tod_period;
                cat002_last_tod_ [std::make_pair(sac, sic)] = cat002_last_tod;
            }
        }
    }
    else if (category == 21)
    {
        //"030": "Time of Day": 33501.4140625

        if (record.find("150") != record.end()) // true airspeed
        {
            json& air_speed_item = record.at("150");
            assert (air_speed_item.find("IM") != air_speed_item.end());
            assert (air_speed_item.find("Air Speed") != air_speed_item.end());

            bool mach = air_speed_item.at("IM") == 1;
            double airspeed = air_speed_item.at("Air Speed");

            if (mach)
            {
                air_speed_item["Air Speed [knots]"] = airspeed*666.739;
                air_speed_item["Air Speed [mach]"] = airspeed;
            }
            else
            {
                air_speed_item["Air Speed [knots]"] = airspeed;
                air_speed_item["Air Speed [mach]"] = airspeed/666.739;
            }
        }
        else if (record.find("150") != record.end())
        {
            json& ground_speed_item = record.at("160");
            assert (ground_speed_item.find("IM") != ground_speed_item.end());
            assert (ground_speed_item.find("Air Speed") != ground_speed_item.end());

            double ground_speed = ground_speed_item.at("Ground Speed");
            ground_speed_item.at("Ground Speed") = ground_speed * 3600;
        }
    }

    extracted_records_->push_back(std::move(record));
    category_counts_.at(category) += 1;
}

std::map<unsigned int, size_t> ASTERIXDecodeJob::categoryCounts() const
{
    return category_counts_;
}

size_t ASTERIXDecodeJob::numErrors() const
{
    return num_errors_;
}

std::string ASTERIXDecodeJob::errorMessage() const
{
    return error_message_;
}



