/*
 * ShapeFileParser.h
 *
 *  Created on: Aug 11, 2014
 *      Author: sk
 */

#ifndef SHAPEFILEPARSER_H_
#define SHAPEFILEPARSER_H_

#include "AirspaceSectorParser.h"

#include "gdal_priv.h"
#include "ogrsf_frmts.h"

class AirspaceSector;

class ShapeFileParser : public AirspaceSectorParser
{
public:
    ShapeFileParser();
    virtual ~ShapeFileParser();

    virtual void parse (std::string filename);

protected:
    void addLineString( const OGRLineString* obj, AirspaceSector* sector, bool new_line );
    void addMultiLineString( const OGRMultiLineString* obj, AirspaceSector* sector );
    void addLinearRing( const OGRLinearRing* obj, AirspaceSector* sector, bool new_line );
    void addPoly( OGRPolygon* obj, AirspaceSector* sector );
    void addMultiPoly( OGRMultiPolygon* obj, AirspaceSector* sector );
    void addPoint( OGRPoint* obj, AirspaceSector* sector );
    void addMultiPoint( OGRMultiPoint* obj, AirspaceSector* sector );
};

#endif /* SHAPEFILEPARSER_H_ */
