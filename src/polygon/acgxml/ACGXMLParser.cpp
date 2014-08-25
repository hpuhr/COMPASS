/*
 * ACGXMLParser.cpp
 *
 *  Created on: Aug 11, 2014
 *      Author: sk
 */

#include "ACGXMLParser.h"
#include "Logger.h"
#include "Abd.h"
#include "Ase.h"
#include "String.h"
#include "AirspaceSectorManager.h"
#include "AirspaceSector.h"
#include "ProjectionManager.h"

#include <cmath>

using namespace tinyxml2;
using namespace Utils::String;
using namespace ACGXML;

ACGXMLParser::ACGXMLParser()
: AirspaceSectorParser()
{

}

ACGXMLParser::~ACGXMLParser()
{
}

void ACGXMLParser::parse (std::string filename, AirspaceSector *base_sector)
{
    logdbg  << "ACGXMLParser: parse: opening '" << filename << "'";
    XMLDocument *config_file_doc = new XMLDocument ();

    if (config_file_doc->LoadFile(filename.c_str()) == 0)
    {
        loginf  << "ACGXMLParser: parse: file '" << filename << "' opened";

        XMLElement *doc;
        XMLElement *child;

        for ( doc = config_file_doc->FirstChildElement(); doc != 0;
                doc = doc->NextSiblingElement())
        {
            if (strcmp ("AIXM-Snapshot", doc->Value() ) != 0)
            {
                logerr << "ACGXMLParser: parse: missing AIXM-Snapshot";
                break;
            }

            logdbg  << "ACGXMLParser: parse: found AIXM-Snapshot";

            for (child = doc->FirstChildElement(); child != 0; child = child->NextSiblingElement())
            {
                logdbg  << "ACGXMLParser: parse: found element '" << child->Value() << "'";
                if (strcmp ("Abd", child->Value() ) == 0)
                {
                    logdbg  << "ACGXMLParser: parse: is Abd";
                    parseAdb (child);
                }
                else if (strcmp ("Ase", child->Value() ) == 0)
                {
                    logdbg  << "ACGXMLParser: parse: is Ase";
                    parseAse (child);
                }
                else
                {
                    throw std::runtime_error (std::string("ACGXMLParser: parse: unknown section '")+child->Value()+"'");
                }
            }
        }
    }
    else
    {
        logerr << "ACGXMLParser: parse: could not load file '" << filename << "'";
        throw std::runtime_error ("ACGXMLParser: parse: load error");
    }

    loginf  << "ACGXMLParser: parse: file '" << filename << "' read, found " << abds_.size() << " Abds and " << ases_.size() << " Ases";
    delete config_file_doc;

    checkConistency();
    loginf << "ACGXMLParser: parse: checking consistency done";

    createSectors(base_sector);

}

void ACGXMLParser::clear()
{
    abds_.clear();
    ases_.clear();
}

