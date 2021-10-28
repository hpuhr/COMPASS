#include "dbcontentdatasourcebase.h"
#include "logger.h"

using namespace nlohmann;

namespace DBContent {

DataSourceBase::DataSourceBase()
{

}

std::string DataSourceBase::dbContentType() const
{
    return db_content_type_;
}

void DataSourceBase::dbContentType(const std::string &db_content_type)
{
    db_content_type_ = db_content_type;
}

unsigned int DataSourceBase::sac() const
{
    return sac_;
}

void DataSourceBase::sac(unsigned int sac)
{
    sac_ = sac;
}

unsigned int DataSourceBase::sic() const
{
    return sic_;
}

void DataSourceBase::sic(unsigned int sic)
{
    sic_ = sic;
}

std::string DataSourceBase::name() const
{
    return name_;
}

void DataSourceBase::name(const std::string &name)
{
    name_ = name;
}

bool DataSourceBase::hasShortName() const { return has_short_name_; }

void DataSourceBase::removeShortName()
{
    has_short_name_ = false;
    short_name_ = "";
}

void DataSourceBase::shortName(const std::string& short_name)
{
    loginf << "DataSourceBase " << name_ << ": shortName: " << short_name;
    has_short_name_ = true;
    this->short_name_ = short_name;
}

const std::string& DataSourceBase::shortName() const
{
    assert(has_short_name_);
    return short_name_;
}

nlohmann::json& DataSourceBase::info()
{
    return info_;
}




} // namespace DBContent
