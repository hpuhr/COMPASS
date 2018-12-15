#include "jsonparsejob.h"
#include "logger.h"

using namespace nlohmann;

JSONParseJob::JSONParseJob(std::vector<std::string>&& objects)
    : Job ("JSONParseJob"), objects_(objects)
{

}

JSONParseJob::~JSONParseJob()
{

}

void JSONParseJob::run ()
{
    loginf << "JSONParseJob: run: start with " << objects_.size() << " objects";

    started_ = true;
    for (auto& str_it : objects_)
    {
        try
        {
            json_objects_.push_back(json::parse(str_it));
        }
        catch (nlohmann::detail::parse_error e)
        {
            logwrn << "JSONParseJob: run: parse error " << e.what() << " in '" << str_it << "'";
            continue;
        }
        ++objects_parsed_;
    }

    loginf << "JSONParseJob: run: done with " << objects_parsed_ << " objects";
    done_ = true;
}

size_t JSONParseJob::objectsParsed() const
{
    return objects_parsed_;
}

std::vector<nlohmann::json>& JSONParseJob::jsonObjects()
{
    return json_objects_;
}


