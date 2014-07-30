/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * AirspaceSectorManager.cpp
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#include "AirspaceSectorManager.h"
#include "AirspaceSector.h"

#include "gdal_priv.h"
#include "ogr_geometry.h"
#include "ogrsf_frmts.h"
#include "ogr_api.h"

#include <algorithm>
#include "String.h"

using namespace Utils::String;
AirspaceSectorManager::AirspaceSectorManager()
: Configurable ("AirspaceSectorManager", "AirspaceSectorManager0", 0, "conf/config_airspace.xml")
{
    createSubConfigurables ();

    createAllSectorsFlat();

    GDALAllRegister();
    OGRRegisterAll();
}

AirspaceSectorManager::~AirspaceSectorManager()
{
    std::map <std::string, AirspaceSector*>::iterator it;
    for (it = sectors_.begin(); it != sectors_.end(); it++)
        delete it->second;
    sectors_.clear();
}

void AirspaceSectorManager::generateSubConfigurable (std::string class_id, std::string instance_id)
{
    if (class_id.compare ("AirspaceSector") == 0)
    {
        AirspaceSector *sector = new AirspaceSector (class_id, instance_id, this);
        assert (sectors_.find(sector->getName()) == sectors_.end());
        sectors_[sector->getName()]=sector;
    }
    else
        throw std::runtime_error ("AirspaceSectorManager: generateSubConfigurable: unknown class_id "+class_id );
}
void AirspaceSectorManager::checkSubConfigurables ()
{
}

void AirspaceSectorManager::addNewSector (std::string name)
{
    Configuration &configuration = addNewSubConfiguration ("AirspaceSector");
    configuration.addParameterString ("name", name);
    generateSubConfigurable(configuration.getClassId(), configuration.getInstanceId());
}

void AirspaceSectorManager::rebuildSectorNames ()
{
    std::map <std::string, AirspaceSector*> old_sectors = sectors_;
    sectors_.clear();

    std::map <std::string, AirspaceSector*>::iterator it;

    for (it = old_sectors.begin(); it != old_sectors.end(); it++)
    {
        assert (sectors_.find(it->second->getName()) == sectors_.end());
        sectors_[it->second->getName()] = it->second;
    }
}

bool AirspaceSectorManager::deleteSectorIfPossible (AirspaceSector *sector)
{
    assert (sector);
    if (sectors_.find(sector->getName()) == sectors_.end())
        return false;

    sectors_.erase (sectors_.find(sector->getName()));
    delete sector;
    return true;
}

void AirspaceSectorManager::removeSector (AirspaceSector *sector)
{
    assert (sectors_.find(sector->getName()) != sectors_.end());
    sectors_.erase (sectors_.find(sector->getName()));
}

