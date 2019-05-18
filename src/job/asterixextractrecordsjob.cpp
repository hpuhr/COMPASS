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

    extracted_records_.reset(new std::vector <json>());
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
                {
                    record["category"] = category;

                    extracted_records_->push_back(std::move(record));
                    category_counts_.at(category) += 1;
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

std::shared_ptr<std::vector<nlohmann::json> > ASTERIXExtractRecordsJob::extractedRecords() const
{
    return extracted_records_;
}

std::map<unsigned int, size_t> ASTERIXExtractRecordsJob::categoryCounts() const
{
    return category_counts_;
}
