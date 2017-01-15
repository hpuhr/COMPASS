/*
 * ACGXMLParser.h
 *
 *  Created on: Aug 11, 2014
 *      Author: sk
 */

#ifndef ACGXMLPARSER_H_
#define ACGXMLPARSER_H_

#include "AirspaceSectorParser.h"
#include <tinyxml2.h>
#include <map>

namespace ACGXML
{
    class Abd;
    class Ase;
}

class AirspaceSector;

class ACGXMLParser : public AirspaceSectorParser
{
public:
    ACGXMLParser();
    virtual ~ACGXMLParser();

    void parse (std::string filename, AirspaceSector *base_sector);

protected:
    std::map <unsigned int, ACGXML::Abd> abds_;
    std::map <unsigned int, ACGXML::Ase> ases_;

    void clear();

    void parseAdb (tinyxml2::XMLElement *adb_elem);
    void parseAdbAseUid (tinyxml2::XMLElement *elem, ACGXML::Abd &adb);
    void parseAdbAvx (tinyxml2::XMLElement *elem, ACGXML::Abd &adb);

    void parseAse (tinyxml2::XMLElement *ase_elem);

    void checkConistency ();
    void createSectors (AirspaceSector *base_sector);

    double getHeight (double value, std::string dist);
};

#endif /* ACGXMLPARSER_H_ */
