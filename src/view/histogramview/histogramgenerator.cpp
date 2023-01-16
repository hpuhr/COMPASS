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

#include "histogramgenerator.h"
#include "logger.h"

/**
 * Resets all data.
 */
void HistogramGenerator::reset()
{
    num_bins_          = 0;
    sub_range_active_  = false;
    intermediate_data_ = {};
    results_           = {};

    reset_impl();
}

/**
 */    
unsigned int HistogramGenerator::currentBins() const 
{
    return num_bins_;
}

/**
 * Checks if a valid result is available.
 */
bool HistogramGenerator::hasValidResult() const
{
    return (num_bins_ > 0 && 
            !results_.content_results.empty() &&
            results_.content_results.size() == intermediate_data_.size());
}

/**
 * Refills the existing histograms and generates new result data.
 */
bool HistogramGenerator::refill()
{
    loginf << "HistogramGenerator: Refilling...";

    if (!hasData())
        return false;

    //reset computed data
    intermediate_data_ = {};
    results_           = {};

    //run refill in derived classes
    if (!refill_impl())
        return false;

    loginf << "HistogramGenerator: Finalizing results...";

    //compile results from intermediate data
    if (!finalizeResults())
        return false;

    loginf << "HistogramGenerator: Refilled!";

    return true;
}

/**
 * Runs a complete update.
 */
void HistogramGenerator::update()
{
    loginf << "HistogramGenerator: Running update...";

    if (!hasData())
        return;

    //reset content
    reset();

    //generate histograms before refill (if implemented in derived class)
    generateHistograms_impl();

    //refill data
    refill();

    loginf << "HistogramGenerator: Updated!";
}

/**
 * Selects the given sub range of bins in the data.
 */
bool HistogramGenerator::select(unsigned int bin0, unsigned int bin1)
{
    loginf << "HistogramGenerator: Selecting...";

    if (!hasData())
        return false;

    //selection needs a valid result
    if (!hasValidResult())
        return false;

    //determine min and max index
    unsigned int min_index = std::min(bin0, bin1);
    unsigned int max_index = std::max(bin0, bin1);

    //selection is implemented in derived classes
    return select_impl(min_index, max_index);
}

/**
 * Selection stub.
 */
bool HistogramGenerator::select_impl(unsigned int bin0, unsigned int bin1)
{
    throw std::runtime_error("HistogramGenerator::select_impl: not implemented yet");
    return false;
}

/**
 * Zooms to the given subrange of bins.
 */
bool HistogramGenerator::zoom(unsigned int bin0, unsigned int bin1)
{
    loginf << "HistogramGenerator: Zooming to bin range...";

    if (!hasData())
        return false;

    if (!hasValidResult())
        return false;

    //both bins oor -> do nothing
    if (bin0 >= num_bins_ && bin1 >= num_bins_)
        return false;

    unsigned int min_index = std::min(bin0, bin1);
    unsigned int max_index = std::max(bin0, bin1);

    min_index = std::min(min_index, num_bins_ - 1);
    max_index = std::min(max_index, num_bins_ - 1);

    //zooming is implemented in derived classes.
    if (!zoom_impl(min_index, max_index))
        return false;

    sub_range_active_ = true;

    //refill histograms
    if (!refill())
        return false;

    loginf << "HistogramGenerator: Zooming finished!";

    return true;
}

/**
 * Zoom stub.
 */
bool HistogramGenerator::zoom_impl(unsigned int bin0, unsigned int bin1)
{
    throw std::runtime_error("HistogramGenerator::zoom_impl: not implemented yet");
    return false;
}

/**
 * Extracts histogram data from intermediate results and stores it to the final results.
 */
bool HistogramGenerator::finalizeResults()
{
    results_  = {};
    num_bins_ = 0;

    if (!intermediate_data_.empty())
    {
        num_bins_ = (unsigned int)intermediate_data_.begin()->second.bin_data.size();
    }

    results_.valid_counts.assign(num_bins_, 0);
    results_.selected_counts.assign(num_bins_, 0);

    for (auto& elem : intermediate_data_)
    {
        const auto& d = elem.second;
        auto&       r = results_.content_results[ elem.first ];

        r.bins.resize(d.bin_data.size());

        r.null_count          = d.null_count;
        r.null_selected_count = d.null_selected_count;
        r.not_inserted_count  = d.not_inserted_count;

        r.bins_are_sorted     = d.bins_are_sorted;

        for (size_t i = 0; i < d.bin_data.size(); ++i)
        {
            const auto& bin_data = d.bin_data.at(i);
            auto&       res_bin  = r.bins.at(i);

            res_bin = bin_data;

            //per db content counts
            r.valid_count    += res_bin.count;
            r.selected_count += res_bin.selected;

            if (res_bin.count > r.max_count)
                r.max_count = res_bin.count;

            //per bin counts
            results_.valid_counts   [ i ] += res_bin.count;
            results_.selected_counts[ i ] += res_bin.selected;
        }

        //total counts
        results_.valid_count         += r.valid_count;
        results_.selected_count      += r.selected_count;
        results_.null_count          += r.null_count;
        results_.null_selected_count += r.null_selected_count;
        results_.not_inserted_count  += r.not_inserted_count;
        
        if (r.max_count > results_.max_count)
            results_.max_count = r.max_count;

        num_bins_ = (int)r.bins.size();
    }

    assert(subRangeActive() || results_.not_inserted_count == 0);

    return true;
}

/**
 * Prints the result to the command line.
 */
void HistogramGenerator::print() const
{
    for (const auto& elem : results_.content_results)
    {
        const auto& r = elem.second;

        std::cout << "* DBContent = " << elem.first << std::endl;

        std::cout << "  ";
        for (const auto& bin : r.bins)
            std::cout << "[" << bin.labels.label << ": " << bin.count << "]"; 
        std::cout << std::endl;

        std::cout << "  null:            " << r.null_count << std::endl;
        std::cout << "  null + selected: " << r.null_selected_count << std::endl;
        std::cout << "  not inserted:    " << r.not_inserted_count << std::endl;
        
        std::cout << std::endl;
    }
}

/**
 */
std::pair<std::string, std::string> HistogramGenerator::currentRangeAsLabels() const
{
    if (!hasValidResult())
        return {};

    if (!results_.content_results.begin()->second.bins_are_sorted)
        return {};

    const auto& bin_data0 = results_.content_results.begin()->second.bins.front();
    const auto& bin_data1 = results_.content_results.begin()->second.bins.back();

    return std::make_pair(bin_data0.labels.label_min, bin_data1.labels.label_max);
}
