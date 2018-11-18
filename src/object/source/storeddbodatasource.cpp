#include "storeddbodatasource.h"
#include "storeddbodatasourcewidget.h"
#include "dbodatasource.h"

#include "dbobject.h"

StoredDBODataSource::StoredDBODataSource(const std::string& class_id, const std::string& instance_id, DBObject* object)
    : Configurable(class_id, instance_id, object), object_(object)
{
    registerParameter ("id", &id_, 0);
    registerParameter ("name", &name_, "");
    registerParameter ("has_short_name", &has_short_name_,  false);
    registerParameter ("short_name", &short_name_,  "");
    registerParameter ("has_sac", &has_sac_, false);
    registerParameter ("sac", &sac_, 0);
    registerParameter ("has_sic", &has_sic_, false);
    registerParameter ("sic", &sic_, 0);
    registerParameter ("has_latitude", &has_latitude_, false);
    registerParameter ("latitude", &latitude_, 0);
    registerParameter ("has_longitude", &has_longitude_, false);
    registerParameter ("longitude", &longitude_, 0);
    registerParameter ("has_altitude", &has_altitude_, false);
    registerParameter ("altitude", &altitude_, 0);
}

StoredDBODataSource::~StoredDBODataSource ()
{
}

StoredDBODataSource& StoredDBODataSource::operator=(DBODataSource& other)
{
    //id_ = other.id();
    name_ = other.name();
    has_short_name_ = other.hasShortName();
    if (has_short_name_)
        short_name_ = other.shortName();
    has_sac_ = other.hasSac();
    if (has_sac_)
        sac_ = other.sac();
    has_sic_ = other.hasSic();
    if (has_sic_)
        sic_ = other.sic();
    has_latitude_ = other.hasLatitude();
    if (has_latitude_)
        latitude_ = other.latitude();
    has_longitude_ = other.hasLongitude();
    if (has_longitude_)
        longitude_ = other.longitude();
    has_altitude_ = other.hasAltitude();
    if (has_altitude_)
        altitude_ = other.altitude();

    if (widget_)
        widget_->update();

    return *this;
}

StoredDBODataSource& StoredDBODataSource::operator=(StoredDBODataSource&& other)
{
    loginf << "StoredDBODataSource: move operator: moving";

    object_ = other.object_;
    other.object_ = nullptr;

    //id_ = other.id_; do not use others id, keep your own

    name_ = other.name_;

    has_short_name_ = other.has_short_name_;
    short_name_ = other.short_name_;

    has_sac_ = other.has_sac_;
    sac_ = other.sac_;

    has_sic_ = other.has_sic_;
    sic_ = other.sic_;

    has_latitude_ = other.has_latitude_;
    latitude_ = other.latitude_;

    has_longitude_ = other.has_longitude_;
    longitude_ = other.longitude_;

    has_altitude_ = other.has_altitude_;
    altitude_ = other.altitude_;

    widget_ = std::move(other.widget_);
    if (widget_)
        widget_->setDataSource(*this);

    other.configuration().updateParameterPointer ("id", &id_);
    other.configuration().updateParameterPointer ("name", &name_);
    other.configuration().updateParameterPointer ("has_short_name", &has_short_name_);
    other.configuration().updateParameterPointer ("short_name", &short_name_);
    other.configuration().updateParameterPointer ("has_sac", &has_sac_);
    other.configuration().updateParameterPointer ("sac", &sac_);
    other.configuration().updateParameterPointer ("has_sic", &has_sic_);
    other.configuration().updateParameterPointer ("sic", &sic_);
    other.configuration().updateParameterPointer ("has_latitude", &has_latitude_);
    other.configuration().updateParameterPointer ("latitude", &latitude_);
    other.configuration().updateParameterPointer ("has_longitude", &has_longitude_);
    other.configuration().updateParameterPointer ("longitude", &longitude_);
    other.configuration().updateParameterPointer ("has_altitude", &has_altitude_);
    other.configuration().updateParameterPointer ("altitude", &altitude_);

    return static_cast<StoredDBODataSource&>(Configurable::operator=(std::move(other)));
}

bool StoredDBODataSource::operator==(DBODataSource& other)
{
    logdbg << "StoredDBODataSource: operator==: name " << (name_ == other.name())
           << " short " << (short_name_ == other.shortName())
           << " sac " << (sac_ == other.sac())
           << " sic " << (sic_ == other.sic())
           << " lat " << (fabs(latitude_ - other.latitude()) < 1e-10)
           << " long " << (fabs(longitude_ - other.longitude()) < 1e-10)
           << " alt " << (fabs(altitude_ - other.altitude()) < 1e-10);

    return (name_ == other.name()) &&
            (has_short_name_ == other.hasShortName()) &&
            (has_short_name_ ? short_name_ == other.shortName() : true) &&
            (has_sac_ == other.hasSac()) &&
            (has_sac_ ? sac_ == other.sac() : true) &&
            (has_sic_ == other.hasSic()) &&
            (has_sic_ ? sic_ == other.sic() : true) &&
            (has_latitude_ == other.hasLatitude()) &&
            (has_latitude_ ? fabs(latitude_ - other.latitude()) < 1e-10 : true) &&
            (has_longitude_ == other.hasLongitude()) &&
            (has_longitude_ ? fabs(longitude_ - other.longitude()) < 1e-10 : true) &&
            (has_altitude_ == other.hasAltitude()) &&
            (has_altitude_? fabs(altitude_ - other.altitude()) < 1e-10 : true);
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


void StoredDBODataSource::name(const std::string &name)
{
    loginf << "StoredDBODataSource " << id_ << ": name: " << name;
    name_ = name;
}

void StoredDBODataSource::shortName(const std::string &short_name)
{
    loginf << "StoredDBODataSource " << id_ << ": shortName: " << short_name;
    has_short_name_ = true;
    this->short_name_ = short_name;
}

void StoredDBODataSource::altitude(double altitude)
{
    loginf << "StoredDBODataSource " << id_ << ": altitude: " << altitude;
    has_altitude_ = true;
    this->altitude_ = altitude;
}

void StoredDBODataSource::latitude(double latitude)
{
    loginf << "StoredDBODataSource " << id_ << ": latitude: " << latitude;
    has_latitude_ = true;
    this->latitude_ = latitude;
}

void StoredDBODataSource::longitude(double longitude)
{
    loginf << "StoredDBODataSource " << id_ << ": longitude: " << longitude;
    has_longitude_ = true;
    this->longitude_ = longitude;
}

void StoredDBODataSource::sac(unsigned char sac)
{
    loginf << "StoredDBODataSource " << id_ << ": sac: " << static_cast<int> (sac);
    has_sac_ = true;
    this->sac_ = sac;
}

void StoredDBODataSource::sic(unsigned char sic)
{
    loginf << "StoredDBODataSource " << id_ << ": sic: " << static_cast<int> (sic);
    has_sic_ = true;
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

bool StoredDBODataSource::hasSac() const
{
    return has_sac_;
}

bool StoredDBODataSource::hasSic() const
{
    return has_sic_;
}

bool StoredDBODataSource::hasLatitude() const
{
    return has_latitude_;
}

bool StoredDBODataSource::hasLongitude() const
{
    return has_longitude_;
}

bool StoredDBODataSource::hasAltitude() const
{
    return has_altitude_;
}

bool StoredDBODataSource::hasShortName() const
{
    return has_short_name_;
}
