#include "asterixextractrecordsjob.h"

#include "logger.h"

using namespace nlohmann;

ASTERIXExtractRecordsJob::ASTERIXExtractRecordsJob(const std::string& framing, std::shared_ptr<nlohmann::json> data)
    : Job ("ASTERIXExtractRecordsJob"), framing_(framing), data_(data)
{

}

void ASTERIXExtractRecordsJob::run ()
{
    logdbg << "ASTERIXExtractRecordsJob: run";

    started_ = true;

    unsigned int category;

    if (framing_ == "")
    {
        assert (data_->find("data_blocks") != data_->end());

        for (json& data_block : data_->at("data_blocks"))
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
        assert (data_->find("frames") != data_->end());
        assert (data_->at("frames").is_array());

        for (json& frame : data_->at("frames"))
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

//    for (auto& cat_cnt_it : category_counts_)
//    {
//        loginf <<  "CAT"+std::to_string(cat_cnt_it.first)+": "+std::to_string(cat_cnt_it.second)+"\n";
//    }

    done_ = true;

    logdbg << "ASTERIXExtractRecordsJob: run: done";
}

void ASTERIXExtractRecordsJob::processRecord (unsigned int category, nlohmann::json& record)
{
    record["category"] = category;

    if (record.find("010") != record.end())
    {
        unsigned int sac = record.at("010").at("SAC");
        unsigned int sic = record.at("010").at("SIC");
        record["ds_id"] =  sac*255 + sic;
    }

    if (category == 1) // CAT001 coversion hack
    {
        if (record.find("090") != record.end())
            if (record.at("090").find("Flight Level") != record.at("090").end())
            {
                double flight_level = record.at("090").at("Flight Level"); // is mapped in ft
                record.at("090").at("Flight Level") = flight_level* 1e-2;  // ft to fl
            }
    }

    extracted_records_.push_back(std::move(record));
    category_counts_.at(category) += 1;
}

std::vector<nlohmann::json>& ASTERIXExtractRecordsJob::extractedRecords()
{
    return extracted_records_;
}

std::map<unsigned int, size_t> ASTERIXExtractRecordsJob::categoryCounts() const
{
    return category_counts_;
}