void ACGXMLParser::parseAdb (tinyxml2::XMLElement *adb_elem)
{
    logdbg  << "ACGXMLParser: parseAdb: element '" << adb_elem->Value() << "'" ;

    Abd *abd_ptr=0;

    XMLElement *element;
    for (element = adb_elem->FirstChildElement(); element != 0; element = element->NextSiblingElement())
    {
        logdbg  << "ACGXMLParser: parseAdb: found element '" << element->Value() << "'";
        if (strcmp ("AbdUid", element->Value() ) == 0)
        {
            std::string mid;

            const XMLAttribute* attribute=element->FirstAttribute();
            while (attribute)
            {
                logdbg  << "ACGXMLParser: parseAdb: attribute '" << attribute->Name() << "' value '"<< attribute->Value() << "'";
                if (strcmp ("mid", attribute->Name()) == 0)
                {
                    assert (mid.size() == 0); //undefined
                    mid=attribute->Value();
                }
                else
                    throw std::runtime_error (std::string ("ACGXMLParser: parseAdb: unknown attribute ")+attribute->Name());

                attribute=attribute->Next();
            }

            assert (mid.size() != 0);

            bool ok=true;
            unsigned int mid_num = intFromString(mid, &ok);
            assert (ok);

            assert (abds_.find(mid_num) == abds_.end());
            Abd &adb = abds_[mid_num];
            adb.mid_=mid_num;

            XMLElement *child;
            for (child = element->FirstChildElement(); child != 0; child = child->NextSiblingElement())
            {
                logdbg  << "ACGXMLParser: parseAdb: found child '" << child->Value() << "'";

                if (strcmp ("AseUid", child->Value() ) == 0)
                {
                    parseAdbAseUid (child, adb);
                }
                else
                {
                    throw std::runtime_error ("ACGXMLParser: parseAdb: unknown AbdUid section '"+std::string(child->Value())+"'");
                }
            }

            abd_ptr = &(abds_[mid_num]);
        }
        else if (strcmp ("Avx", element->Value() ) == 0)
        {
            assert (abd_ptr);
            parseAdbAvx (element, *abd_ptr);
        }
        else
        {
            throw std::runtime_error ("ACGXMLParser: parseAdb: unknown adb section '"+std::string(element->Value())+"'");
        }
    }
}

void ACGXMLParser::parseAdbAseUid (tinyxml2::XMLElement *elem, ACGXML::Abd &adb)
{
    logdbg << "ACGXMLParser: parseAdbAseUid: element '" << elem->Value();

    std::string mid;

    const XMLAttribute* attribute=elem->FirstAttribute();
    while (attribute)
    {
        logdbg  << "ACGXMLParser: parseAdbAseUid: attribute '" << attribute->Name() << "' value '"<< attribute->Value() << "'";
        if (strcmp ("mid", attribute->Name()) == 0)
        {
            assert (mid.size() == 0); //undefined
            mid=attribute->Value();
        }
        else
            throw std::runtime_error (std::string ("ACGXMLParser: parseAdbAseUid: unknown AbdUid attribute ")+attribute->Name());

        attribute=attribute->Next();
    }

    assert (mid.size() != 0);
    bool ok;
    unsigned int mid_num = intFromString(mid, &ok);
    assert (ok);

    AseUid ase_uid;
    ase_uid.mid_ = mid_num;

    XMLElement *child;

    for (child = elem->FirstChildElement(); child != 0; child = child->NextSiblingElement())
    {
        logdbg  << "ACGXMLParser: parseAdbAseUid: found child '" << child->Value() << "' text '" << child->GetText() << "'" ;

        if (strcmp ("codeType", child->Value() ) == 0)
            ase_uid.code_type_ = child->GetText();
        else if (strcmp ("codeId", child->Value() ) == 0)
            ase_uid.code_id_ = child->GetText();
        else
            throw std::runtime_error (std::string ("ACGXMLParser: parseAdbAseUid: unknown attribute ")+child->Value());
    }

    assert (ase_uid.code_type_.size() != 0);
    assert (ase_uid.code_id_.size() != 0);

    adb.ase_uid_=ase_uid;
}

