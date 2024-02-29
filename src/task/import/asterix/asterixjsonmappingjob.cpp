#include "asterixjsonmappingjob.h"
#include "asterixjsonparser.h"
#include "buffer.h"
//#include "dbcontent/dbcontent.h"
#include "json.h"
//#include "jsonobjectparser.h"
#include "logger.h"

#include <exception>

using namespace std;
using namespace Utils;
using namespace nlohmann;

ASTERIXJSONMappingJob::ASTERIXJSONMappingJob(std::vector<std::unique_ptr<nlohmann::json>> data,
                                             const std::vector<std::string>& data_record_keys,
                                             const std::map<unsigned int, std::unique_ptr<ASTERIXJSONParser>>& parsers)
    : Job("ASTERIXJSONMappingJob"),
      data_(std::move(data)),
      data_record_keys_(data_record_keys),
      parsers_(parsers)
{
    logdbg << "ASTERIXJSONMappingJob: ctor";
}

ASTERIXJSONMappingJob::~ASTERIXJSONMappingJob()
{
    logdbg << "ASTERIXJSONMappingJob: dtor";
}

void ASTERIXJSONMappingJob::run()
{
    logdbg << "ASTERIXJSONMappingJob: run";

    started_ = true;

    string dbcontent_name;

    for (auto& parser_it : parsers_)
    {
        dbcontent_name = parser_it.second->dbContentName();

        if (!buffers_.count(dbcontent_name))
            buffers_[dbcontent_name] = parser_it.second->getNewBuffer();
        else
            parser_it.second->appendVariablesToBuffer(*buffers_.at(dbcontent_name));
    }

    auto process_lambda = [this](nlohmann::json& record) {
        //loginf << "UGA '" << record.dump(4) << "'";

        if (this->obsolete_)
            return;

        unsigned int category{0};

        if (!record.contains("category"))
        {
            logerr << "ASTERIXJSONMappingJob: run: record without category '" << record.dump(4) << "', skipping";
            return;
        }

        assert (record.contains("category"));

        category = record.at("category");

        bool parsed{false};
        bool parsed_any{false};

        if (!parsers_.count(category))
            return;

        const unique_ptr<ASTERIXJSONParser>& parser = parsers_.at(category);

        string dbcontent_name = parser->dbContentName();

        logdbg << "ASTERIXJSONMappingJob: run: mapping json: cat " << category;

        std::shared_ptr<Buffer>& buffer = buffers_.at(dbcontent_name);
        assert(buffer);

        try
        {
            logdbg << "ASTERIXJSONMappingJob: run: obj " << dbcontent_name << " parsing JSON";

            parsed = parser->parseJSON(record, *buffer);

            logdbg << "ASTERIXJSONMappingJob: run: obj " << dbcontent_name << " done";

            parsed_any |= parsed;
        }
        catch (exception& e)
        {
            logerr << "ASTERIXJSONMappingJob: run: caught exception '" << e.what() << "' in \n'"
                       << record.dump(4) << "' parser dbo " << dbcontent_name;

            ++num_errors_;

            return;
        }

        if (parsed_any)
        {
            category_mapped_counts_[category].first += 1;
            ++num_mapped_;
        }
        else
        {
            category_mapped_counts_[category].second += 1;
            ++num_not_mapped_;
        }
    };

    for (auto& data_slice : data_)
    {
        if (data_slice)
        {
            logdbg << "ASTERIXJSONMappingJob: run: applying JSON function";
            JSON::applyFunctionToValues(*data_slice.get(), data_record_keys_, data_record_keys_.begin(),
                                        process_lambda, false);
        }
    }

    std::map<std::string, std::shared_ptr<Buffer>> not_empty_buffers;

    logdbg << "ASTERIXJSONMappingJob: run: counting buffer sizes";
    for (auto& buf_it : buffers_)
    {
        if (buf_it.second && buf_it.second->size())
        {
            num_created_ += buf_it.second->size();
            not_empty_buffers[buf_it.first] = buf_it.second;
        }
    }
    buffers_ = not_empty_buffers;  // cleaner

    data_.clear();

    done_ = true;

    logdbg << "ASTERIXJSONMappingJob: run: done: mapped " << num_created_ << " skipped "
           << num_not_mapped_;
}

size_t ASTERIXJSONMappingJob::numMapped() const { return num_mapped_; }

size_t ASTERIXJSONMappingJob::numNotMapped() const { return num_not_mapped_; }

size_t ASTERIXJSONMappingJob::numCreated() const { return num_created_; }

std::map<unsigned int, std::pair<size_t, size_t>> ASTERIXJSONMappingJob::categoryMappedCounts() const
{
    return category_mapped_counts_;
}

size_t ASTERIXJSONMappingJob::numErrors() const
{
    return num_errors_;
}
