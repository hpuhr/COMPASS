#include "jsonmappingstubsjob.h"
#include "jsonobjectparser.h"
#include "buffer.h"
#include "dbobject.h"
#include "json.h"

using namespace Utils;

JSONMappingStubsJob::JSONMappingStubsJob(std::unique_ptr<nlohmann::json> data,
                                         const std::vector<std::string>& data_record_keys,
                               std::map <std::string, JSONObjectParser>& parsers)
    : Job ("JSONMappingStubsJob"), data_(std::move(data)), data_record_keys_(data_record_keys), parsers_(parsers)
{

}

JSONMappingStubsJob::~JSONMappingStubsJob()
{

}

void JSONMappingStubsJob::run ()
{
    logdbg << "JSONMappingStubsJob: run";

    started_ = true;

    auto process_lambda = [this](nlohmann::json& record)
    {
        for (auto& map_it : parsers_)
        {
            logdbg << "JSONMappingStubsJob: run: mapping json: obj " << map_it.second.dbObject().name();
            map_it.second.createMappingStubs(record);
        }

    };

    logdbg << "JSONMappingStubsJob: run: mapping json";

    assert (data_);
    loginf << "JSONMappingStubsJob: run: applying JSON function";
    JSON::applyFunctionToValues(*data_.get(), data_record_keys_, data_record_keys_.begin(), process_lambda, false);

//    for (auto& j_it : *extracted_records_)
//    {
//        for (auto& map_it : parsers_)
//        {
//            logdbg << "JSONMappingStubsJob: run: mapping json: obj " << map_it.second.dbObject().name();
//            map_it.second.createMappingStubs(j_it);
//        }
//    }

    done_ = true;
    logdbg << "JSONMappingStubsJob: run: done";
}

