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

#include "intervalbase.h"
#include "timeperiod.h"
#include "evaluationmanager.h"

#include "eval/results/intervalbase.h"

#include "util/timeconv.h"


namespace EvaluationRequirement 
{

/**
*/
boost::posix_time::time_duration Event::dt() const
{
    return interval_time1 - interval_time0;
}

/**
*/
double Event::dtSeconds() const
{
    return Utils::Time::partialSeconds(dt());
}

/**
*/
IntervalBase::IntervalBase(const std::string& name, 
                           const std::string& short_name, 
                           const std::string& group_name,
                           float prob, 
                           COMPARISON_TYPE prob_check_type, 
                           EvaluationManager& eval_man,
                           float update_interval_s, 
                           const boost::optional<float>& min_gap_length_s,
                           const boost::optional<float>& max_gap_length_s,
                           const boost::optional<float>& miss_tolerance_s,
                           const boost::optional<float>& min_ref_period_s,
                           const boost::optional<bool>& must_hold_for_any_target)
:   ProbabilityBase          (name, short_name, group_name, prob, prob_check_type, eval_man)
,   update_interval_s_       (update_interval_s       )
,   min_gap_length_s_        (min_gap_length_s        )
,   max_gap_length_s_        (max_gap_length_s        )
,   miss_tolerance_s_        (miss_tolerance_s        )
,   min_ref_period_s_        (min_ref_period_s        )
,   must_hold_for_any_target_(must_hold_for_any_target)
{
}

/**
*/
float IntervalBase::missThreshold() const
{
    return miss_tolerance_s_.has_value() ? update_interval_s_ + miss_tolerance_s_.value() : update_interval_s_;
}

/**
*/
std::shared_ptr<EvaluationRequirementResult::Single> IntervalBase::evaluate(const EvaluationTargetData& target_data, 
                                                                            std::shared_ptr<Base> instance,
                                                                            const SectorLayer& sector_layer)
{
    auto max_ref_time_diff    = Utils::Time::partialSeconds(eval_man_.settings().max_ref_time_diff_);
    bool skip_no_data_details = eval_man_.settings().report_skip_no_data_details_;

    typedef EvaluationRequirementResult::SingleIntervalBase Result;
    typedef EvaluationDetail                                Detail;
    typedef Result::EvaluationDetails                       Details;

    const auto& tst_data = target_data.tstChain().timestampIndexes();

    //determine min max test times
    boost::optional<boost::posix_time::ptime> tmin_tst;
    boost::optional<boost::posix_time::ptime> tmax_tst;
    if (!tst_data.empty())
    {
        tmin_tst = tst_data.begin()->first;
        tmax_tst = tst_data.rbegin()->first;
    }

    //create inside/outside sector reference periods and extend to test data min max times
    TimePeriodCollection periods;
    periods.createFromReference(target_data, sector_layer, max_ref_time_diff);

    if (min_ref_period_s_.has_value() && min_ref_period_s_.value() > 0)
        periods.removeSmallPeriods(Utils::Time::partialSeconds(min_ref_period_s_.value()));
    
    periods.fillInOutsidePeriods(tmin_tst, tmax_tst);

    uint32_t sum_uis = periods.getUIs(update_interval_s_);

    //sort test updates into ref periods
    for (const auto& tst_it : tst_data)
    {
        auto timestamp = tst_it.first;

        bool inside_period = periods.isInside(timestamp);
        assert(inside_period); //!should _always_ land in a period!

        TimePeriodUpdate update;
        update.data_id = dbContent::TargetReport::DataID(tst_it);

        periods.addUpdate(update);
    }

    //create events from periods
    auto events = periodEvents(periods, target_data, sector_layer, skip_no_data_details);

    //process events and create details
    Details details;

    auto addDetail = [ & ] (const boost::posix_time::ptime& timestamp,
                            const dbContent::TargetPosition& pos_event,
                            const boost::optional<dbContent::TargetPosition>& pos_event_ref,
                            const QVariant& d_tod,
                            const QVariant& miss_occurred,
                            const QVariant& ref_exists,
                            const QVariant& missed_uis,
                            const std::string& comment)
    {
        details.push_back(Detail(timestamp, pos_event).setValue(Result::DetailKey::DiffTOD     , d_tod        )
                                                      .setValue(Result::DetailKey::MissOccurred, miss_occurred)
                                                      .setValue(Result::DetailKey::RefExists   , ref_exists   )
                                                      .setValue(Result::DetailKey::MissedUIs   , missed_uis   )
                                                      .addPosition(pos_event_ref)
                                                      .generalComment(comment));
    };

    size_t   ne           = events.size();
    uint32_t misses_total = 0;

    for (size_t i = 0; i < ne; ++i)
    {
        const auto& event = events[ i ];

        //accumulate misses up to the current event
        misses_total += event.misses;

        //generate detail info for event and skip if no detail shall be generated
        auto detail_info = eventDetailInfo(target_data, event);
        if (!detail_info.generate_detail)
            continue;

        //!events which cause misses should generate comments!
        assert(event.misses == 0 || !detail_info.evt_comment.empty());

        //add detail for event
        addDetail(detail_info.evt_time, 
                  detail_info.evt_position, 
                  detail_info.evt_position_ref, 
                  detail_info.evt_dt, 
                  detail_info.evt_has_misses, 
                  detail_info.evt_has_ref,
                  misses_total, 
                  detail_info.evt_comment);
    }

    //create derived-specific result
    return createResult("UTN:" + std::to_string(target_data.utn_),
                        instance,
                        target_data,
                        sector_layer,
                        details,
                        periods,
                        sum_uis,
                        misses_total);
}

/**
*/
std::vector<Event> IntervalBase::periodEvents(const TimePeriodCollection& periods,
                                              const EvaluationTargetData& target_data,
                                              const SectorLayer& sector_layer,
                                              bool skip_no_data_details) const
{
    std::vector<Event> events;

    uint32_t inside_period_cnt = 0;

    for (size_t i = 0; i < periods.size(); ++i)
    {
        const auto& period = periods.period(i);

        //create events for period
        auto events_period = periodEvents(period, target_data, sector_layer, skip_no_data_details);

        //period is inside period?
        if (period.type() == TimePeriod::Type::InsideSector)
        {
            //add inside period index to events
            for (auto& e : events_period)
                e.period = inside_period_cnt;

            ++inside_period_cnt;
        }

        //collect
        if (!events_period.empty())
            events.insert(events.end(), events_period.begin(), events_period.end());
    }

    return events;
}

/**
*/
std::vector<Event> IntervalBase::periodEvents(const TimePeriod& period,
                                              const EvaluationTargetData& target_data,
                                              const SectorLayer& sector_layer,
                                              bool skip_no_data_details) const
{
    std::vector<Event> events;

    auto logEvent = [ & ] (Event::Type type, 
                           const TimePeriodUpdate* update,
                           const boost::posix_time::ptime& time0,
                           const boost::posix_time::ptime& time1,
                           const std::string& error,
                           bool has_ref_data,
                           bool compute_misses)
    {
        assert(time1 >= time0);

        Event e;
        e.type           = type;
        e.error          = error;
        e.interval_time0 = time0;
        e.interval_time1 = time1;

        if (update)
            e.data_id = update->data_id;

        if (compute_misses)
        {
            double dt = Utils::Time::partialSeconds(time1 - time0);

            e.misses = numMisses(dt);
        }

        e.had_ref_data = has_ref_data;

        events.push_back(e);
    };

    //outside sector period => add all updates as OutsideSector
    if (period.type() == TimePeriod::Type::OutsideSector)
    {
        if (!skip_no_data_details)
        {
            for (const auto& update : period.getUpdates())
            {
                logEvent(Event::TypeNoReference, &update, update.data_id.timestamp(), update.data_id.timestamp(), "", false, false);
            }
        }

        return events;
    }

    //log period entered
    if (!skip_no_data_details)
        logEvent(Event::TypeEnterPeriod, nullptr, period.begin(), period.end(), "", false, false);

    //inside sector period
    const auto& updates = period.getUpdates();

    if (updates.empty())
    {
        //no updates in period => add single event for this case
        if (!skip_no_data_details)
            logEvent(Event::TypeEmptyPeriod, nullptr, period.begin(), period.end(), "", false, false);
    }
    else
    {
        //period obtains updates
        auto max_ref_time_diff = Utils::Time::partialSeconds(eval_man_.settings().max_ref_time_diff_);

        size_t n = updates.size();

        const TimePeriodUpdate* valid_last = nullptr;
        bool ref_data_missing = false;

        for (size_t i = 0; i < n; ++i)
        {
            const auto& update = updates[ i ];

            //check if update is valid
            auto validity = isValid(update.data_id, target_data, sector_layer, max_ref_time_diff);

            //invalid update => log and skip
            if (validity.value == Validity::Value::Invalid)
            {
                logEvent(Event::TypeInvalid, &update, update.data_id.timestamp(), update.data_id.timestamp(), validity.comment, false, false);
                continue;
            }

            //first valid update? => log
            if (!valid_last)
                logEvent(Event::TypeFirstValidUpdateInPeriod, &update, update.data_id.timestamp(), update.data_id.timestamp(), "", false, false);

            //was reference data missing? => yields a valid update (in dubio pro reo)
            ref_data_missing = validity.value == Validity::Value::RefDataMissing;
            
            //valid update => check for misses
            logEvent(valid_last ? Event::TypeValid : Event::Type::TypeValidFirst,
                     &update,
                     valid_last ? valid_last->data_id.timestamp() : period.begin(),      // time0 is either the last valid update or period begin
                     update.data_id.timestamp(),                                         // time1 always update time
                     "",
                     !ref_data_missing,
                     true);                                                              // check misses in both cases

            valid_last = &updates[ i ];
        }

        //add event for last valid update
        if (valid_last)
        {
            logEvent(Event::TypeLastValidUpdateInPeriod, valid_last, valid_last->data_id.timestamp(), valid_last->data_id.timestamp(), "", false, false);
            logEvent(Event::TypeValidLast, valid_last, valid_last->data_id.timestamp(), period.end(), "", !ref_data_missing, true);
        }

        //no valid updates in period => add single event for this case
        if (!valid_last)
        {
            if (!skip_no_data_details)
                logEvent(Event::TypeEmptyPeriod, nullptr, period.begin(), period.end(), "", false, false);
        }
    }

    //log period left
    if (!skip_no_data_details)
        logEvent(Event::TypeLeavePeriod, nullptr, period.begin(), period.end(), "", false, false);

    return events;
}

/**
*/
uint32_t IntervalBase::numMisses(double dt) const
{
    if (miss_tolerance_s_.has_value())
        dt -= miss_tolerance_s_.value();

    if (min_gap_length_s_.has_value() && dt < min_gap_length_s_.value()) // supress gaps smaller as min gap length
        return 0;

    if (max_gap_length_s_.has_value() && dt > max_gap_length_s_.value()) // supress gaps larger as max gap length
        return 0;

    if (dt <= update_interval_s_)
        return 0;

    return (uint32_t)std::floor(dt / update_interval_s_);
}

/**
*/
IntervalBase::DetailInfo IntervalBase::eventDetailInfo(const EvaluationTargetData& target_data, 
                                                       const Event& event) const
{
    std::string thres        = Utils::String::doubleToStringPrecision(missThreshold(), 2);
    std::string period       = "period " + (event.period.has_value() ? std::to_string(event.period.value()) : "");
    std::string miss         = "(DToD > " + thres + ")";
    std::string hit          = "(DToD <= " + thres + ")";
    std::string evt_ts       = Utils::Time::toString(event.interval_time1);
    std::string evt_interval = "[" + Utils::Time::toString(event.interval_time0) + "," + evt_ts + "]";
    std::string error        = event.error.empty() ? "Unknown" : event.error;

    bool has_miss = event.misses > 0;

    IntervalBase::DetailInfo dinfo;
    dinfo.evt_has_misses = has_miss;
    dinfo.evt_dt         = event.dtSeconds();

    switch (event.type)
    {
        case Event::TypeNoReference:
        {
            assert(target_data.tstChain().posOpt(event.data_id).has_value());

            dinfo.evt_time        = event.data_id.timestamp();
            dinfo.evt_comment     = "Outside sector/reference period";
            dinfo.evt_position    = target_data.tstChain().pos(event.data_id);
            dinfo.evt_has_ref     = false;
            dinfo.generate_detail = true;

            break;
        }
        case Event::TypeEnterPeriod:
        {
            assert(target_data.refChain().posOpt(event.interval_time0).has_value());

            dinfo.evt_time        = event.interval_time0;
            dinfo.evt_comment     = "Entering " + period;
            dinfo.evt_position    = target_data.refChain().pos(event.interval_time0);
            dinfo.evt_has_ref     = true;
            dinfo.generate_detail = true;

            break;
        }
        case Event::TypeFirstValidUpdateInPeriod:
        {
            assert(target_data.tstChain().posOpt(event.data_id).has_value());

            dinfo.evt_time        = event.data_id.timestamp();
            dinfo.evt_comment     = "First valid target report of " + period;
            dinfo.evt_position    = target_data.tstChain().pos(event.data_id);
            dinfo.evt_has_ref     = true;
            dinfo.generate_detail = true;

            break;
        }
        case Event::TypeValidFirst:
        {
            assert(target_data.tstChain().posOpt(event.data_id).has_value());
            assert(target_data.refChain().posOpt(event.interval_time0).has_value());

            dinfo.evt_time         = event.data_id.timestamp();
            dinfo.evt_comment      = has_miss ? "Miss detected at start of current " + period + " " + miss + ", between " + evt_interval : "OK " + hit;
            dinfo.evt_position     = target_data.tstChain().pos(event.data_id);
            dinfo.evt_position_ref = target_data.refChain().pos(event.interval_time0);
            dinfo.evt_has_ref      = true;
            dinfo.generate_detail  = true;

            break;
        }
        case Event::TypeValid:
        {
            assert(target_data.tstChain().posOpt(event.data_id).has_value());
            assert(target_data.tstChain().posOpt(event.interval_time0).has_value());

            dinfo.evt_time         = event.data_id.timestamp();
            dinfo.evt_comment      = has_miss ? "Miss detected in current " + period + " " + miss + ", between " + evt_interval : "OK " + hit;
            dinfo.evt_position     = target_data.tstChain().pos(event.data_id);
            dinfo.evt_position_ref = target_data.tstChain().pos(event.interval_time0);
            dinfo.evt_has_ref      = true;
            dinfo.generate_detail  = true;

            break;
        }
        case Event::TypeValidLast:
        {
            assert(target_data.tstChain().posOpt(event.data_id).has_value());
            assert(target_data.refChain().posOpt(event.interval_time1).has_value());

            dinfo.evt_time         = event.data_id.timestamp();
            dinfo.evt_comment      = has_miss ? "Miss detected at end of current " + period + " " + miss + ", between " + evt_interval : 
                                                "End of current " + period + " OK " + hit;
            dinfo.evt_position     = target_data.tstChain().pos(event.data_id);
            dinfo.evt_position_ref = target_data.refChain().pos(event.interval_time1);
            dinfo.evt_has_ref      = true;
            dinfo.generate_detail  = true;

            break;
        }
        case Event::TypeInvalid:
        {
            assert(target_data.tstChain().posOpt(event.data_id).has_value());

            dinfo.evt_time        = event.data_id.timestamp();
            dinfo.evt_comment     = "Invalid target report: " + error;
            dinfo.evt_position    = target_data.tstChain().pos(event.data_id);
            dinfo.evt_has_ref     = true;
            dinfo.generate_detail = true;

            break;
        }
        case Event::TypeDataMissing:
        {
            assert(target_data.tstChain().posOpt(event.data_id).has_value());

            dinfo.evt_time        = event.data_id.timestamp();
            dinfo.evt_comment     = "Invalid target report: " + error;
            dinfo.evt_position    = target_data.tstChain().pos(event.data_id);
            dinfo.evt_has_ref     = true;
            dinfo.generate_detail = true;

            break;
        }
        case Event::TypeLastValidUpdateInPeriod:
        {
            assert(target_data.tstChain().posOpt(event.data_id).has_value());

            dinfo.evt_time        = event.data_id.timestamp();
            dinfo.evt_comment     = "Last valid target report of " + period;
            dinfo.evt_position    = target_data.tstChain().pos(event.data_id);
            dinfo.evt_has_ref     = true;
            dinfo.generate_detail = true;

            break;
        }
        case Event::TypeLeavePeriod:
        {
            assert(target_data.refChain().posOpt(event.interval_time1).has_value());

            dinfo.evt_time        = event.interval_time1;
            dinfo.evt_comment     = "Leaving " + period;
            dinfo.evt_position    = target_data.refChain().pos(event.interval_time1);
            dinfo.evt_has_ref     = true;
            dinfo.generate_detail = true;

            break;
        }
        case Event::TypeEmptyPeriod:
        {
            assert(target_data.refChain().posOpt(event.interval_time0).has_value());
            assert(target_data.refChain().posOpt(event.interval_time1).has_value());

            dinfo.evt_time         = event.interval_time1;
            dinfo.evt_comment      = has_miss ? "Miss detected in empty period " + period + " " + miss + ", between " + evt_interval : "";
            dinfo.evt_position     = target_data.refChain().pos(event.interval_time1);
            dinfo.evt_position_ref = target_data.refChain().pos(event.interval_time0);
            dinfo.evt_has_ref      = true;
            dinfo.generate_detail  = has_miss;

            break;
        }
    }

    //add remark that reference data was missing
    if (event.had_ref_data)
        dinfo.evt_comment += " (no ref info)";

    return dinfo;
}

} // namespace EvaluationRequirement