void AirspaceSectorManager::createNewSectorFromShapefile (std::string path)
{
    loginf << "AirspaceSectorManager: createNewSectorFromShapefile: path " << path;

    OGRDataSource *ogr_dataset = OGRSFDriverRegistrar::Open (path.c_str());

    if( !ogr_dataset )
        throw std::runtime_error ("AirspaceSectorManager: createNewSectorFromShapefile: unable to open");

    std::string idstd;
    OGRLayer* layer;
    OGRFeature* feature;
    OGRGeometry* geometry;

    int i, n = ogr_dataset->GetLayerCount();
    int count=0;

    for( i=0; i<n; ++i )
    {
        layer = ogr_dataset->GetLayer( i );

        loginf << "AirspaceSectorManager: createNewSectorFromShapefile: got layer " << i;

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

            loginf <<"AirspaceSectorManager: createNewSectorFromShapefile: name " << name << " min_alt " << min_alt
                    << " max_alt " << max_alt << " min_agl " << min_agl;


            geometry = feature->GetGeometryRef();
            std::string featurename = feature->GetDefnRef()->GetName();

            loginf << "AirspaceSectorManager: createNewSectorFromShapefile: got feature " << count << " " << featurename;

            if( geometry == NULL )
                continue;

            name.erase (std::remove(name.begin(), name.end(), ' '), name.end());

            addNewSector (name);
            assert (sectors_.find(name) != sectors_.end());
            AirspaceSector *sector = sectors_[name];
            sector->setHeightMin(min_alt);
            sector->setHeightMax(max_alt);

            if( wkbFlatten(geometry->getGeometryType()) == wkbPoint )
            {
                loginf << "AirspaceSectorManager:createNewSectorFromShapefile: wkbPoint";
                addPoint( (OGRPoint*)geometry, sector );
            }
            else if( wkbFlatten(geometry->getGeometryType()) ==  wkbLineString )
            {
                loginf << "AirspaceSectorManager:createNewSectorFromShapefile: wkbLineString";
                addLineString( (OGRLineString*)geometry, sector, true );
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbPolygon )
            {
                loginf << "AirspaceSectorManager:createNewSectorFromShapefile: wkbPolygon";
                addPoly( (OGRPolygon*)geometry, sector );
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiPoint )
            {
                loginf << "AirspaceSectorManager:createNewSectorFromShapefile: wkbMultiPoint";
                addMultiPoint( (OGRMultiPoint*)geometry, sector );
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiLineString )
            {
                loginf << "AirspaceSectorManager:createNewSectorFromShapefile: wkbMultiLineString";
                addMultiLineString( (OGRMultiLineString*)geometry, sector );
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiPolygon )
            {
                loginf << "AirspaceSectorManager:createNewSectorFromShapefile: wkbMultiPolygon";
                addMultiPoly( (OGRMultiPolygon*)geometry, sector );
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbGeometryCollection )
            {
                logwrn  << "AirspaceSectorManager:createNewSectorFromShapefile: wkbGeometryCollection not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbLinearRing )
            {
                addLinearRing( (OGRLinearRing*)geometry, sector, true );
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbPoint25D )
            {
                logwrn  << "AirspaceSectorManager: createNewSectorFromShapefile: wkbPoint25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbLineString25D )
            {
                logwrn  << "AirspaceSectorManager: createNewSectorFromShapefile: wkbLineString25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbPolygon25D )
            {
                logwrn  << "AirspaceSectorManager: createNewSectorFromShapefile: wkbPolygon25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiPoint25D )
            {
                logwrn  << "AirspaceSectorManager: createNewSectorFromShapefile: wkbMultiPoint25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiLineString25D )
            {
                logwrn  << "AirspaceSectorManager: createNewSectorFromShapefile: wkbMultiLineString25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbMultiPolygon25D )
            {
                logwrn  << "AirspaceSectorManager: createNewSectorFromShapefile: wkbMultiPolygon25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbGeometryCollection25D )
            {
                logwrn  << "AirspaceSectorManager: createNewSectorFromShapefile: wkbGeometryCollection25D not yet supported";
            }
            else if( wkbFlatten(geometry->getGeometryType()) == wkbNone )
            {
                continue;
            }
            else
            {
                logdbg  << "AirspaceSectorManager::createLayerOGR(): Unknown geometry!";
            }
            count++;
        }
    }
}

void AirspaceSectorManager::addLineString( const OGRLineString* obj, AirspaceSector* sector, bool new_line )
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
void AirspaceSectorManager::addMultiLineString( const OGRMultiLineString* obj, AirspaceSector* sector )
{
//    int i, n = obj->getNumGeometries();
//    for( i=0; i<n; ++i )
//        addLineString( (OGRLineString*)(obj->getGeometryRef( i )), lines, false );
}

/*
*/
void AirspaceSectorManager::addLinearRing( const OGRLinearRing* obj, AirspaceSector* sector, bool new_line )
{
    double x, y, z = 0.0;
    OGRPoint pt;
    int i, n = obj->getNumPoints();
    for( i=0; i<n; ++i )
    {
        obj->getPoint( i, &pt );
        OGR_G_GetPoint( &pt, 0, &x, &y, &z );
        //loginf << "AirspaceSectorManager: addLinearRing: " << x << ", " << y << ", " << z;
        sector->addPoint(y,x);
    }
}

/*
*/
void AirspaceSectorManager::addPoly( OGRPolygon* obj, AirspaceSector* sector )
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
void AirspaceSectorManager::addMultiPoly( OGRMultiPolygon* obj, AirspaceSector* sector )
{
//    int i, n = obj->getNumGeometries();
//    for( i=0; i<n; ++i )
//        addPoly( (OGRPolygon*)(obj->getGeometryRef( i )), lines );
}

/*
*/
void AirspaceSectorManager::addPoint( OGRPoint* obj, AirspaceSector* sector )
{
//    double x, y, z = 0.0;
//    OGR_G_GetPoint( obj, 0, &x, &y, &z );
//    transform( x, y, z );
//    points->addPoint( -1, -1, x, z, y, col_ );
}

/*
*/
void AirspaceSectorManager::addMultiPoint( OGRMultiPoint* obj, AirspaceSector* sector )
{
//    int i, n = obj->getNumGeometries();
//    for( i=0; i<n; ++i )
//        addPoint( (OGRPoint*)(obj->getGeometryRef( i )), points );
}

void AirspaceSectorManager::createAllSectorsFlat ()
{
    all_sectors_flat_.clear();
    point_inside_sectors_.clear();

    std::map <std::string, AirspaceSector*>::iterator it;

    for (it = sectors_.begin(); it != sectors_.end(); it++)
    {
        it->second->addAllVolumeSectors(all_sectors_flat_);
    }

    for (unsigned int cnt=0; cnt < all_sectors_flat_.size(); cnt++)
    {
        std::string name = all_sectors_flat_.at(cnt)->getName();
        assert (point_inside_sectors_.find(name) == point_inside_sectors_.end());
        point_inside_sectors_[name]=false;    }
}

std::map <std::string, bool> &AirspaceSectorManager::isPointInsideSector (double latitude, double longitude, bool height_given, double height_ft,
        const std::map <std::string, bool> &old_insides, bool debug)
{
    assert (all_sectors_flat_.size() != 0);
    assert (all_sectors_flat_.size() == point_inside_sectors_.size());

    AirspaceSector *current_sector;

    bool was_inside_defined;
    bool was_inside;

    for (unsigned int cnt=0; cnt < all_sectors_flat_.size(); cnt++)
    {
        current_sector = all_sectors_flat_.at(cnt);
        assert (current_sector);
        std::string name = current_sector->getName();

        assert (point_inside_sectors_.find(name) != point_inside_sectors_.end());

        was_inside = false;
        was_inside_defined = old_insides.find (name) != old_insides.end();

        if (was_inside_defined)
            was_inside = old_insides.at(name);

//        if (debug)
//            loginf << "ASM: checking sector " << name << " was in sector " << ;

//        if (debug && (name == "ALL" || name == "En-Route"))
//        {
//            if (old_insides.find (name) == old_insides.end())
//                loginf << "ASM: checking sector " << name << " was in sector undefined";
//            else
//                loginf << "ASM: checking sector " << name << " was in sector " << old_insides.at(name);
//        }

        if (!current_sector->getUsedForChecking())
        {
            point_inside_sectors_[name] = false;

//            if (debug && (name == "ALL" || name == "En-Route"))
//                loginf << "ASM: checking sector " << name << " not used for checking; false";
        }
        else
        {
            if (height_given)
            {
                point_inside_sectors_[name] = current_sector->isPointInside(latitude, longitude, height_ft, false);

//                if (debug && (name == "ALL" || name == "En-Route"))
//                    loginf << "ASM: checking sector " << name << " height given, using height; " << point_inside_sectors_[name];
//
//                if (debug && was_inside_defined && was_inside && !point_inside_sectors_[name])
//                {
//                    loginf << "ASM: sector " << name << " change, debugging inside";
//                    bool tmp = current_sector->isPointInside(latitude, longitude, height_ft, true);
//                }
            }
            else
            {
                if (was_inside_defined)
                {
//                    bool was_inside = old_insides.at(name);
                    if (was_inside)
                    {
                        point_inside_sectors_[name] = current_sector->isPointInside(latitude, longitude, false);

//                        if (debug && (name == "ALL" || name == "En-Route"))
//                            loginf << "ASM: checking sector " << name << " height not given, was_inside " << was_inside << "; " << point_inside_sectors_[name];
                    }
                    else
                    {
                        point_inside_sectors_[name]=false;

//                        if (debug && (name == "ALL" || name == "En-Route"))
//                            loginf << "ASM: checking sector " << name << " height not given, was_inside not " << was_inside << "; false";
                    }
                }
                else
                {
                    point_inside_sectors_[name]=false; //TODO HACK
                    //logwrn << "AirspaceSectorManager: isPointInsideSector: cannot check on sector " << name << ", this should be handled";

//                    if (debug && (name == "ALL" || name == "En-Route"))
//                        loginf << "ASM: checking sector " << name << " inside unknown; false";

                }
            }
        }
    }

    return point_inside_sectors_;
}

