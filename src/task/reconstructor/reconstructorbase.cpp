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
#include "reconstructortask.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "logger.h"
#include "timeconv.h"

using namespace std;
using namespace Utils;

/**
 */
ReconstructorBase::ReconstructorBase(const std::string& class_id, const std::string& instance_id,
                                     ReconstructorTask& task)
    : Configurable (class_id, instance_id, &task)
{
    accessor_ = make_shared<dbContent::DBContentAccessor>();
}

/**
 */
ReconstructorBase::~ReconstructorBase() = default;

bool ReconstructorBase::hasNextTimeSlice()
{
    if (current_slice_begin_.is_not_a_date_time())
    {
        assert (COMPASS::instance().dbContentManager().hasMinMaxTimestamp());
        std::tie(timestamp_min_, timestamp_max_) = COMPASS::instance().dbContentManager().minMaxTimestamp();

        current_slice_begin_ = timestamp_min_;
        next_slice_begin_ = timestamp_min_; // first slice

        loginf << "ReconstructorBase: hasNextTimeSlice: new min " << Time::toString(current_slice_begin_)
               << " max " << Time::toString(timestamp_max_) << " first_slice " << first_slice_;
    }

    assert (!current_slice_begin_.is_not_a_date_time());
    assert (!timestamp_max_.is_not_a_date_time());

    first_slice_ = current_slice_begin_ == timestamp_min_;

    return next_slice_begin_ < timestamp_max_;
}

TimeWindow ReconstructorBase::getNextTimeSlice()
{
    assert (hasNextTimeSlice());

    current_slice_begin_ = next_slice_begin_;

    assert (!current_slice_begin_.is_not_a_date_time());
    assert (!timestamp_max_.is_not_a_date_time());

    assert (current_slice_begin_ <= timestamp_max_);

    boost::posix_time::ptime current_slice_end = current_slice_begin_ + slice_duration_;

    TimeWindow window {current_slice_begin_, current_slice_end};

    loginf << "ReconstructorBase: getNextTimeSlice: current_slice_begin " << Time::toString(current_slice_begin_)
           << " current_slice_end " << Time::toString(current_slice_end);

    first_slice_ = current_slice_begin_ == timestamp_min_;

    remove_before_time_ = current_slice_begin_ - outdated_duration_;

    next_slice_begin_ = current_slice_end; // for next iteration

            //assert (current_slice_begin_ <= timestamp_max_); can be bigger

    return window;
}

/**
 */
bool ReconstructorBase::processSlice(Buffers&& buffers)
{
    loginf << "ReconstructorBase: processSlice: first_slice " << first_slice_;

    if (!first_slice_)
        accessor_->removeContentBeforeTimestamp(remove_before_time_);

    accessor_->add(buffers);

    return processSlice_impl();
}


void ReconstructorBase::reset()
{
    current_slice_begin_ = {};
    next_slice_begin_ = {};
    timestamp_min_ = {};
    timestamp_max_ = {};
}

