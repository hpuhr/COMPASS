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
    logdbg << "JSONParseJob: run: start";

    started_ = true;
    for (auto& str_it : objects_)
    {
        json_objects_.push_back(json::parse(str_it));
        ++objects_parsed_;
    }

    logdbg << "JSONParseJob: run: done with " << objects_parsed_ << " objects";
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


