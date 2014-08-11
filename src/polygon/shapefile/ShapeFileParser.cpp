/*
 * ShapeFileParser.cpp
 *
 *  Created on: Aug 11, 2014
 *      Author: sk
 */

#include "ShapeFileParser.h"
#include "Logger.h"
#include "String.h"
#include "AirspaceSectorManager.h"
#include "AirspaceSector.h"

#include <algorithm>
#include "gdal_priv.h"
#include "ogr_geometry.h"
#include "ogrsf_frmts.h"
#include "ogr_api.h"

using namespace Utils::String;

ShapeFileParser::ShapeFileParser()
 : AirspaceSectorParser()
{
    GDALAllRegister();
    OGRRegisterAll();
}

ShapeFileParser::~ShapeFileParser()
{
}

void ShapeFileParser::parse (std::string filename)
{
    OGRDataSource *ogr_dataset = OGRSFDriverRegistrar::Open (filename.c_str());

    if( !ogr_dataset )
        throw std::runtime_error ("ShapeFileParser: parse: unable to open");

    std::string idstd;
    OGRLayer* layer;
    OGRFeature* feature;
    OGRGeometry* geometry;

    int i, n = ogr_dataset->GetLayerCount();
    int count=0;

    for( i=0; i<n; ++i )
    {
        layer = ogr_dataset->GetLayer( i );

        loginf << "ShapeFileParser: parse: got layer " << i;

        //id = QString::number( i );
        idstd = intToString(i);//id.toStdString();

        //set correct transform
//        updateGeometryTransform( layer );

//        DOPoints* points = new DOPoints( layer_manager_->getDOManager(), NULL );
//        DOLines* lines = new DOLines( layer_manager_->getDOManager(), NULL );
//
//        DOLayer* dolayer = layer_manager_->newLayer( idstd, idstd );
//        layer_manager_->addSubLayer( dolayer, parent );
//
//        parent->getSubLayer( idstd )->addDisplayObject( points );
//        parent->getSubLayer( idstd )->addDisplayObject( lines );



        layer->ResetReading();
        while( ( feature = layer->GetNextFeature() ) != NULL )
        {
            OGRFeatureDefn *poFDefn = layer->GetLayerDefn();
            int iField;

            std::string name;
            double min_alt=-1;
            double max_alt=-1;
            double min_agl=-1;

            for( iField = 0; iField < poFDefn->GetFieldCount(); iField++ )
            {
                OGRFieldDefn *poFieldDefn = poFDefn->GetFieldDefn( iField );

//                if( poFieldDefn->GetType() == OFTInteger )
//                    printf( "%d,", feature->GetFieldAsInteger( iField ) );
//                else if( poFieldDefn->GetType() == OFTReal )
//                    printf( "%.3f,", feature->GetFieldAsDouble(iField) );
//                else if( poFieldDefn->GetType() == OFTString )
//                    printf( "%s,", feature->GetFieldAsString(iField) );
//                else
//                    printf( "%s,", feature->GetFieldAsString(iField) );

                std::string fieldname = poFieldDefn->GetNameRef();

                if (fieldname == "Name")
                    name = feature->GetFieldAsString(iField);
                else if (fieldname == "min_alt")
                    min_alt = feature->GetFieldAsDouble(iField);
                else if (fieldname == "max_alt")
                    max_alt = feature->GetFieldAsDouble(iField);
                else if (fieldname == "min_agl")
                    min_agl = feature->GetFieldAsDouble(iField);
            }

            if (name.size() == 0)
                name = "Layer"+intToString(i);

            loginf <<"ShapeFileParser: parse: name " << name << " min_alt " << min_alt
                    << " max_alt " << max_alt << " min_agl " << min_agl;


            geometry = feature->GetGeometryRef();
            std::string featurename = feature->GetDefnRef()->GetName();

            loginf << "ShapeFileParser: parse: got feature " << count << " " << featurename;

            if( geometry == NULL )
                continue;

            name.erase (std::remove(name.begin(), name.end(), ' '), name.end());

            if (AirspaceSectorManager::getInstance().hasSector(name))
            {
                logwrn << "ShapeFileParser: parse: airspace sector '" << name << "' already exists, deleting previous";
                bool ret = AirspaceSectorManager::getInstance().deleteSectorIfPossible(name);
                assert (ret);
                assert (!AirspaceSectorManager::getInstance().hasSector(name));
            }

            AirspaceSectorManager::getInstance().addNewSector (name);
            assert (AirspaceSectorManager::getInstance().hasSector(name));
            AirspaceSector *sector = AirspaceSectorManager::getInstance().getSector(name);
            sector->setHeightMin(min_alt);
            sector->setHeightMax(max_alt);

            if( wkbFlatten(geometry->getGeometryType()) == wkbPoint )
            {
                loginf << "ShapeFileParser:parse: wkbPoint";
                addPoint( (OGRPoint*)geometry, sector );
            }
            else if( wkbFlatten(geometry->getGeometryType()) ==  wkbLineString )
            {
                loginf << "ShapeFileParser:parse: wkbLineString";
                addLineString( (OGRLineString*)geometry, sector, true );
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbPolygon )
            {
                loginf << "ShapeFileParser:parse: wkbPolygon";
                addPoly( (OGRPolygon*)geometry, sector );
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiPoint )
            {
                loginf << "ShapeFileParser:parse: wkbMultiPoint";
                addMultiPoint( (OGRMultiPoint*)geometry, sector );
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiLineString )
            {
                loginf << "ShapeFileParser:parse: wkbMultiLineString";
                addMultiLineString( (OGRMultiLineString*)geometry, sector );
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiPolygon )
            {
                loginf << "ShapeFileParser:parse: wkbMultiPolygon";
                addMultiPoly( (OGRMultiPolygon*)geometry, sector );
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbGeometryCollection )
            {
                logwrn  << "ShapeFileParser:parse: wkbGeometryCollection not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbLinearRing )
            {
                addLinearRing( (OGRLinearRing*)geometry, sector, true );
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbPoint25D )
            {
                logwrn  << "ShapeFileParser: parse: wkbPoint25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbLineString25D )
            {
                logwrn  << "ShapeFileParser: parse: wkbLineString25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbPolygon25D )
            {
                logwrn  << "ShapeFileParser: parse: wkbPolygon25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiPoint25D )
            {
                logwrn  << "ShapeFileParser: parse: wkbMultiPoint25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiLineString25D )
            {
                logwrn  << "ShapeFileParser: parse: wkbMultiLineString25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiPolygon25D )
            {
                logwrn  << "ShapeFileParser: parse: wkbMultiPolygon25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbGeometryCollection25D )
            {
                logwrn  << "ShapeFileParser: parse: wkbGeometryCollection25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbNone )
            {
                continue;
            }
            else
            {
                logdbg  << "ShapeFileParser::createLayerOGR(): Unknown geometry!";
            }
            count++;
        }
    }

}

void ShapeFileParser::addLineString( const OGRLineString* obj, AirspaceSector* sector, bool new_line )
{
//    if( new_line )
//        lines->startLines( col_ );
//
//    int closed = obj->get_IsClosed();
//
//    OGRPoint pt;
//    Ogre::Vector3 last, coord, first;
//    double x, y, z = 0.0;
//    int i, n = obj->getNumPoints();
//    for( i=0; i<n; ++i )
//    {
//        obj->getPoint( i, &pt );
//        OGR_G_GetPoint( &pt, 0, &x, &y, &z );
//        transform( x, y, z );
//        coord[ 0 ] = x;
//        coord[ 1 ] = z;
//        coord[ 2 ] = y;
//        if( i > 0 )
//            lines->addLine( last, coord );
//        else
//            first = coord;
//        last = coord;
//    }
//    if( closed )
//        lines->addLine( last, first );
//
//    if( new_line )
//        lines->endLines();
}

/*
*/
void ShapeFileParser::addMultiLineString( const OGRMultiLineString* obj, AirspaceSector* sector )
{
//    int i, n = obj->getNumGeometries();
//    for( i=0; i<n; ++i )
//        addLineString( (OGRLineString*)(obj->getGeometryRef( i )), lines, false );
}

/*
*/
void ShapeFileParser::addLinearRing( const OGRLinearRing* obj, AirspaceSector* sector, bool new_line )
{
    double x, y, z = 0.0;
    OGRPoint pt;
    int i, n = obj->getNumPoints();
    for( i=0; i<n; ++i )
    {
        obj->getPoint( i, &pt );
        OGR_G_GetPoint( &pt, 0, &x, &y, &z );
        //loginf << "ShapeFileParser: addLinearRing: " << x << ", " << y << ", " << z;
        sector->addPoint(y,x);
    }
}

/*
*/
void ShapeFileParser::addPoly( OGRPolygon* obj, AirspaceSector* sector )
{
    std::string name = obj->getGeometryName();
    int i, n = obj->getNumInteriorRings();
    for( i=0; i<n; ++i )
    {

        AirspaceSector *sub_sector = sector->addNewSubSector(name+"_"+intToString(i));

        const OGRLinearRing* ring = obj->getInteriorRing( i );
        addLinearRing( ring, sub_sector, false );
    }

    //AirspaceSector *sub_sector = sector->addNewSubSector(name+"_"+intToString(i));
    const OGRLinearRing* ring = obj->getExteriorRing();
    addLinearRing( ring, sector, false );
}

/*
*/
void ShapeFileParser::addMultiPoly( OGRMultiPolygon* obj, AirspaceSector* sector )
{
//    int i, n = obj->getNumGeometries();
//    for( i=0; i<n; ++i )
//        addPoly( (OGRPolygon*)(obj->getGeometryRef( i )), lines );
}

/*
*/
void ShapeFileParser::addPoint( OGRPoint* obj, AirspaceSector* sector )
{
//    double x, y, z = 0.0;
//    OGR_G_GetPoint( obj, 0, &x, &y, &z );
//    transform( x, y, z );
//    points->addPoint( -1, -1, x, z, y, col_ );
}

/*
*/
void ShapeFileParser::addMultiPoint( OGRMultiPoint* obj, AirspaceSector* sector )
{
//    int i, n = obj->getNumGeometries();
//    for( i=0; i<n; ++i )
//        addPoint( (OGRPoint*)(obj->getGeometryRef( i )), points );
}
