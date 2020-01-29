#include "jsonmappingjob.h"
#include "jsonobjectparser.h"
#include "buffer.h"
#include "dbobject.h"
#include "logger.h"
#include "json.h"

using namespace Utils;
using namespace nlohmann;

JSONMappingJob::JSONMappingJob(std::unique_ptr<nlohmann::json> data,
                               const std::vector<std::string>& data_record_keys,
                               const std::map <std::string, JSONObjectParser>& parsers) // , size_t key_count
    : Job ("JSONMappingJob"), data_(std::move(data)), data_record_keys_(data_record_keys), parsers_(parsers)
      //key_count_(key_count)
{
    logdbg << "JSONMappingJob: ctor";
}

JSONMappingJob::~JSONMappingJob()
{
    logdbg << "JSONMappingJob: dtor";
}

void JSONMappingJob::run ()
{
    logdbg << "JSONMappingJob: run";

    started_ = true;

    for (auto& parser_it : parsers_)
    {
        if (!buffers_.count(parser_it.second.dbObject().name()))
            buffers_[parser_it.second.dbObject().name()] = parser_it.second.getNewBuffer();
        else
            parser_it.second.appendVariablesToBuffer(*buffers_.at(parser_it.second.dbObject().name()));
    }

//    bool parsed;
//    bool parsed_any = false;

//    bool has_cat;
    unsigned int category;

    auto process_lambda = [this, &category](nlohmann::json& record)
    {
        //loginf << "UGA '" << record.dump(4) << "'";

        bool has_cat = record.contains("category");

        if (has_cat)
            category = record.at("category");

        bool parsed {false};
        bool parsed_any {false};

        for (auto& map_it : parsers_)
        {
            logdbg << "JSONMappingJob: run: mapping json: obj " << map_it.second.dbObject().name();
            std::shared_ptr<Buffer>& buffer = buffers_.at(map_it.second.dbObject().name());
            assert (buffer);
            parsed = map_it.second.parseJSON(record, *buffer);

            if (parsed)
                map_it.second.transformBuffer(*buffer, buffer->size()-1);

            parsed_any |= parsed;
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

    assert (data_);
    //loginf << "JSONMappingJob: run: applying JSON function";
    JSON::applyFunctionToValues(*data_.get(), data_record_keys_, data_record_keys_.begin(), process_lambda, false);

//    assert (JSON::canFindKey(*data_.get(), data_record_keys_));
//    const nlohmann::json& extracted_records = JSON::findKey(*data_.get(), data_record_keys_);
//    assert (extracted_records.is_array());

//    logdbg << "JSONMappingJob: run: mapping json";
//    for (auto& j_it : extracted_records.get<json::array_t>())
//    {
//        //loginf << "UGA '" << j_it.dump(4) << "'";

//        has_cat = j_it.find("category") != j_it.end();

//        if (has_cat)
//            category = j_it.at("category");

//        parsed = false;
//        parsed_any = false;

//        for (auto& map_it : parsers_)
//        {
//            logdbg << "JSONMappingJob: run: mapping json: obj " << map_it.second.dbObject().name();
//            std::shared_ptr<Buffer>& buffer = buffers_.at(map_it.second.dbObject().name());
//            parsed = map_it.second.parseJSON(j_it, buffer);

//            if (parsed)
//                map_it.second.transformBuffer(buffer, buffer->size()-1);

//            parsed_any |= parsed;
//        }

//        if (parsed_any)
//        {
//            if (has_cat)
//                category_mapped_counts_[category].first += 1;
//            ++num_mapped_;
//        }
//        else
//        {
//            if (has_cat)
//                category_mapped_counts_[category].second += 1;
//            ++num_not_mapped_;
//        }
//    }

    logdbg << "JSONMappingJob: run: counting buffer sizes";
    for (auto& parser_it : parsers_)
    {
        assert (buffers_.count(parser_it.second.dbObject().name()));

        logdbg << "JSONMappingJob: run: creating buffer for " << parser_it.second.dbObject().name();

        std::shared_ptr<Buffer>& buffer = buffers_.at(parser_it.second.dbObject().name());

        if (buffer && buffer->size())
        {
            num_created_ += buffer->size();
        }
    }

    done_ = true;

    logdbg << "JSONMappingJob: run: done: mapped " << num_created_ << " skipped " << num_not_mapped_;
}

size_t JSONMappingJob::numMapped() const
{
    return num_mapped_;
}


size_t JSONMappingJob::numNotMapped() const
{
    return num_not_mapped_;
}

size_t JSONMappingJob::numCreated() const
{
    return num_created_;
}

std::map<unsigned int, std::pair<size_t, size_t> > JSONMappingJob::categoryMappedCounts() const
{
    return category_mapped_counts_;
}
