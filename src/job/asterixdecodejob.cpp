#include "asterixdecodejob.h"
#include "asteriximportertask.h"
#include "logger.h"

#include <jasterix/jasterix.h>

using namespace nlohmann;

ASTERIXDecodeJob::ASTERIXDecodeJob(ASTERIXImporterTask& task, const std::string& filename, const std::string& framing,
                                   bool test)
    : Job ("ASTERIXDecodeJob"), task_(task), filename_(filename), framing_(framing), test_(test)
{

}

void ASTERIXDecodeJob::run ()
{
    loginf << "ASTERIXDecodeJob: run";

    started_ = true;

    using namespace std::placeholders;
    std::function<void(nlohmann::json&, size_t, size_t)> callback = std::bind(&ASTERIXDecodeJob::jasterix_callback,
                                                                              this, _1, _2, _3);

    if (framing_ == "")
        task_.jASTERIX()->decodeFile (filename_, callback);
    else
        task_.jASTERIX()->decodeFile (filename_, framing_, callback);

    done_ = true;

    loginf << "ASTERIXDecodeJob: run: done";
}

void ASTERIXDecodeJob::jasterix_callback(nlohmann::json& data, size_t num_frames, size_t num_records)
{
    num_frames_ = num_frames;
    num_records_ = num_records;

    std::vector <json> extracted_records;

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
                {
                    record["category"] = category;

                    extracted_records.push_back(std::move(record));
                    category_counts_.at(category) += 1;
                }
            }
        }
    }

    //loginf << "ASTERIXDecodeJob: jasterix_callback: got " << extracted_records.size() << " records";

    task_.addDecodedASTERIX(extracted_records);
}


size_t ASTERIXDecodeJob::numFrames() const
{
    return num_frames_;
}

size_t ASTERIXDecodeJob::numRecords() const
{
    return num_records_;
}

std::map<unsigned int, size_t> ASTERIXDecodeJob::categoryCounts() const
{
    return category_counts_;
}