void ACGXMLParser::parseAdbAvx (tinyxml2::XMLElement *elem, ACGXML::Abd &adb)
{
    logdbg << "ACGXMLParser: parseAdbAvx: element '" << elem->Value() << "'";

    Avx avx;
    bool ok=false;
    XMLElement *child;

    avx.has_gbr_uid_=false;
    avx.gbr_mid_=666;
    avx.has_arc_=false;
    avx.geo_lat_arc_=0.0;
    avx.geo_long_arc_=0.0;
    avx.val_radius_arc_=0.0;

    bool has_geo_lat=false;
    bool has_geo_long=false;
    bool has_geo_lat_arc=false;
    bool has_geo_long_arc=false;
    bool has_val_radius_arc=false;

    double tmp_num;
    std::string tmp_str;

    for (child = elem->FirstChildElement(); child != 0; child = child->NextSiblingElement())
    {
        logdbg  << "ACGXMLParser: parseAdbAvx: found Avx child '" << child->Value() << "' text '" << child->GetText() << "'" ;

//        <GbrUid mid="7">
//            <txtName>AUSTRIA_SWITZERLAND</txtName>
//        </GbrUid>
//        <codeType>FNT</codeType>
//        <geoLat>472430.4272N</geoLat>
//        <geoLong>0093906.8780E</geoLong>
//        <codeDatum>WGE</codeDatum>

//        <geoLatArc>472830.0000N</geoLatArc>
//        <geoLongArc>0133620.0000E</geoLongArc>
//        <valRadiusArc>3.0150</valRadiusArc>
//        <uomRadiusArc>NM</uomRadiusArc>

//        <VorUidCen mid="16"> ignored
//            <codeId>FMD</codeId>
//            <geoLat>480618.4050N</geoLat>
//            <geoLong>0163745.3503E</geoLong>
//        </VorUidCen>

        if (strcmp ("GbrUid", child->Value() ) == 0)
        {
            tmp_str = child->Attribute("mid");
            assert (tmp_str.size() != 0);
            tmp_num = intFromString(tmp_str, &ok);
            assert (ok);

            avx.has_gbr_uid_=true;
            avx.gbr_mid_= tmp_num;

            assert (strcmp ("txtName", child->FirstChildElement()->Value()) == 0);
            tmp_str = child->FirstChildElement()->GetText();
            assert (tmp_str.size() != 0);

            avx.gbr_txt_name_ = tmp_str;
        }
        else if (strcmp ("codeType", child->Value() ) == 0)
            avx.code_type_ = child->GetText();
        else if (strcmp ("geoLat", child->Value() ) == 0)
        {
            tmp_str = child->GetText();
            tmp_num = doubleFromLatitudeString(tmp_str, ok);
            assert (ok);
            //loginf << "ACGXMLParser: parseAdbAvx: lat '" << lat_str << "' num " <<  lat;
            avx.geo_lat_=tmp_num;
            has_geo_lat=true;
        }
        else if (strcmp ("geoLong", child->Value() ) == 0)
        {
            tmp_str = child->GetText();
            tmp_num = doubleFromLongitudeString(tmp_str, ok);
            assert (ok);
            //loginf << "ACGXMLParser: parseAdbAvx: long '" << lon_str << "' num " <<  lon;
            avx.geo_long_=tmp_num;
            has_geo_long=true;
        }
        else if (strcmp ("codeDatum", child->Value() ) == 0)
            avx.code_datum_ = child->GetText();
        else if (strcmp ("geoLatArc", child->Value() ) == 0)
        {
            tmp_str = child->GetText();
            tmp_num = doubleFromLatitudeString(tmp_str, ok);
            assert (ok);
            //loginf << "ACGXMLParser: parseAdbAvx: lat '" << lat_str << "' num " <<  lat;
            avx.geo_lat_arc_=tmp_num;
            avx.has_arc_=true;
            has_geo_lat_arc=true;
        }
        else if (strcmp ("geoLongArc", child->Value() ) == 0)
        {
            tmp_str = child->GetText();
            tmp_num = doubleFromLongitudeString(tmp_str, ok);
            assert (ok);
            //loginf << "ACGXMLParser: parseAdbAvx: long '" << lon_str << "' num " <<  lon;
            avx.geo_long_arc_=tmp_num;
            avx.has_arc_=true;
        }
        else if (strcmp ("valRadiusArc", child->Value() ) == 0)
        {
            tmp_str = child->GetText();
            tmp_num = doubleFromString(tmp_str, &ok);
            assert (ok);
            //loginf << "ACGXMLParser: parseAdbAvx: long '" << lon_str << "' num " <<  lon;
            avx.val_radius_arc_=tmp_num;
            avx.has_arc_=true;
            has_geo_long_arc=true;
            has_val_radius_arc=true;
        }
        else if (strcmp ("uomRadiusArc", child->Value() ) == 0)
        {
            avx.uom_radius_arc_ = child->GetText();
            avx.has_arc_=true;
        }
        else if (strcmp ("VorUidCen", child->Value() ) == 0) // no can do - what's that a please near katmandu
            continue;
        else
            throw std::runtime_error (std::string ("ACGXMLParser: parseAdbAvx: unknown attribute ")+child->Value());
    }

    assert (avx.code_type_.size() != 0);
    assert (has_geo_lat);
    assert (has_geo_long);
    assert (avx.code_datum_.size() != 0);

    if (avx.has_arc_)
    {
        assert (has_geo_lat_arc);
        assert (has_geo_long_arc);
        assert (has_val_radius_arc);
        assert (avx.uom_radius_arc_.size() != 0);
    }

    adb.avxes_.push_back (avx);
}

