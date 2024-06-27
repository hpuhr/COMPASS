
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

#include <vector>

/**
*/
class ValueAccumulator
{
public:
    ValueAccumulator() = default;
    virtual ~ValueAccumulator() = default;

    void reset();
    void accumulate(const std::vector<double>& values, bool do_finalize = false);
    void finalize();

    double min() const;
    double max() const;
    double mean() const;
    double rms() const;
    double var() const;
    double stddev() const;

    std::size_t numValues() const;

private:
    double      value_min_    = 0;
    double      value_max_    = 0;
    double      value_avg_    = 0;
    double      value_rms_    = 0;
    double      value_var_    = 0;
    double      value_stddev_ = 0;
    std::size_t value_cnt_    = 0;
    bool        finalized_    = false;
};
