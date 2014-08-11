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

class ACGXMLParser : public AirspaceSectorParser
{
public:
    ACGXMLParser();
    virtual ~ACGXMLParser();

    virtual void parse (std::string filename);

protected:
    std::map <unsigned int, ACGXML::Abd> abds_;
    std::map <unsigned int, ACGXML::Ase> ases_;

    void clear();

    void parseAdb (tinyxml2::XMLElement *adb_elem);
    void parseAdbAseUid (tinyxml2::XMLElement *elem, ACGXML::Abd &adb);
    void parseAdbAvx (tinyxml2::XMLElement *elem, ACGXML::Abd &adb);

    void parseAse (tinyxml2::XMLElement *ase_elem);
};

#endif /* ACGXMLPARSER_H_ */
