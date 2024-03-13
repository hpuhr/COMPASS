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
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "logger.h"
#include "timeconv.h"

using namespace std;
using namespace Utils;

/**
 */
ReconstructorBase::ReconstructorBase()
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

        loginf << "ReconstructorBase: hasNextTimeSlice: new min " << Time::toString(current_slice_begin_)
               << " max " << Time::toString(timestamp_max_) << " first_slice " << first_slice_;
    }

    assert (!current_slice_begin_.is_not_a_date_time());
    assert (!timestamp_max_.is_not_a_date_time());

    first_slice_ = current_slice_begin_ == timestamp_min_;

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

    first_slice_ = current_slice_begin_ == timestamp_min_;

    if (first_slice_)
        remove_before_time_ = current_slice_begin_;
    else
        current_slice_begin_ = current_slice_begin_ - outdated_duration_;

    current_slice_begin_ = current_slice_end; // for next iteration


            //assert (current_slice_begin_ <= timestamp_max_); can be bigger

    return window;
}

/**
 */
bool ReconstructorBase::processSlice(Buffers&& buffers)
{
    loginf << "ReconstructorBase: processSlice: first_slice " << first_slice_;

    if (!first_slice_)
    {
        removeOldBufferData();
        accessor_->removeEmptyBuffers();
    }

    accessor_->add(buffers);

    return processSlice_impl();
}


void ReconstructorBase::clear()
{
    current_slice_begin_ = {};
    timestamp_max_ = {};
}

void ReconstructorBase::removeOldBufferData()
{
    unsigned int buffer_size;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    loginf << "ReconstructorBase: clearOldBufferData: remove_before_time " << Time::toString(remove_before_time_);

    for (auto& buf_it : buffers_)
    {
        buffer_size = buf_it.second->size();

        assert (dbcont_man.metaVariable(DBContent::meta_var_timestamp_.name()).existsIn(buf_it.first));

        dbContent::Variable& ts_var = dbcont_man.metaVariable(DBContent::meta_var_timestamp_.name()).getFor(buf_it.first);

        Property ts_prop {ts_var.name(), ts_var.dataType()};

        if (buf_it.second->hasProperty(ts_prop))
        {
            NullableVector<boost::posix_time::ptime>& ts_vec = buf_it.second->get<boost::posix_time::ptime>(
                ts_var.name());

            unsigned int index=0;

            for (; index < buffer_size; ++index)
            {
                if (!ts_vec.isNull(index) && ts_vec.get(index) > remove_before_time_)
                {
                    logdbg << "ReconstructorBase: clearOldBufferData: found " << buf_it.first
                           << " cutoff tod index " << index
                           << " ts " << Time::toString(ts_vec.get(index));
                    break;
                }
            }
            // index == buffer_size if none bigger than min_ts

            if (index) // index found
            {
                index--; // cut at previous

                logdbg << "ReconstructorBase: clearOldBufferData: cutting " << buf_it.first
                       << " up to index " << index
                       << " total size " << buffer_size
                       << " index time " << (ts_vec.isNull(index) ? "null" : Time::toString(ts_vec.get(index)));
                assert (index < buffer_size);
                buf_it.second->cutUpToIndex(index);
            }
        }
    }
}
