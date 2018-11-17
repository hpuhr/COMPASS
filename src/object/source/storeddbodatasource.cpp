#include "storeddbodatasource.h"
#include "storeddbodatasourcewidget.h"
#include "dbodatasource.h"

#include "dbobject.h"

StoredDBODataSource::StoredDBODataSource(const std::string& class_id, const std::string& instance_id, DBObject* object)
    : Configurable(class_id, instance_id, object), object_(object)
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

StoredDBODataSource::~StoredDBODataSource ()
{
}

StoredDBODataSource& StoredDBODataSource::operator=(DBODataSource& other)
{
    //id_ = other.id();
    name_ = other.name();
    if (other.hasShortName())
        short_name_ = other.shortName();
    if (other.hasSac())
        sac_ = other.sac();
    if (other.hasSic())
        sic_ = other.sic();
    if (other.hasLatitude())
        latitude_ = other.latitude();
    if (other.hasLongitude())
        longitude_ = other.longitude();
    if (other.hasAltitude())
        altitude_ = other.altitude();

    return *this;
}

StoredDBODataSource& StoredDBODataSource::operator=(StoredDBODataSource&& other)
{
    loginf << "StoredDBODataSource: move operator: moving";

    object_ = other.object_;
    other.object_ = nullptr;

    //id_ = other.id_;

    name_ = other.name_;
    other.name_ = "";

    short_name_ = other.short_name_;
    other.short_name_ = "";

    sac_ = other.sac_;
    other.sac_ = 0;

    sic_ = other.sic_;
    other.sic_ = 0;

    latitude_ = other.latitude_;
    other.latitude_ = 0.0;

    longitude_ = other.longitude_;
    other.longitude_ = 0.0;

    altitude_ = other.altitude_;
    other.altitude_ = 0;

    widget_ = std::move(other.widget_);
    if (widget_)
        widget_->setDataSource(*this);
    //other.widget_ = nullptr;

    other.configuration().updateParameterPointer ("id", &id_);
    other.configuration().updateParameterPointer ("name", &name_);
    other.configuration().updateParameterPointer ("short_name", &short_name_);
    other.configuration().updateParameterPointer ("sac", &sac_);
    other.configuration().updateParameterPointer ("sic", &sic_);
    other.configuration().updateParameterPointer ("latitude", &latitude_);
    other.configuration().updateParameterPointer ("longitude", &longitude_);
    other.configuration().updateParameterPointer ("altitude", &altitude_);

    return static_cast<StoredDBODataSource&>(Configurable::operator=(std::move(other)));
}


double StoredDBODataSource::altitude() const
{
    return altitude_;
}

unsigned int StoredDBODataSource::id() const
{
    return id_;
}

//void StoredDBODataSource::id(unsigned int id)
//{
//    // TODO
//    assert (false);
//}

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


void StoredDBODataSource::name(const std::string &name)
{
    loginf << "StoredDBODataSource " << id_ << ": name: " << name;
    name_ = name;
}

void StoredDBODataSource::shortName(const std::string &short_name)
{
    loginf << "StoredDBODataSource " << id_ << ": shortName: " << short_name;
    this->short_name_ = short_name;
}

void StoredDBODataSource::altitude(double altitude)
{
    loginf << "StoredDBODataSource " << id_ << ": altitude: " << altitude;
    this->altitude_ = altitude;
}

void StoredDBODataSource::latitude(double latitude)
{
    loginf << "StoredDBODataSource " << id_ << ": latitude: " << latitude;
    this->latitude_ = latitude;
}

void StoredDBODataSource::longitude(double longitude)
{
    loginf << "StoredDBODataSource " << id_ << ": longitude: " << longitude;
    this->longitude_ = longitude;
}

void StoredDBODataSource::sac(unsigned char sac)
{
    loginf << "StoredDBODataSource " << id_ << ": sac: " << static_cast<int> (sac);
    this->sac_ = sac;
}

void StoredDBODataSource::sic(unsigned char sic)
{
    loginf << "StoredDBODataSource " << id_ << ": sic: " << static_cast<int> (sic);
    this->sic_ = sic;
}

StoredDBODataSourceWidget* StoredDBODataSource::widget (bool add_headers, QWidget* parent, Qt::WindowFlags f)
{
    if (!widget_)
    {
        widget_.reset (new StoredDBODataSourceWidget (*this, add_headers, parent, f));
        assert (widget_);
    }
    return widget_.get();
}
