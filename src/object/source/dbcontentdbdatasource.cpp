#include "dbcontentdbdatasource.h"
#include "logger.h"

using namespace nlohmann;

namespace DBContent {

DBDataSource::DBDataSource()
{

}

DBDataSource::~DBDataSource()
{

}

nlohmann::json& DBDataSource::counts()
{
    return counts_;
}

json DBDataSource::getAsJSON()
{
    json j = DataSourceBase::getAsJSON();

    j["counts"] = counts_;

    return j;
}

void DBDataSource::setFromJSON(json& j)
{
    DataSourceBase::setFromJSON(j);

    counts_ = j.at("counts");
}

} // namespace DBContent
