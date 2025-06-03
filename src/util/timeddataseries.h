#pragma once

#include <vector>
#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

/**
 * @brief A time‐indexed series storing unsigned long values along with an integer confidence
 *        and the original insert timestamp.
 *
 * Template parameter T should be a callable type (e.g., a lambda or functor) that,
 * given an unsigned long, returns an int representing the confidence of that value.
 *
 * Internally, timestamps (boost::posix_time::ptime) are mapped into a zero‐based
 * std::vector index based on a minimum and maximum ptime, with 1‐second resolution.
 * Each element of the vector is a boost::optional<Entry>, where Entry holds:
 *   - the stored unsigned long value
 *   - the computed confidence (int) for that value
 *   - the original ptime at which this entry was inserted
 *
 * On insertion, you supply:
 *   1) a timestamp
 *   2) an unsigned long value
 *   3) a maxSeconds window
 *
 * The class computes:
 *   int conf = confidenceFunc(value);
 *
 * Then, for each second‐offset i in [ -maxSeconds, +maxSeconds ], it checks the slot at:
 *   index = convertTimestampToIndex(timestamp) + i
 * If index is in‐bounds, it will write { value, conf, timestamp } into data_[index]
 * under these rules:
 *   1. If the slot is empty → accept immediately.
 *   2. If existing.confidence < conf    → override.
 *   3. If existing.confidence == conf → compare distances:
 *         abs((existing.insert_time) – (slot_timestamp))
 *       vs
 *         abs((timestamp) – (slot_timestamp)).
 *       If the new timestamp is strictly closer, override.
 *
 * Any timestamp outside [min_ptime_, max_ptime_] is ignored by insert().
 *
 * Function names use CamelCase; member variables use snake_case.
 */
template <typename T>
class TimedDataSeries
{
public:
    /// Internal storage type for each slot: (value + confidence + insert_time)
    struct Entry
    {
        unsigned long                value;
        int                          confidence;
        boost::posix_time::ptime     insert_time;
    };

    using OptionalEntry = boost::optional<Entry>;
    using ptime         = boost::posix_time::ptime;
    using time_duration = boost::posix_time::time_duration;

    /**
     * @brief Construct a TimedDataSeries with a given [min_ptime, max_ptime] range and a confidence functor.
     *
     * @param min_ptime     The earliest timestamp (inclusive) that the vector will cover.
     * @param max_ptime     The latest timestamp (inclusive) that the vector will cover.
     * @param confidenceFunc A callable (e.g., lambda) that takes an unsigned long and returns an int confidence.
     *
     * Initializes an internal std::vector of size ((max_ptime - min_ptime).total_seconds() + 1).
     * All slots start out empty (boost::none).  Timestamps outside this range will be ignored by insert().
     */
    TimedDataSeries(const ptime&                 min_ptime,
               const ptime&                      max_ptime,
               unsigned int                      max_seconds,
               const T&                          confidenceFunc)
        : min_ptime_(min_ptime)
        , max_ptime_(max_ptime)
        , max_seconds_(max_seconds)
        , confidence_func_(confidenceFunc)
    {
        assert (!min_ptime_.is_not_a_date_time());
        assert (!max_ptime_.is_not_a_date_time());
        assert (min_ptime_ < max_ptime_);

        initializeDataVector();
    }

    /**
     * @brief Reset the covered time range to a new [min_ptime, max_ptime], clearing all stored data.
     *
     * After calling resetRange(), any previously stored samples are discarded.  A new internal vector
     * is allocated with 1‐second resolution from min_ptime to max_ptime (inclusive).
     *
     * @param new_min_ptime   The new earliest timestamp (inclusive).
     * @param new_max_ptime   The new latest timestamp (inclusive).
     */
    void resetRange(const ptime& new_min_ptime,
                    const ptime& new_max_ptime)
    {
        assert (!new_min_ptime.is_not_a_date_time());
        assert (!new_max_ptime.is_not_a_date_time());
        assert (new_min_ptime < new_max_ptime);

        min_ptime_ = new_min_ptime;
        max_ptime_ = new_max_ptime;

        initializeDataVector();
    }