void ACGXMLParser::parseAse (tinyxml2::XMLElement *ase_elem)
{
    logdbg << "ACGXMLParser: parseAse: element '" << ase_elem->Value() << "'";

    Ase ase;
    bool ok=false;
    XMLElement *child;

    ase.has_minimum_=false;

    bool has_uid_mid=false;
    bool has_val_dist_ver_upper=false;
    bool has_val_dist_ver_lower=false;
    bool val_dist_ver_minimum=false;

    double tmp_num;
    std::string tmp_str;

    for (child = ase_elem->FirstChildElement(); child != 0; child = child->NextSiblingElement())
    {
        logdbg  << "ACGXMLParser: parseAse: found Ase child '" << child->Value() << "' text '" << child->GetText() << "'" ;

        //        <AseUid mid="1395">
        //            <codeType>RAS</codeType>
        //            <codeId>LO61</codeId>
        //        </AseUid>
        //        <txtLocalType>MFA-ASR</txtLocalType>
        //        <codeMil>CIVIL</codeMil>
        //        <codeDistVerUpper>STD</codeDistVerUpper>
        //        <valDistVerUpper>999</valDistVerUpper>
        //        <uomDistVerUpper>FL</uomDistVerUpper>
        //        <codeDistVerLower>ALT</codeDistVerLower>
        //        <valDistVerLower>7700</valDistVerLower>
        //        <uomDistVerLower>FT</uomDistVerLower>

//        <codeDistVerMnm>ALT</codeDistVerMnm>
//        <valDistVerMnm>5500</valDistVerMnm>
//        <uomDistVerMnm>FT</uomDistVerMnm>

        //        <Att>
        //            <codeWorkHr>H24</codeWorkHr>
        //        </Att>

        if (strcmp ("AseUid", child->Value() ) == 0)
        {
            tmp_str = child->Attribute("mid");
            assert (tmp_str.size() != 0);
            tmp_num = intFromString(tmp_str, &ok);
            assert (ok);

            has_uid_mid=true;
            ase.uid_mid_= tmp_num;

            XMLElement *aseuidchild;
            for (aseuidchild = child->FirstChildElement(); aseuidchild != 0; aseuidchild = aseuidchild->NextSiblingElement())
            {
                if (strcmp ("codeType", aseuidchild->Value()) == 0)
                {
                    tmp_str = aseuidchild->GetText();
                    assert (tmp_str.size() != 0);
                    ase.uid_code_type_ = tmp_str;
                }
                else if(strcmp ("codeId", aseuidchild->Value()) == 0)
                {
                    tmp_str = aseuidchild->GetText();
                    assert (tmp_str.size() != 0);
                    ase.uid_code_id_ = tmp_str;
                }
                else
                    throw std::runtime_error (std::string ("ACGXMLParser: parseAse: unknown attribute ")+aseuidchild->Value());
            }
        }
        else if (strcmp ("txtLocalType", child->Value() ) == 0)
            ase.txt_local_type_ = child->GetText();
        else if (strcmp ("codeMil", child->Value() ) == 0)
            ase.code_mil_ = child->GetText();
        else if (strcmp ("codeDistVerUpper", child->Value() ) == 0)
            ase.code_dist_ver_upper_ = child->GetText();
        else if (strcmp ("valDistVerUpper", child->Value() ) == 0)
        {
            tmp_str = child->GetText();
            tmp_num = doubleFromString(tmp_str, &ok);
            assert (ok);
            ase.val_dist_ver_upper_=tmp_num;
            has_val_dist_ver_upper=true;
        }
        else if (strcmp ("uomDistVerUpper", child->Value() ) == 0)
            ase.uom_dist_ver_upper_ = child->GetText();
        else if (strcmp ("codeDistVerLower", child->Value() ) == 0)
            ase.code_dist_ver_lower_= child->GetText();
        else if (strcmp ("valDistVerLower", child->Value() ) == 0)
        {
            tmp_str = child->GetText();
            tmp_num = doubleFromString(tmp_str, &ok);
            assert (ok);
            ase.val_dist_ver_lower_=tmp_num;
            has_val_dist_ver_lower=true;
        }
        else if (strcmp ("uomDistVerLower", child->Value() ) == 0)
            ase.uom_dist_ver_lower_ = child->GetText();
        else if (strcmp ("codeDistVerMnm", child->Value() ) == 0)
        {
            ase.code_dist_ver_minimum_= child->GetText();
            ase.has_minimum_=true;
        }
        else if (strcmp ("valDistVerMnm", child->Value() ) == 0)
        {
            tmp_str = child->GetText();
            tmp_num = doubleFromString(tmp_str, &ok);
            assert (ok);
            ase.val_dist_ver_minimum_=tmp_num;
            ase.has_minimum_=true;
            val_dist_ver_minimum=true;
        }
        else if (strcmp ("uomDistVerMnm", child->Value() ) == 0)
        {
            ase.has_minimum_=true;
            ase.uom_dist_ver_minimum_ = child->GetText();
        }
        else if (strcmp ("Att", child->Value() ) == 0)
        {
            assert (strcmp ("codeWorkHr", child->FirstChildElement()->Value()) == 0);
            tmp_str = child->FirstChildElement()->GetText();
            assert (tmp_str.size() != 0);
            ase.code_work_hr_ = tmp_str;
        }
        else
            throw std::runtime_error (std::string ("ACGXMLParser: parseAse: unknown attribute ")+child->Value());
    }

    assert (has_uid_mid);
    assert (ase.uid_code_type_.size() != 0);
    assert (ase.uid_code_id_.size() != 0);
    assert (ase.txt_local_type_.size() != 0);
    assert (ase.code_mil_.size() != 0);
    assert (ase.code_dist_ver_upper_.size() != 0);
    assert (has_val_dist_ver_upper);
    assert (ase.uom_dist_ver_upper_.size() != 0);
    assert (ase.code_dist_ver_lower_.size() != 0);
    assert (has_val_dist_ver_lower);
    assert (ase.uom_dist_ver_lower_.size() != 0);

    if (ase.has_minimum_)
    {
        assert (ase.code_dist_ver_minimum_.size() != 0);
        assert (val_dist_ver_minimum);
        assert (ase.uom_dist_ver_minimum_.size() != 0);
    }

    assert (ase.code_work_hr_.size() != 0);

    assert (ases_.find (ase.uid_mid_) == ases_.end());
    ases_[ase.uid_mid_] = ase;
}

