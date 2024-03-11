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

#include "reconstructorbase.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "logger.h"
#include "timeconv.h"

using namespace std;
using namespace Utils;

/**
*/
ReconstructorBase::ReconstructorBase()
{
}

/**
*/
ReconstructorBase::~ReconstructorBase() = default;

bool ReconstructorBase::hasNextTimeSlice()
{
    if (current_slice_begin_.is_not_a_date_time())
    {
        assert (COMPASS::instance().dbContentManager().hasMinMaxTimestamp());
        std::tie(current_slice_begin_, timestamp_max_) = COMPASS::instance().dbContentManager().minMaxTimestamp();

        loginf << "ReconstructorBase: hasNextTimeSlice: new min " << Time::toString(current_slice_begin_)
               << " max " << Time::toString(timestamp_max_);
    }

    assert (!current_slice_begin_.is_not_a_date_time());
    assert (!timestamp_max_.is_not_a_date_time());

    return current_slice_begin_ < timestamp_max_;
}

TimeWindow ReconstructorBase::getNextTimeSlice()
{
    assert (hasNextTimeSlice());

    assert (!current_slice_begin_.is_not_a_date_time());
    assert (!timestamp_max_.is_not_a_date_time());

    assert (current_slice_begin_ <= timestamp_max_);

    boost::posix_time::ptime current_slice_end = current_slice_begin_ + slice_duration_;

    TimeWindow window {current_slice_begin_, current_slice_end};

    current_slice_begin_ = current_slice_end;

    //assert (current_slice_begin_ <= timestamp_max_); can be bigger

    return window;
}

/**
*/
bool ReconstructorBase::processSlice(Buffers&& buffers)
{
    return processSlice_impl(std::move(buffers));
}


void ReconstructorBase::clear()
{
    current_slice_begin_ = {};
    timestamp_max_ = {};
}
