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
 * FinalizeDBOReadJob.cpp
 *
 *  Created on: Feb 25, 2013
 *      Author: sk
 */

#include <QThread>

#include "finalizedboreadjob.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "propertylist.h"
#include "dbtablecolumn.h"
#include "dbovariableset.h"
#include "unitmanager.h"
#include "unit.h"
#include "buffer.h"

FinalizeDBOReadJob::FinalizeDBOReadJob(DBObject &dbobject, DBOVariableSet &read_list, std::shared_ptr<Buffer> buffer)
    : Job (), dbobject_(dbobject), read_list_(read_list), buffer_ (buffer)
{
    assert (buffer_);
}

FinalizeDBOReadJob::~FinalizeDBOReadJob()
{

}

void FinalizeDBOReadJob::run ()
{
    loginf << "FinalizeDBOReadJob: run";

    std::vector <DBOVariable*> &variables = read_list_.getSet ();
    const PropertyList &properties = buffer_->properties();

    for (auto var_it : variables)
    {
        const DBTableColumn &column = var_it->currentDBColumn ();
        assert (properties.hasProperty(column.name()));
        const Property &property = properties.get(column.name());
        assert (property.dataType() == var_it->dataType());
        if (column.dimension() != var_it->dimension())
            loginf << "FinalizeDBOReadJob UGA " << var_it->name() << " dimension " << column.dimension() << " " << var_it->dimension();
        if (column.unit() != var_it->unit())
            loginf << "FinalizeDBOReadJob UGA2 " << var_it->name() << " unit " << column.unit() << " " << var_it->unit();
    }


//    assert (type != DBO_UNDEFINED);

//    DBOVariableSet *type_set = read_list.getFor (type);
//    std::vector <DBOVariable*> &variables =type_set->getSet();
//    std::vector <DBOVariable*>::iterator it;

//    PropertyList *buffer_list = buffer->getPropertyList ();

//    logdbg << "Util: finalizeDBData: vars " << buffer_list->getNumProperties();

//    for (it = variables.begin(); it != variables.end(); it++)
//    {
//        DBOVariable *variable = *it;

//        assert (!variable->isMetaVariable());

//        std::string meta_tablename = variable->getCurrentMetaTable ();
//        std::string table_varname = variable->getCurrentVariableName ();

//        DBTableColumn *table_column = DBSchemaManager::getInstance().getCurrentSchema ()->getMetaTable(meta_tablename)->getTableColumn(table_varname);

//        logdbg  << "Util: finalizeDBData: type " << type << " variable " << table_column->getName();
//        if (!buffer_list->hasProperty(table_column->getName()))
//        {
//            logwrn  << "Util: finalizeDBData: variable information not synchronized";
//            continue;
//        }

//        unsigned col = buffer_list->getPropertyIndex(table_column->getName());
//        PROPERTY_DATA_TYPE data_type = (PROPERTY_DATA_TYPE) buffer->getPropertyList()->getProperty(col)->data_type_int_;

//        if (table_column->hasSpecialNull())
//        {
//            logdbg << "Util: finalizeDBData: setting special null '" << table_column->getSpecialNull()
//                   << "' col " << col;
//            setSpecialNullsNan (buffer, col, data_type, table_column->getSpecialNull());
//        }

//        if (variable->hasUnit () || table_column->hasUnit())
//        {
//            logdbg  << "Util: finalizeDBData: unit var type " << variable->getDBOType() << " dim '" << variable->getUnitDimension() << "'";
//            if (variable->hasUnit () != table_column->hasUnit())
//            {
//                logerr << "Util: finalizeDBData: unit transformation inconsistent: var " << variable->getName () << " has unit " << variable->hasUnit ()
//                       << " table column " << table_column->getName() << " has unit " << table_column->hasUnit();
//                continue;
//            }

//            if (variable->getUnitDimension().compare(table_column->getUnitDimension()) != 0)
//            {
//                logerr << "Util: finalizeDBData: unit transformation inconsistent: var " << variable->getName () << " has dimension " << variable->getUnitDimension ()
//                       << " table column " << table_column->getName() << " has dimension " << table_column->getUnitDimension();
//                continue;
//            }

//            Unit *unit = UnitManager::getInstance().getUnit (variable->getUnitDimension());
//            double factor = unit->getFactor (table_column->getUnitUnit(), variable->getUnitUnit());
//            logdbg  << "Util: finalizeDBData: correct unit transformation with factor " << factor;

//            buffer->setIndex( 0 );

//            std::vector<void*>* output_adresses;
//            unsigned int cnt;
//            unsigned n = buffer->getSize();

//            for( cnt=0; cnt<n; ++cnt )
//            {
//                if( cnt != 0 )
//                {
//                    assert (buffer);
//                    buffer->incrementIndex();
//                }

//                output_adresses = buffer->getAdresses();

//                if (isNan(data_type, output_adresses->at( col )))
//                    continue;

//                multiplyData(output_adresses->at( col ), data_type, factor);
//            }
//        }
//    }
//    delete type_set;

    done_=true;
}

