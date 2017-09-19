#include "dbodatasource.h"
#include "dbobject.h"
#include "dbodatasourcedefinitionwidget.h"

DBODataSourceDefinition::DBODataSourceDefinition(const std::string &class_id, const std::string &instance_id, DBObject* object)
    : Configurable (class_id, instance_id, object), object_(object)
{
    registerParameter ("schema", &schema_, "");
    registerParameter ("local_key", &local_key_, "");
    registerParameter ("meta_table", &meta_table_,  "");
    registerParameter ("foreign_key", &foreign_key_, "");
    registerParameter ("short_name_column", &short_name_column_, "");
    registerParameter ("name_column", &name_column_, "");
    registerParameter ("sac_column", &sac_column_, "");
    registerParameter ("sic_column", &sic_column_, "");
    registerParameter ("latitude_column", &latitude_column_, "");
    registerParameter ("longitude_column", &longitude_column_, "");
    registerParameter ("altitude_column", &altitude_column_, "");
}

DBODataSourceDefinition::~DBODataSourceDefinition()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

DBODataSourceDefinitionWidget* DBODataSourceDefinition::widget ()
{
    if (!widget_)
    {
        widget_ = new DBODataSourceDefinitionWidget (*object_, *this);
    }

    assert (widget_);
    return widget_;
}

std::string DBODataSourceDefinition::latitudeColumn() const
{
    return latitude_column_;
}

std::string DBODataSourceDefinition::longitudeColumn() const
{
    return longitude_column_;
}

void DBODataSourceDefinition::longitudeColumn(const std::string &longitude_column)
{
    loginf << "DBODataSourceDefinition: localKey: value " << longitude_column;
    longitude_column_ = longitude_column;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::shortNameColumn() const
{
    return short_name_column_;
}

void DBODataSourceDefinition::shortNameColumn(const std::string &short_name_column)
{
    loginf << "DBODataSourceDefinition: shortNameColumn: value " << short_name_column;
    short_name_column_ = short_name_column;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::sacColumn() const
{
    return sac_column_;
}

void DBODataSourceDefinition::sacColumn(const std::string &sac_column)
{
    loginf << "DBODataSourceDefinition: sacColumn: value " << sac_column;
    sac_column_ = sac_column;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::sicColumn() const
{
    return sic_column_;
}

void DBODataSourceDefinition::sicColumn(const std::string &sic_column)
{
    loginf << "DBODataSourceDefinition: sicColumn: value " << sic_column;
    sic_column_ = sic_column;
    emit definitionChangedSignal();
}

std::string DBODataSourceDefinition::altitudeColumn() const
{
    return altitude_column_;
}

void DBODataSourceDefinition::altitudeColumn(const std::string &altitude_column)
{
    loginf << "DBODataSourceDefinition: altitudeColumn: value " << altitude_column;
    altitude_column_ = altitude_column;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::latitudeColumn(const std::string &latitude_column)
{
    loginf << "DBODataSourceDefinition: latitudeColumn: value " << latitude_column;
    latitude_column_ = latitude_column;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::localKey(const std::string &local_key)
{
    loginf << "DBODataSourceDefinition: localKey: value " << local_key;
    local_key_ = local_key;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::metaTable(const std::string &meta_table)
{
    loginf << "DBODataSourceDefinition: metaTable: value " << meta_table;
    meta_table_ = meta_table;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::foreignKey(const std::string &foreign_key)
{
    loginf << "DBODataSourceDefinition: foreignKey: value " << foreign_key;
    foreign_key_ = foreign_key;
    emit definitionChangedSignal();
}

void DBODataSourceDefinition::nameColumn(const std::string &name_column)
{
    loginf << "DBODataSourceDefinition: nameColumn: value " << name_column;
    name_column_ = name_column;
    emit definitionChangedSignal();
}

DBODataSource::DBODataSource()
: id_(0), sac_(255), sic_(255), latitude_(360.0), longitude_(360), altitude_(1701)
//finalized_(false),  system_x_(0), system_y_(0)//, local_trans_x_(0), local_trans_y_(0)
{
    //deg2rad_ = 2*M_PI/360.0;
}

DBODataSource::~DBODataSource()
{

}

//void DBODataSource::finalize ()
//{
//    assert (!finalized_);

//    ProjectionManager::getInstance().geo2Cart(latitude_, longitude_, system_x_, system_y_, false);

//    logdbg << "DBODataSource: finalize: " << short_name_ << " lat " << latitude_ << " lon " << longitude_ << " x " << system_x_ << " y " << system_y_;
////    double center_system_x = ProjectionManager::getInstance().getCenterSystemX();
////    double center_system_y = ProjectionManager::getInstance().getCenterSystemY();

////    local_trans_x_ = center_system_x-system_x_;
////    local_trans_y_ = center_system_y-system_y_;

//    finalized_=true;
//}

// azimuth degrees, range & altitude in meters
//void DBODataSource::calculateSystemCoordinates (double azimuth, double slant_range, double altitude, bool has_altitude, double &sys_x, double &sys_y)
//{
//    if (!finalized_)
//        finalize ();

//    assert (finalized_);

//    double range;

////    if (slant_range <= altitude)
////    {
////        logerr << "DataSource: calculateSystemCoordinates: a " << azimuth << " sr " << slant_range << " alt " << altitude
////                << ", assuming range = slant range";
////        range = slant_range; // TODO pure act of desperation
////    }
////    else
////        range = sqrt (slant_range*slant_range-altitude*altitude); // TODO: flatland

//    if (has_altitude && slant_range > altitude)
//        range = sqrt (slant_range*slant_range-altitude*altitude);
//    else
//        range = slant_range; // TODO pure act of desperation

//    azimuth *= deg2rad_;

//    sys_x = range * sin (azimuth);
//    sys_y = range * cos (azimuth);

//    sys_x += system_x_;
//    sys_y += system_y_;

//    if (sys_x != sys_x || sys_y != sys_y)
//    {
//        logerr << "DBODataSource: calculateSystemCoordinates: a " << azimuth << " sr " << slant_range << " alt " << altitude
//                << " range " << range << " sys_x " << sys_x << " sys_y " << sys_y;
//        assert (false);
//    }
//}

double DBODataSource::altitude() const
{
    return altitude_;
}

unsigned int DBODataSource::id() const
{
    return id_;
}

double DBODataSource::latitude() const
{
    return latitude_;
}

double DBODataSource::longitude() const
{
    return longitude_;
}

const std::string &DBODataSource::name() const
{
    return name_;
}

unsigned char DBODataSource::sac() const
{
    return sac_;
}

const std::string &DBODataSource::shortName() const
{
    return short_name_;
}

unsigned char DBODataSource::sic() const
{
    return sic_;
}

void DBODataSource::altitude(double altitude)
{
    this->altitude_ = altitude;
}

void DBODataSource::id(unsigned int id)
{
    this->id_ = id;
}

void DBODataSource::latitude(double latitiude)
{
    this->latitude_ = latitiude;
}

void DBODataSource::longitude(double longitude)
{
    this->longitude_ = longitude;
}

void DBODataSource::name(const std::string &name)
{
    this->name_ = name;
}

void DBODataSource::sac(unsigned char sac)
{
    this->sac_ = sac;
}

void DBODataSource::shortName(const std::string &short_name)
{
    this->short_name_ = short_name;
}

void DBODataSource::sic(unsigned char sic)
{
    this->sic_ = sic;
}