void ACGXMLParser::checkConistency ()
{
    std::map <unsigned int, Abd>::iterator it;

    unsigned int ase_mid;
    for (it = abds_.begin(); it != abds_.end(); it++)
    {
        ase_mid = it->second.ase_uid_.mid_;
        assert (ases_.find(ase_mid) != ases_.end());
    }
}

void ACGXMLParser::createSectors (AirspaceSector *base_sector)
{
    loginf << "ACGXMLParser: createSectors";

    assert (base_sector);

    std::map <unsigned int, Abd>::iterator it;

    unsigned int ase_mid;
    std::string name;

    ProjectionManager &proj_man = ProjectionManager::getInstance ();
    double rad2deg = 360.0/(2.0*M_PI);
    double deg2rad = 1.0/rad2deg;
    //double pi_half = M_PI/2.0;

    for (it = abds_.begin(); it != abds_.end(); it++)
    {
        Abd &abd = it->second;

        ase_mid = abd.ase_uid_.mid_;
        name = abd.ase_uid_.code_id_;

        loginf << "ACGXMLParser: createSectors: sector '" << name << "' with " << abd.avxes_.size() << " points and base sector " << base_sector->getName();

        assert (ases_.find(ase_mid) != ases_.end());
        Ase &ase = ases_.at(ase_mid);

        if (AirspaceSectorManager::getInstance().hasSector(name))
        {
            logwrn << "ACGXMLParser: createSectors: sector '" << name << "' already exists, deleting";
            AirspaceSectorManager::getInstance().deleteSectorIfPossible(name);
            assert (!AirspaceSectorManager::getInstance().hasSector(name));
        }

        AirspaceSectorManager::getInstance().addNewSector(name);
        assert (AirspaceSectorManager::getInstance().hasSector(name));
        AirspaceSector *sector = AirspaceSectorManager::getInstance().getSector(name);

        unsigned int size = abd.avxes_.size();
        for (unsigned int cnt=0; cnt < size; cnt++)
        {
            Avx &avx = abd.avxes_.at(cnt);

            if (avx.code_type_ == "GRC") //normal polygon point
                sector->addPoint(avx.geo_lat_, avx.geo_long_);
            else if (avx.code_type_ == "FNT") // border point start
            {
                loginf << "ACGXMLParser: createSectors: sector '" << name << " with FNT point at cnt " << cnt << " size " << abd.avxes_.size();

                sector->addPoint(avx.geo_lat_, avx.geo_long_);

                Avx avx2;
                if (cnt == abd.avxes_.size()-1)
                    avx2 = abd.avxes_.at(0);
                else
                    avx2 = abd.avxes_.at(cnt+1); // border point end
                //assert (avx2.code_type_ == "FNT");

                sector->addPoints(base_sector->getPointsBetween (avx.geo_lat_, avx.geo_long_, avx2.geo_lat_, avx2.geo_long_)); // shot & cut
//                cnt++;
            }
            else if (avx.code_type_ == "CCA" || avx.code_type_ == "CWA") // circle
            {
                loginf << "ACGXMLParser: createSectors: starting circle in sector '" << name << "'";
                assert (avx.has_arc_);
                assert (avx.uom_radius_arc_ == "NM");

                double center_lat = avx.geo_lat_arc_;
                double center_long = avx.geo_long_arc_;

                double rad_nm = avx.val_radius_arc_;
                double rad_meter = rad_nm * 1852.0;

                double start_lat = avx.geo_lat_;
                double start_long = avx.geo_long_;
                sector->addPoint(start_lat, start_long);

                double end_lat;
                double end_long;

                if (cnt == abd.avxes_.size()-1)
                {
                    end_lat = abd.avxes_.at(0).geo_lat_;
                    end_long  = abd.avxes_.at(0).geo_long_;
                }
                else
                {
                    end_lat = abd.avxes_.at(cnt+1).geo_lat_;
                    end_long  = abd.avxes_.at(cnt+1).geo_long_;
                }

                //project stuff
                double center_x, center_y;
                proj_man.geo2Cart(center_lat, center_long, center_x, center_y, false);
                double start_x, start_y;
                proj_man.geo2Cart(start_lat, start_long, start_x, start_y, false);
                double end_x, end_y;
                proj_man.geo2Cart(end_lat, end_long, end_x, end_y, false);

                logdbg << "rad " << rad_meter <<" rad 1 " << sqrt (pow(center_x-start_x, 2)+pow(center_y-start_y, 2))
                        << " rad 2" << sqrt (pow(center_x-end_x, 2)+pow(center_y-end_y, 2));
                assert (sqrt (pow(center_x-start_x, 2)+pow(center_y-start_y, 2)) - rad_meter < 500.0); // should be exact to 10 m
                assert (sqrt (pow(center_x-end_x, 2)+pow(center_y-end_y, 2)) - rad_meter < 500.0);

//                double atan2(double y, double x);
//                Compute arc tangent with two parameters
//                Returns the principal value of the arc tangent of y/x, expressed in radians.
//                To compute the value, the function takes into account the sign of both arguments in order to determine the quadrant.
//                Return Value
//                Principal arc tangent of y/x, in the interval [-pi,+pi] radians.

                double start_azimuth = atan2 (start_y-center_y, start_x-center_x);
                double end_azimuth = atan2 (end_y-center_y, end_x-center_x);

                logdbg << "ACGXMLParser: createSectors: start_azimuth " << start_azimuth << " end_azimuth "
                        << end_azimuth << " type " << avx.code_type_;
                double current;
                double step;
                double end;

                double current_x, current_y;
                double current_lat, current_long;

                if (avx.code_type_ == "CCA")
                {
                    current = deg2rad* ceil(rad2deg*start_azimuth);
                    step = 1.0 * deg2rad;
                    end = deg2rad*floor (rad2deg*end_azimuth);

                    if (end < current)
                        end += 2*M_PI;
                }
                else
                {
                    current = deg2rad*floor (rad2deg*start_azimuth);
                    step = -1.0 * deg2rad;
                    end = deg2rad*ceil (rad2deg*end_azimuth);

                    if (end > current)
                        end -= 2*M_PI;
                }
                logdbg << "ACGXMLParser: createSectors: current " << current << " step " << step << " end " << end;

                unsigned int cnt2=0;
                while (1)
                {

                    current_x = center_x + rad_meter * cos (current);
                    current_y = center_y + rad_meter * sin (current);

                    proj_man.cart2geo(current_x, current_y, current_lat, current_long, false);

                    sector->addPoint(current_lat, current_long);

                    current += step;

//                    if (current > M_PI)
//                        current = current-(2*M_PI);
//                    if (current < -M_PI)
//                        current = current+(2*M_PI);

                    if (avx.code_type_ == "CCA")
                    {
                        if (current > end)
                            break;
                    }
                    else
                    {
                        if (current < end)
                            break;
                    }
                    cnt2++;
                    assert (cnt2 < 360);
                }
                loginf << "ACGXMLParser: createSectors: circle part with " << cnt2 << " points";
            }
            else
                throw std::runtime_error ("ACGXMLParser: createSectors: unknown code type '" + avx.code_type_ +"'");
        }

        sector->setHasOwnVolume(true);
        if (ase.has_minimum_)
            sector->setHeightMin(getHeight(ase.val_dist_ver_minimum_, ase.uom_dist_ver_minimum_));
        else
            sector->setHeightMin(getHeight(ase.val_dist_ver_lower_, ase.uom_dist_ver_lower_));
        sector->setHeightMax(getHeight(ase.val_dist_ver_upper_, ase.uom_dist_ver_upper_));
    }
}

double ACGXMLParser::getHeight (double value, std::string dist)
{
    if (dist == "FL")
        return value*100.0;
    else if (dist == "FT")
        return value;
    else
        throw std::runtime_error ("ACGXMLParser: getHeight: unknown dist '"+dist+"'");
}
