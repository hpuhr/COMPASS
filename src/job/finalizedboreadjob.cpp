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
    logdbg << "FinalizeDBOReadJob: run: read_list size " << read_list_.getSize();
    started_ = true;

    std::vector <DBOVariable*> &variables = read_list_.getSet ();
    PropertyList properties = buffer_->properties();

    for (auto var_it : variables)
    {
        logdbg << "FinalizeDBOReadJob: run: variable " << var_it->name() << " has representation " << Utils::String::representationToString(var_it->representation());
        const DBTableColumn &column = var_it->currentDBColumn ();
        assert (properties.hasProperty(column.name()));
        const Property &property = properties.get(column.name());
        assert (property.dataType() == var_it->dataType());

        if (var_it->representation() != Utils::String::Representation::STANDARD) // do representation stuff
        {
            Utils::String::Representation rep = var_it->representation();
            logdbg << "FinalizeDBOReadJob: run: variable " << var_it->name() << ": setting string representation " << Utils::String::representationToString(rep);

            switch (property.dataType())
            {
            case PropertyDataType::BOOL:
            {
                buffer_->getBool (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::CHAR:
            {
                buffer_->getChar (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::UCHAR:
            {
                buffer_->getUChar (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::INT:
            {
                buffer_->getInt (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::UINT:
            {
                buffer_->getUInt (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::LONGINT:
            {
                buffer_->getLongInt (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::ULONGINT:
            {
                buffer_->getULongInt (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::FLOAT:
            {
                buffer_->getFloat (property.name()).representation(rep);

                break;
            }
            case PropertyDataType::DOUBLE:
            {
                buffer_->getDouble (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::STRING:
                logerr << "FinalizeDBOReadJob: run: string representation for string variable " << var_it->name() << " impossible";
                break;
            default:
                logerr  <<  "FinalizeDBOReadJob: run: unknown property type " << Property::asString(property.dataType());
                throw std::runtime_error ("FinalizeDBOReadJob: run: unknown property type "+Property::asString(property.dataType()));
            }

        }

        if (column.dimension() != var_it->dimension())
            logwrn << "FinalizeDBOReadJob: run: variable " << var_it->name() << " has differing dimensions " << column.dimension() << " " << var_it->dimension();
        else if (column.unit() != var_it->unit()) // do unit conversion stuff
        {
            logdbg << "FinalizeDBOReadJob: run: variable " << var_it->name() << " of same dimension has different units " << column.unit() << " " << var_it->unit();

            const Dimension &dimension = UnitManager::instance().dimension (var_it->dimension());
            double factor = dimension.getFactor (column.unit(), var_it->unit());
            logdbg  << "FinalizeDBOReadJob: run: correct unit transformation with factor " << factor;

            switch (property.dataType())
            {
            case PropertyDataType::BOOL:
            {
                ArrayListTemplate<bool> &array_list = buffer_->getBool (property.name());
                logwrn << "FinalizeDBOReadJob: run: double multiplication of boolean variable " << var_it->name();
                array_list *= factor;
                break;
            }
            case PropertyDataType::CHAR:
            {
                ArrayListTemplate<char> &array_list = buffer_->getChar (property.name());
                logwrn << "FinalizeDBOReadJob: run: double multiplication of char variable " << var_it->name();
                array_list *= factor;
                break;
            }
            case PropertyDataType::UCHAR:
            {
                ArrayListTemplate<unsigned char> &array_list = buffer_->getUChar (property.name());
                logwrn << "FinalizeDBOReadJob: run: double multiplication of unsigned char variable " << var_it->name();
                array_list *= factor;
                break;
            }
            case PropertyDataType::INT:
            {
                ArrayListTemplate<int> &array_list = buffer_->getInt (property.name());
                array_list *= factor;
                break;
            }
            case PropertyDataType::UINT:
            {
                ArrayListTemplate<unsigned int> &array_list = buffer_->getUInt (property.name());
                array_list *= factor;
                break;
            }
            case PropertyDataType::LONGINT:
            {
                ArrayListTemplate<long> &array_list = buffer_->getLongInt(property.name());
                array_list *= factor;
                break;
            }
            case PropertyDataType::ULONGINT:
            {
                ArrayListTemplate<unsigned long> &array_list = buffer_->getULongInt(property.name());
                array_list *= factor;
                break;
            }
            case PropertyDataType::FLOAT:
            {
                ArrayListTemplate<float> &array_list = buffer_->getFloat (property.name());
                array_list *= factor;
                break;
            }
            case PropertyDataType::DOUBLE:
            {
                ArrayListTemplate<double> &array_list = buffer_->getDouble (property.name());
                array_list *= factor;
                break;
            }
            case PropertyDataType::STRING:
                logerr << "FinalizeDBOReadJob: run: unit transformation for string variable " << var_it->name() << " impossible";
                break;
            default:
                logerr  <<  "FinalizeDBOReadJob: run: unknown property type " << Property::asString(property.dataType());
                throw std::runtime_error ("FinalizeDBOReadJob: run: unknown property type "+Property::asString(property.dataType()));
            }
        }

        // rename to reflect dbo variable
        switch (property.dataType())
        {
        case PropertyDataType::BOOL:
        {
            buffer_->renameBool (property.name(), var_it->name());
            break;
        }
        case PropertyDataType::CHAR:
        {
            buffer_->renameChar (property.name(), var_it->name());
            break;
        }
        case PropertyDataType::UCHAR:
        {
            buffer_->renameUChar (property.name(), var_it->name());
            break;
        }
        case PropertyDataType::INT:
        {
            buffer_->renameInt (property.name(), var_it->name());
            break;
        }
        case PropertyDataType::UINT:
        {
            buffer_->renameUInt (property.name(), var_it->name());
            break;
        }
        case PropertyDataType::LONGINT:
        {
            buffer_->renameLongInt (property.name(), var_it->name());
            break;
        }
        case PropertyDataType::ULONGINT:
        {
            buffer_->renameULongInt (property.name(), var_it->name());
            break;
        }
        case PropertyDataType::FLOAT:
        {
            buffer_->renameFloat (property.name(), var_it->name());
            break;
        }
        case PropertyDataType::DOUBLE:
        {
            buffer_->renameDouble (property.name(), var_it->name());
            break;
        }
        case PropertyDataType::STRING:
        {
            buffer_->renameString (property.name(), var_it->name());
            break;
        }
        default:
            logerr  <<  "FinalizeDBOReadJob: run: unknown property type " << Property::asString(property.dataType());
            throw std::runtime_error ("FinalizeDBOReadJob: run: unknown property type "+Property::asString(property.dataType()));
        }

    }

    logdbg << "FinalizeDBOReadJob: run: done";
    done_=true;
}

