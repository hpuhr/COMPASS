#include "jsonmappingstubsjob.h"
#include "jsonobjectparser.h"
#include "buffer.h"
#include "dbobject.h"

JSONMappingStubsJob::JSONMappingStubsJob(std::shared_ptr<std::vector<nlohmann::json>> extracted_records,
                               std::map <std::string, JSONObjectParser>& parsers)
    : Job ("JSONMappingStubsJob"), extracted_records_(extracted_records), parsers_(parsers)
{

}

JSONMappingStubsJob::~JSONMappingStubsJob()
{

}

void JSONMappingStubsJob::run ()
{
    logdbg << "JSONMappingStubsJob: run";

    started_ = true;

    logdbg << "JSONMappingStubsJob: run: mapping json";
    for (auto& j_it : *extracted_records_)
    {
        //loginf << "UGA '" << j_it.dump(4) << "'";

//        has_cat = j_it.find("category") != j_it.end();

//        if (has_cat)
//            category = j_it.at("category");

//        if (j_it.find("010") != j_it.end())
//        {
//            has_sac_sic = true;
//            sac = j_it.at("010").at("SAC");
//            sic = j_it.at("010").at("SIC");
//        }

        for (auto& map_it : parsers_)
        {
            logdbg << "JSONMappingStubsJob: run: mapping json: obj " << map_it.second.dbObject().name();
            map_it.second.createMappingStubs(j_it);
        }
    }

    done_ = true;
    logdbg << "JSONMappingStubsJob: run: done";
}

