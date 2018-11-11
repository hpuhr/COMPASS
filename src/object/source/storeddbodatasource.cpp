#include "storeddbodatasource.h"

#include "dbobject.h"

StoredDBODataSource::StoredDBODataSource(const std::string& class_id, const std::string& instance_id, DBObject& object)
    : Configurable(class_id, instance_id, &object), object_(object)
{
    registerParameter ("id", &id_, 0);
    registerParameter ("name", &name_, "");
    registerParameter ("short_name", &short_name_,  "");
    registerParameter ("sac", &sac_, 0);
    registerParameter ("sic", &sic_, 0);
    registerParameter ("latitude", &latitude_, 0);
    registerParameter ("longitude", &longitude_, 0);
    registerParameter ("altitude", &altitude_, 0);
}

double StoredDBODataSource::altitude() const
{
    return altitude_;
}

unsigned int StoredDBODataSource::id() const
{
    return id_;
}

double StoredDBODataSource::latitude() const
{
    return latitude_;
}

double StoredDBODataSource::longitude() const
{
    return longitude_;
}

const std::string &StoredDBODataSource::name() const
{
    return name_;
}

unsigned char StoredDBODataSource::sac() const
{
    return sac_;
}

const std::string &StoredDBODataSource::shortName() const
{
    return short_name_;
}

unsigned char StoredDBODataSource::sic() const
{
    return sic_;
}

void StoredDBODataSource::altitude(double altitude)
{
    this->altitude_ = altitude;
}

void StoredDBODataSource::latitude(double latitiude)
{
    this->latitude_ = latitiude;
}

void StoredDBODataSource::longitude(double longitude)
{
    this->longitude_ = longitude;
}

void StoredDBODataSource::sac(unsigned char sac)
{
    this->sac_ = sac;
}

void StoredDBODataSource::shortName(const std::string &short_name)
{
    this->short_name_ = short_name;
}

void StoredDBODataSource::sic(unsigned char sic)
{
    this->sic_ = sic;
}
