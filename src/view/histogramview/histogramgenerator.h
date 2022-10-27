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
#include <map>
#include <memory>
#include <string>

/**
 * Base class for histogram generation.
 * Completely independent of the analyzed data.
 * Contains intermediate data containers to be filled by derived classes.
 * Contains result data generated from the intermediate data.
 */
class HistogramGenerator
{
public:
    /**
     * Result bin.
     */
    struct BinData
    {
        unsigned int count    = 0; //number of data points in this bin
        unsigned int selected = 0; //number of selected data points in this bin
        std::string  label;        //label describing this bin
    };

    /**
     * Used to keep track of intermediate values
     */
    struct IntermediateContentData
    {
        void init(size_t n = 0)
        {
            *this = {};

            if (n > 0)
                bin_data.assign(n, {});
        }

        std::vector<BinData> bin_data;        //per bin data

        unsigned int null_count          = 0; //number of encountered null values
        unsigned int null_selected_count = 0; //number of encountered selected null values
        unsigned int not_inserted_count  = 0; //number of encountered non-insertable values (e.g. out of histogram range)
    };

    typedef std::map<std::string, IntermediateContentData> IntermediateData;

    /**
     * Result for a certain DBContent.
     */
    struct ContentResult
    {
        std::vector<BinData> bins;                    //resulting histogram bins
        unsigned int         valid_count         = 0; //number of valid data values in result
        unsigned int         selected_count      = 0; //number of selected data values in result
        unsigned int         null_count          = 0; //number of encountered null values in result
        unsigned int         null_selected_count = 0; //number of encountered selected null values in result
        unsigned int         not_inserted_count  = 0; //number of encountered non-insertable values in result (e.g. out of histogram range)
        unsigned int         max_count           = 0; //maximum encountered bin count
    };

    typedef std::map<std::string, ContentResult> ContentResults;

    /**
     * Result structure.
     */
    struct Results
    {
        bool hasNullValues() const
        {
            return (null_selected_count > 0 || null_count > 0);
        }
        bool hasSelectedValues() const
        {
            return (selected_count > 0 || null_selected_count > 0);
        }
        bool hasOutOfRangeValues() const
        {
            return (not_inserted_count > 0);
        }
        
        ContentResults            content_results;         //results per content type
        std::vector<unsigned int> valid_counts;            //valid data per bin (over multiple content types)
        std::vector<unsigned int> selected_counts;         //selected data per bin (over multiple content types)
        unsigned int              valid_count         = 0; //total valid data count
        unsigned int              selected_count      = 0; //total selected valid data    
        unsigned int              null_count          = 0; //total null data
        unsigned int              null_selected_count = 0; //total selected null data 
        unsigned int              not_inserted_count  = 0; //total non-insertable data (e.g. out of histogram range)
        unsigned int              max_count           = 0; //total maximum bin count
    };

    HistogramGenerator() = default;
    virtual ~HistogramGenerator() = default;

    const Results& getResults() const
    {
        return results_;
    }

    unsigned int currentBins() const;

    bool hasValidResult() const;
    bool subRangeActive() const { return sub_range_active_; }

    void reset();

    void update();
    bool refill();
    bool select(unsigned int bin0, unsigned int bin1);
    bool zoom(unsigned int bin0, unsigned int bin1);

    void print() const;

    virtual bool hasData() const = 0;
    
protected:
    //implements data reset
    virtual void reset_impl() = 0;

    //(optional) implements generation of histograms before refill (could also be computed on the fly in refill_impl())
    virtual bool generateHistograms_impl() { return true; }

    //implements refill behavior, which is redistribution of the data into the existing histograms
    virtual bool refill_impl() = 0;

    //(optional) implements selection if the given bin range in the data
    virtual bool select_impl(unsigned int bin0, unsigned int bin1);

    //(optional) implements zoom to a given bin range
    virtual bool zoom_impl(unsigned int bin0, unsigned int bin1);

    IntermediateData intermediate_data_; //intermediate data to be filled by derived classes
    
private:
    bool finalizeResults();

    Results results_; //result data compiled form intermediate data by this base class (derived classes do not need writing access to it)

    bool         sub_range_active_ = false;
    unsigned int num_bins_         = 0;
};
