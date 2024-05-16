/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "property.h"

#define SwitchPropertyDataTypeEntry(Name, Type, Func) \
    case Name:                                        \
    {                                                 \
        Func(Name, Type)                              \
        break;                                        \
    }

#define SwitchPropertyDataType(Var, PTypeFunc, DefaultFunc)                                               \
        switch(Var)                                                                                       \
        {                                                                                                 \
            SwitchPropertyDataTypeEntry(PropertyDataType::BOOL     , bool                    , PTypeFunc) \
            SwitchPropertyDataTypeEntry(PropertyDataType::CHAR     , char                    , PTypeFunc) \
            SwitchPropertyDataTypeEntry(PropertyDataType::UCHAR    , unsigned char           , PTypeFunc) \
            SwitchPropertyDataTypeEntry(PropertyDataType::INT      , int                     , PTypeFunc) \
            SwitchPropertyDataTypeEntry(PropertyDataType::UINT     , unsigned int            , PTypeFunc) \
            SwitchPropertyDataTypeEntry(PropertyDataType::LONGINT  , long int                , PTypeFunc) \
            SwitchPropertyDataTypeEntry(PropertyDataType::ULONGINT , unsigned long int       , PTypeFunc) \
            SwitchPropertyDataTypeEntry(PropertyDataType::FLOAT    , float                   , PTypeFunc) \
            SwitchPropertyDataTypeEntry(PropertyDataType::DOUBLE   , double                  , PTypeFunc) \
            SwitchPropertyDataTypeEntry(PropertyDataType::STRING   , std::string             , PTypeFunc) \
            SwitchPropertyDataTypeEntry(PropertyDataType::JSON     , nlohmann::json          , PTypeFunc) \
            SwitchPropertyDataTypeEntry(PropertyDataType::TIMESTAMP, boost::posix_time::ptime, PTypeFunc) \
            default:                                                                                      \
            {                                                                                             \
                DefaultFunc                                                                               \
                break;                                                                                    \
            }                                                                                             \
        }
        
#define SwitchPropertyDataTypeNumeric(Var, PTypeFunc, StringFunc, JSONFunc, DefaultFunc)                   \
        switch(Var)                                                                                        \
        {                                                                                                  \
            SwitchPropertyDataTypeEntry(PropertyDataType::BOOL     , bool                    , PTypeFunc ) \
            SwitchPropertyDataTypeEntry(PropertyDataType::CHAR     , char                    , PTypeFunc ) \
            SwitchPropertyDataTypeEntry(PropertyDataType::UCHAR    , unsigned char           , PTypeFunc ) \
            SwitchPropertyDataTypeEntry(PropertyDataType::INT      , int                     , PTypeFunc ) \
            SwitchPropertyDataTypeEntry(PropertyDataType::UINT     , unsigned int            , PTypeFunc ) \
            SwitchPropertyDataTypeEntry(PropertyDataType::LONGINT  , long int                , PTypeFunc ) \
            SwitchPropertyDataTypeEntry(PropertyDataType::ULONGINT , unsigned long int       , PTypeFunc ) \
            SwitchPropertyDataTypeEntry(PropertyDataType::FLOAT    , float                   , PTypeFunc ) \
            SwitchPropertyDataTypeEntry(PropertyDataType::DOUBLE   , double                  , PTypeFunc ) \
            SwitchPropertyDataTypeEntry(PropertyDataType::STRING   , std::string             , StringFunc) \
            SwitchPropertyDataTypeEntry(PropertyDataType::JSON     , nlohmann::json          , JSONFunc  ) \
            SwitchPropertyDataTypeEntry(PropertyDataType::TIMESTAMP, boost::posix_time::ptime, PTypeFunc ) \
            default:                                                                                       \
            {                                                                                              \
                DefaultFunc                                                                                \
                break;                                                                                     \
            }                                                                                              \
        }

namespace property_templates
{

/**
*/
template <typename TFunc>
bool invokeFunctor(PropertyDataType dtype, TFunc& func)
{
    switch (dtype)
    {
    case PropertyDataType::BOOL:
        return func.template operator()<bool, PropertyDataType::BOOL>();
    case PropertyDataType::CHAR:
        return func.template operator()<char, PropertyDataType::CHAR>();
    case PropertyDataType::UCHAR:
        return func.template operator()<unsigned char, PropertyDataType::UCHAR>();
    case PropertyDataType::INT:
        return func.template operator()<int, PropertyDataType::INT>();
    case PropertyDataType::UINT:
        return func.template operator()<unsigned int, PropertyDataType::UINT>();
    case PropertyDataType::LONGINT:
        return func.template operator()<long int, PropertyDataType::LONGINT>();
    case PropertyDataType::ULONGINT:
        return func.template operator()<unsigned long int, PropertyDataType::ULONGINT>();
    case PropertyDataType::FLOAT:
        return func.template operator()<float, PropertyDataType::FLOAT>();
    case PropertyDataType::DOUBLE:
        return func.template operator()<double, PropertyDataType::DOUBLE>();
    case PropertyDataType::TIMESTAMP:
        return func.template operator()<boost::posix_time::ptime, PropertyDataType::TIMESTAMP>();
    case PropertyDataType::STRING:
        return func.template operator()<std::string, PropertyDataType::STRING>();
    case PropertyDataType::JSON:
        return func.template operator()<nlohmann::json, PropertyDataType::JSON>();
    default:
        func.error(dtype);
    }
    return false;
}

} // property_templates
