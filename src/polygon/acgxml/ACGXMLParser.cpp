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

void ACGXMLParser::parse (std::string filename)
{
    loginf  << "ACGXMLParser: parse: opening '" << filename << "'";
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

            loginf  << "ACGXMLParser: parse: found AIXM-Snapshot";

            for (child = doc->FirstChildElement(); child != 0; child = child->NextSiblingElement())
            {
                loginf  << "ACGXMLParser: parse: found element '" << child->Value() << "'";
                if (strcmp ("Abd", child->Value() ) == 0)
                {
                    loginf  << "ACGXMLParser: parse: is Abd";
                    parseAdb (child);
                }
                else if (strcmp ("Ase", child->Value() ) == 0)
                {
                    loginf  << "ACGXMLParser: parse: is Ase";
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

    loginf  << "ACGXMLParser: parse: file '" << filename << "' read";
    delete config_file_doc;
}

void ACGXMLParser::clear()
{
    abds_.clear();
    ases_.clear();
}

void ACGXMLParser::parseAdb (tinyxml2::XMLElement *adb_elem)
{
    loginf  << "ACGXMLParser: parseAdb: element '" << adb_elem->Value() << "'" ;

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
                loginf  << "ACGXMLParser: parseAdb: attribute '" << attribute->Name() << "' value '"<< attribute->Value() << "'";
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
            adb.setMid(mid_num);

            XMLElement *child;
            for (child = element->FirstChildElement(); child != 0; child = child->NextSiblingElement())
            {
                loginf  << "ACGXMLParser: parseAdb: found child '" << child->Value() << "'";

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
    loginf << "ACGXMLParser: parseAdbAseUid: element '" << elem->Value();

    std::string mid;

    const XMLAttribute* attribute=elem->FirstAttribute();
    while (attribute)
    {
        loginf  << "ACGXMLParser: parseAdbAseUid: attribute '" << attribute->Name() << "' value '"<< attribute->Value() << "'";
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

    AseUid ast_uid;
    ast_uid.mid_ = mid_num;

    XMLElement *child;

    for (child = elem->FirstChildElement(); child != 0; child = child->NextSiblingElement())
    {
        loginf  << "ACGXMLParser: parseAdbAseUid: found child '" << child->Value() << "' text '" << child->GetText() << "'" ;

        if (strcmp ("codeType", child->Value() ) == 0)
            ast_uid.code_type_ = child->GetText();
        else if (strcmp ("codeId", child->Value() ) == 0)
            ast_uid.code_id_ = child->GetText();
        else
            throw std::runtime_error (std::string ("ACGXMLParser: parseAdbAseUid: unknown attribute ")+child->Value());
    }

    assert (ast_uid.code_type_.size() != 0);
    assert (ast_uid.code_id_.size() != 0);

    adb.setAseUid(ast_uid);
}

void ACGXMLParser::parseAdbAvx (tinyxml2::XMLElement *elem, ACGXML::Abd &adb)
{
    loginf << "ACGXMLParser: parseAdbAvx: element '" << elem->Value() << "'";

    Avx avx;
    bool ok=false;
    XMLElement *child;

    avx.has_gbr_uid_=false;
    avx.gbr_mid_=666;
    avx.has_arc_=false;
    avx.geo_lat_arc_=0.0;
    avx.geo_long_arc_=0.0;
    avx.val_radius_arc_=0.0;

    for (child = elem->FirstChildElement(); child != 0; child = child->NextSiblingElement())
    {
        loginf  << "ACGXMLParser: parseAdbAvx: found Avx child '" << child->Value() << "' text '" << child->GetText() << "'" ;

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

        if (strcmp ("GbrUid", child->Value() ) == 0)
        {
            std::string grb_uid_mid = child->Attribute("mid");
            assert (grb_uid_mid.size() != 0);
            unsigned int grb_uid_mid_num = intFromString(grb_uid_mid, &ok);
            assert (ok);

            assert (strcmp ("txtName", child->FirstChildElement()->Value()) == 0);
            std::string gbr_uid_text_name = child->FirstChildElement()->GetText();
            assert (gbr_uid_text_name.size() != 0);

            avx.has_gbr_uid_=true;
            avx.gbr_mid_= grb_uid_mid_num;
            avx.gbr_txt_name_ = gbr_uid_text_name;
        }
        else if (strcmp ("codeType", child->Value() ) == 0)
            avx.code_type_ = child->GetText();
        else if (strcmp ("geoLat", child->Value() ) == 0)
        {
            std::string geo = child->GetText();
        }
        else if (strcmp ("geoLong", child->Value() ) == 0)
        {
            std::string geo = child->GetText();
        }
        else if (strcmp ("codeDatum", child->Value() ) == 0)
            avx.code_datum_ = child->GetText();
        else
            throw std::runtime_error (std::string ("ACGXMLParser: parseAdbAvx: unknown attribute ")+child->Value());
    }

//    assert (ast_uid.code_type_.size() != 0);
//    assert (ast_uid.code_id_.size() != 0);
//
//    adb.setAseUid(ast_uid);
}

void ACGXMLParser::parseAse (tinyxml2::XMLElement *ase_elem)
{
    loginf  << "ACGXMLParser: parseAse";
}

