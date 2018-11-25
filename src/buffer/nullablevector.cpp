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

#include <cassert>
#include <limits>

#include <QDateTime>

#include "stringconv.h"
#include "nullablevector.h"
#include "buffer.h"

typedef std::numeric_limits<double> double_limit;
typedef std::numeric_limits<float> float_limit;

template <>
NullableVector<bool>& NullableVector<bool>::operator*=(double factor)
{
    bool tmp_factor = static_cast<bool> (factor);

    for (auto data_it : data_)
        data_it = data_it && tmp_factor;

    return *this;
}



