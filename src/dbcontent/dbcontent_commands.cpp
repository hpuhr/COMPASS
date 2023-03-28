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

#include "dbcontent_commands.h"

#include "rtcommand/rtcommand_macros.h"

#include <boost/program_options.hpp>

namespace dbContent
{

/***************************************************************************************
 * RTCommandGetTable
 ***************************************************************************************/

/**
 */
bool RTCommandGetTable::run_impl() const
{
    return false;
}

/**
 */
void RTCommandGetTable::collectOptions_impl(OptionsDescription &options,
                                            PosOptionsDescription &positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("dbcontent", po::value<std::string>()->required(), "DBContent to retrieve data for")
        ("utn"      , po::value<unsigned int>()           , "UTN to retrieve data for"      )
        ("variables", po::value<std::string>()->required(), "variables to retrieve data for");

    ADD_RTCOMMAND_POS_OPTION(positional, "dbcontent", 1)
    ADD_RTCOMMAND_POS_OPTION(positional, "variables", 2)
    ADD_RTCOMMAND_POS_OPTION(positional, "utn"      , 3)
}

/**
 */
void RTCommandGetTable::assignVariables_impl(const VariablesMap &vars)
{
    RTCOMMAND_GET_VAR_OR_THROW(vars, "dbcontent", std::string, dbcontent)
    RTCOMMAND_GET_VAR(vars, "utn", unsigned int, utn)
    RTCOMMAND_GET_STRINGLIST_OR_THROW(vars, "variables", variables)
}

} // namespace dbContent
