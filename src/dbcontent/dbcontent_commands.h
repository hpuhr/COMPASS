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

#include "rtcommand/rtcommand.h"

#include <vector>

#include <boost/optional.hpp>

#include <QStringList>

namespace dbContent
{

/**
 */
struct RTCommandGetTable : public rtcommand::RTCommand
{
    QString                       dbcontent;
    boost::optional<unsigned int> utn;
    QStringList                   variables;

protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(get_table, "retrieves table data from the database")
    DECLARE_RTCOMMAND_OPTIONS
};

} // namespace dbContent