    /**
     * @brief Insert a (timestamp, value) pair, with propagation to ±max_seconds, using the stored confidence functor.
     *
     * This computes:
     *   int conf = confidence_func_(value);
     *
     * Then, for each second‐offset i in [ -max_seconds, +max_seconds ], it checks the slot at:
     *   index = convertTimestampToIndex(timestamp) + i
     * If index is in‐bounds, it will write { value, conf, timestamp } into data_[index]
     * under these rules:
     *   1) If the slot is empty → accept immediately.
     *   2) If existing.confidence < conf → override.
     *   3) If existing.confidence == conf → compare:
     *         distance(existing.insert_time, slot_ptime)
     *       vs
     *         distance(timestamp, slot_ptime).
     *       If new timestamp is strictly closer, override.
     *
     * Any timestamp outside [min_ptime_, max_ptime_] is ignored entirely.
     *
     * @param timestamp   The ptime at which the value was measured.
     * @param value       The unsigned long measurement.
     * @param max_seconds  How many seconds around `timestamp` to propagate this value.
     */
    void insert(const ptime& timestamp,
                unsigned long value)
    {
        // Reject if outside the defined range
        assert (timestamp >= min_ptime_ && timestamp <= max_ptime_);

        // Compute confidence for this value
        int conf = confidence_func_(value);

        // Base index for this exact timestamp
        std::size_t base_index = convertTimestampToIndex(timestamp);

        // The insertion timestamp (to store in each Entry)
        const ptime insertion_time = timestamp;

        // Propagate to all indices in [base_index - maxSeconds, base_index + maxSeconds]
        int window = static_cast<int>(max_seconds_);

        for (int delta = -window; delta <= window; ++delta)
        {
            int idx = static_cast<int>(base_index) + delta;
            if (idx < 0 || idx >= static_cast<int>(data_.size())) {
                continue;
            }
            std::size_t uidx = static_cast<std::size_t>(idx);

            // Compute the ptime corresponding to this slot index:
            //   slot_ptime = min_ptime_ + seconds(idx)
            ptime slot_ptime = min_ptime_ + time_duration(static_cast<long>(uidx), 0, 0);

            // If slot is empty, accept immediately
            if (!data_[uidx].is_initialized()) {
                data_[uidx] = Entry{ value, conf, insertion_time };
                continue;
            }

            // Slot already has an entry: compare confidences
            Entry const& existing = *data_[uidx];

            if (existing.confidence < conf) {
                // New sample has strictly higher confidence → override
                data_[uidx] = Entry{ value, conf, insertion_time };
            }
            else if (existing.confidence == conf)
            {
                // Same confidence tier → compare which insertion time is closer to slot_ptime
                time_duration existing_diff = existing.insert_time - slot_ptime;
                double existing_abs_secs =
                    std::abs(existing_diff.total_microseconds() / 1e6);

                time_duration new_diff = insertion_time - slot_ptime;
                double new_abs_secs =
                    std::abs(new_diff.total_microseconds() / 1e6);

                if (new_abs_secs < existing_abs_secs) {
                    // The new insertion timestamp is strictly closer → override
                    data_[uidx] = Entry{ value, conf, insertion_time };
                }
                // Otherwise, keep the existing entry
            }
            // else (existing.confidence > conf): do nothing
        }
    }

    /**
     * @brief Retrieve the stored Entry (value + confidence + insert_time) at a given timestamp, if any.
     *
     * @param timestamp   The ptime to look up.
     * @return boost::optional<Entry>  The stored entry, or boost::none if empty/out-of-range.
     */
    OptionalEntry getAt(const ptime& timestamp) const
    {
        if (timestamp < min_ptime_ || timestamp > max_ptime_) {
            return boost::none;
        }
        std::size_t idx = convertTimestampToIndex(timestamp);
        return data_[idx];
    }

    /**
     * @brief Return the minimum and maximum ptime currently covered.
     */
    ptime getMinTime() const { return min_ptime_; }
    ptime getMaxTime() const { return max_ptime_; }

    /**
     * @brief Return the total number of seconds (i.e., vector size) covered.
     */
    std::size_t getTotalSeconds() const { return data_.size(); }

private:

    ptime                         min_ptime_;
    ptime                         max_ptime_;
    unsigned int                  max_seconds_{0};

    std::vector<OptionalEntry>    data_;
    T                             confidence_func_;
    /**
     * @brief (Re)initialize the internal data vector based on min_ptime_ and max_ptime_.
     *
     * Computes:
     *   std::size_t num_secs = (max_ptime_ - min_ptime_).total_seconds() + 1;
     * and resizes `data_` to that length, setting all elements to boost::none.
     */
    void initializeDataVector()
    {
        time_duration span = max_ptime_ - min_ptime_;
        assert (!span.is_negative());

        long total_secs = span.total_seconds() + 1;
        data_.clear();
        data_.resize(static_cast<std::size_t>(total_secs), boost::none);
    }

    /**
     * @brief Convert a timestamp into a zero‐based index within [0, data_.size()-1].
     *
     * Assumes `timestamp` ∈ [min_ptime_, max_ptime_].  The calculation is:
     *   index = (timestamp - min_ptime_).total_seconds()
     *
     * @param timestamp   The ptime to convert.
     * @return std::size_t  The index into `data_`.
     */
    std::size_t convertTimestampToIndex(const ptime& timestamp) const
    {
        time_duration diff = timestamp - min_ptime_;

        long seconds       = diff.total_seconds();
        assert (seconds >= 0);

        return static_cast<std::size_t>(seconds);
    }


};

