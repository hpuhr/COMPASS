#include "data.h"

#include "dbovariableset.h"
#include "buffer.h"
#include "dbovariable.h"
#include "dbtablecolumn.h"
#include "stringconv.h"
#include "unitmanager.h"
#include "unit.h"
#include "propertylist.h"

namespace Utils
{

namespace Data
{

void finalizeBuffer (DBOVariableSet &read_list, std::shared_ptr<Buffer> buffer)
{
    std::vector <DBOVariable*> &variables = read_list.getSet ();
    PropertyList properties = buffer->properties();

    for (auto var_it : variables)
    {
        logdbg << "Data: finalizeBuffer: variable " << var_it->name() << " has representation " << Utils::String::representationToString(var_it->representation());
        const DBTableColumn &column = var_it->currentDBColumn ();
        assert (properties.hasProperty(column.name()));
        const Property &property = properties.get(column.name());
        assert (property.dataType() == var_it->dataType());

        if (var_it->representation() != Utils::String::Representation::STANDARD) // do representation stuff
        {
            Utils::String::Representation rep = var_it->representation();
            logdbg << "Data: finalizeBuffer: variable " << var_it->name() << ": setting string representation " << Utils::String::representationToString(rep);

            switch (property.dataType())
            {
            case PropertyDataType::BOOL:
            {
                buffer->getBool (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::CHAR:
            {
                buffer->getChar (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::UCHAR:
            {
                buffer->getUChar (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::INT:
            {
                buffer->getInt (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::UINT:
            {
                buffer->getUInt (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::LONGINT:
            {
                buffer->getLongInt (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::ULONGINT:
            {
                buffer->getULongInt (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::FLOAT:
            {
                buffer->getFloat (property.name()).representation(rep);

                break;
            }
            case PropertyDataType::DOUBLE:
            {
                buffer->getDouble (property.name()).representation(rep);
                break;
            }
            case PropertyDataType::STRING:
                logerr << "Data: finalizeBuffer: string representation for string variable " << var_it->name() << " impossible";
                break;
            default:
                logerr  <<  "Data: finalizeBuffer: unknown property type " << Property::asString(property.dataType());
                throw std::runtime_error ("Data: finalizeBuffer: unknown property type "+Property::asString(property.dataType()));
            }

        }

        if (column.dimension() != var_it->dimension())
            logwrn << "Data: finalizeBuffer: variable " << var_it->name() << " has differing dimensions " << column.dimension() << " " << var_it->dimension();
        else if (column.unit() != var_it->unit()) // do unit conversion stuff
        {
            logdbg << "Data: finalizeBuffer: variable " << var_it->name() << " of same dimension has different units " << column.unit() << " " << var_it->unit();

            const Dimension &dimension = UnitManager::instance().dimension (var_it->dimension());
            double factor = dimension.getFactor (column.unit(), var_it->unit());
            logdbg  << "Data: finalizeBuffer: correct unit transformation with factor " << factor;

            switch (property.dataType())
            {
            case PropertyDataType::BOOL:
            {
                ArrayListTemplate<bool> &array_list = buffer->getBool (property.name());
                logwrn << "Data: finalizeBuffer: double multiplication of boolean variable " << var_it->name();
                array_list *= factor;
                break;
            }
            case PropertyDataType::CHAR:
            {
                ArrayListTemplate<char> &array_list = buffer->getChar (property.name());
                logwrn << "Data: finalizeBuffer: double multiplication of char variable " << var_it->name();
                array_list *= factor;
                break;
            }
            case PropertyDataType::UCHAR:
            {
                ArrayListTemplate<unsigned char> &array_list = buffer->getUChar (property.name());
                logwrn << "Data: finalizeBuffer: double multiplication of unsigned char variable " << var_it->name();
                array_list *= factor;
                break;
            }
            case PropertyDataType::INT:
            {
                ArrayListTemplate<int> &array_list = buffer->getInt (property.name());
                array_list *= factor;
                break;
            }
            case PropertyDataType::UINT:
            {
                ArrayListTemplate<unsigned int> &array_list = buffer->getUInt (property.name());
                array_list *= factor;
                break;
            }
            case PropertyDataType::LONGINT:
            {
                ArrayListTemplate<long> &array_list = buffer->getLongInt(property.name());
                array_list *= factor;
                break;
            }
            case PropertyDataType::ULONGINT:
            {
                ArrayListTemplate<unsigned long> &array_list = buffer->getULongInt(property.name());
                array_list *= factor;
                break;
            }
            case PropertyDataType::FLOAT:
            {
                ArrayListTemplate<float> &array_list = buffer->getFloat (property.name());
                array_list *= factor;
                break;
            }
            case PropertyDataType::DOUBLE:
            {
                ArrayListTemplate<double> &array_list = buffer->getDouble (property.name());
                array_list *= factor;
                break;
            }
            case PropertyDataType::STRING:
                logerr << "Data: finalizeBuffer: unit transformation for string variable " << var_it->name() << " impossible";
                break;
            default:
                logerr  <<  "Data: finalizeBuffer: unknown property type " << Property::asString(property.dataType());
                throw std::runtime_error ("Data: finalizeBuffer: unknown property type "+Property::asString(property.dataType()));
            }
        }

        // rename to reflect dbo variable
        if (property.name() != var_it->name())
        {
            //loginf << "Data: finalizeBuffer: renaming property " << property.name() << " to dbo variable name " << var_it->name();
            switch (property.dataType())
            {
            case PropertyDataType::BOOL:
            {
                buffer->renameBool (property.name(), var_it->name());
                break;
            }
            case PropertyDataType::CHAR:
            {
                buffer->renameChar (property.name(), var_it->name());
                break;
            }
            case PropertyDataType::UCHAR:
            {
                buffer->renameUChar (property.name(), var_it->name());
                break;
            }
            case PropertyDataType::INT:
            {
                buffer->renameInt (property.name(), var_it->name());
                break;
            }
            case PropertyDataType::UINT:
            {
                buffer->renameUInt (property.name(), var_it->name());
                break;
            }
            case PropertyDataType::LONGINT:
            {
                buffer->renameLongInt (property.name(), var_it->name());
                break;
            }
            case PropertyDataType::ULONGINT:
            {
                buffer->renameULongInt (property.name(), var_it->name());
                break;
            }
            case PropertyDataType::FLOAT:
            {
                buffer->renameFloat (property.name(), var_it->name());
                break;
            }
            case PropertyDataType::DOUBLE:
            {
                buffer->renameDouble (property.name(), var_it->name());
                break;
            }
            case PropertyDataType::STRING:
            {
                buffer->renameString (property.name(), var_it->name());
                break;
            }
            default:
                logerr  <<  "Data: finalizeBuffer: unknown property type " << Property::asString(property.dataType());
                throw std::runtime_error ("Data: finalizeBuffer: unknown property type "+Property::asString(property.dataType()));
            }
        }
    }
}

}
}
