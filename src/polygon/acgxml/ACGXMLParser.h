/*
 * ACGXMLParser.h
 *
 *  Created on: Aug 11, 2014
 *      Author: sk
 */

#ifndef ACGXMLPARSER_H_
#define ACGXMLPARSER_H_

#include "AirspaceSectorParser.h"

class ACGXMLParser : public AirspaceSectorParser
{
public:
    ACGXMLParser();
    virtual ~ACGXMLParser();

    virtual void parse (std::string filename);
};

#endif /* ACGXMLPARSER_H_ */
