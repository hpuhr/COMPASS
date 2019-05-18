#ifndef ASTERIXEXTRACTRECORDSJOB_H
#define ASTERIXEXTRACTRECORDSJOB_H

#include "job.h"
#include "json.hpp"

class ASTERIXExtractRecordsJob : public Job
{
public:
    ASTERIXExtractRecordsJob(const std::string& framing, std::shared_ptr<nlohmann::json> data);

    virtual void run ();

    std::shared_ptr<std::vector<nlohmann::json>> extractedRecords() const;
    std::map<unsigned int, size_t> categoryCounts() const;

private:
    std::string framing_;
    std::shared_ptr<nlohmann::json> data_;
    std::shared_ptr<std::vector <nlohmann::json>> extracted_records_;

    std::map<unsigned int, size_t> category_counts_;
};

#endif // ASTERIXEXTRACTRECORDSJOB_H
