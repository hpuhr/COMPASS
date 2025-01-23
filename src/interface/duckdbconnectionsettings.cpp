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

#include "duckdbconnectionsettings.h"

#include <duckdb.h>

/**
 */
std::string DuckDBConnectionSettings::accessModeAsString(AccessMode mode)
{
    return mode == AccessMode::ReadOnly ? "READ_ONLY" : "READ_WRITE"; 
}

/**
 */
std::string DuckDBConnectionSettings::sortOrderAsString(SortOrder order)
{
    return order == SortOrder::Ascending ? "ASC" : "DESC"; 
}

/**
 */
void DuckDBConnectionSettings::configure(duckdb_config* config) const
{
    duckdb_set_config(*config, "access_mode", DuckDBConnectionSettings::accessModeAsString(access_mode).c_str()); // or READ_ONLY
    duckdb_set_config(*config, "threads", std::to_string(num_threads).c_str());
    duckdb_set_config(*config, "max_memory", (std::to_string(max_ram_gb) + "GB").c_str());
    duckdb_set_config(*config, "default_order", DuckDBConnectionSettings::sortOrderAsString(sort_order_default).c_str());
}
