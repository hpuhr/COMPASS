
#pragma once

#include "global.h"
#include "nullablevector.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent.h"
#include "buffer.h"
#include "histogram.h"

#include <vector>
#include <map>
#include <set>
#include <string>
#include <type_traits>

#include <boost/date_time.hpp>
#include <boost/optional.hpp>

/**
 * Base class for histogram generation from buffers or evaluation result data.
 * Hides concrete used data types from the outside.
 */
class HistogramGenerator
{
public:
    static const unsigned int DefaultNumBins = 20;

    /**
     * Histogram generation type.
     */
    enum class HistogramType
    {
        Range = 0, //the histogram is generated from a certain data range and a number of equally sized bins inbetween
        Category   //the histogram is generated from a fixed number of values that need to match (categories)
    };

    /**
     * Current histogram configuration.
     */
    struct Config
    {
        HistogramType type     = HistogramType::Range; //histogram generation type
        unsigned int  num_bins = DefaultNumBins;       //number of histogram bins to be generated
    };

    /**
     * Additional data tracked for each histogram bin.
     */
    struct AdditionBinData
    {
        unsigned int count    = 0;
        unsigned int selected = 0; //number of selected data values in bin
    };

    /**
     * Used to keep track of intermediate values
     */
    struct IntermediateData
    {
        void init(size_t n = 0)
        {
            *this = {};

            if (n > 0)
                bin_data.assign(n, {});
        }

        std::vector<AdditionBinData> bin_data; //additional per bin data

        unsigned int null_count          = 0; //number of encountered null values
        unsigned int null_selected_count = 0; //number of encountered selected null values
        unsigned int not_inserted_count  = 0; //number of encountered non-insertable values (e.g. out of histogram range)
    };

    typedef std::map<std::string, IntermediateData> DBContentIntermediateData;

    /**
     * Result bin.
     */
    struct Bin
    {
        unsigned int count    = 0; //number of data points in this bin
        unsigned int selected = 0; //number of selected data points in this bin
        std::string  label;        //label describing this bin
    };

    /**
     * Result for a certain DBContent.
     */
    struct Result
    {
        std::vector<Bin> bins;                    //resulting histogram bins
        unsigned int     valid_count         = 0; //number of valid data values in result
        unsigned int     selected_count      = 0; //number of selected data values in result
        unsigned int     null_count          = 0; //number of encountered null values in result
        unsigned int     null_selected_count = 0; //number of encountered selected null values in result
        unsigned int     not_inserted_count  = 0; //number of encountered non-insertable values in result (e.g. out of histogram range)
        unsigned int     max_count           = 0; //maximum encountered bin count
    };

    typedef std::map<std::string, Result> DBContentResults; 

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
        
        DBContentResults          db_content_results;      //results per DBContent
        std::vector<unsigned int> valid_counts;            //valid data per bin (over multiple db contents)
        std::vector<unsigned int> selected_counts;         //selected data per bin (over multiple db contents)
        unsigned int              valid_count         = 0; //total valid data count
        unsigned int              selected_count      = 0; //total selected valid data    
        unsigned int              null_count          = 0; //total null data
        unsigned int              null_selected_count = 0; //total selected null data 
        unsigned int              not_inserted_count  = 0; //total non-insertable data (e.g. out of histogram range)
        unsigned int              max_count           = 0; //total maximum bin count
    };

    typedef std::map<std::string, std::shared_ptr<Buffer>> Data;

    HistogramGenerator() = default;
    virtual ~HistogramGenerator() = default;

    const Results& getResults() const
    {
        return results_;
    }

    unsigned int currentBins() const 
    {
        return config_.num_bins;
    }

    bool hasValidResult() const;
    bool subRangeActive() const { return sub_range_active_; }

    void setBufferData(Data* data);
    void setVariable(dbContent::Variable* variable);
    void setMetaVariable(dbContent::MetaVariable* variable);

    virtual void reset();

    void updateFromBufferData();
    void updateFromEvaluation(const std::string& eval_grpreq, const std::string& eval_id);

    bool refill();
    bool select(unsigned int bin0, unsigned int bin1);
    bool zoom(unsigned int bin0, unsigned int bin1);

    void print() const;
    
protected:
    virtual void prepareForRefill();
    virtual bool scanBuffer_impl(const std::string& db_content, Buffer& buffer) = 0;
    virtual bool prepareHistograms() = 0;
    virtual bool addBuffer(const std::string& db_content, Buffer& buffer) = 0;
    virtual bool finalizeResults() = 0;
    virtual bool selectBuffer(const std::string& db_content, 
                              Buffer& buffer, 
                              unsigned int bin0, 
                              unsigned int bin1,
                              bool select_min_max,
                              bool select_null, 
                              bool add_to_selection) = 0;
    virtual bool zoomToSubrange(unsigned int bin0, unsigned int bin1) = 0;

    dbContent::Variable* currentVariable(const std::string& db_content) const;

    std::set<std::string>     scanned_content_;
    DBContentIntermediateData intermediate_data_;
    Results                   results_; 
    Config                    config_;
    
private:
    bool scanBuffer(const std::string& db_content, Buffer& buffer);

    Data*                    buffer_data_   = nullptr; //governed buffer data
    dbContent::Variable*     variable_      = nullptr; //governed variable
    dbContent::MetaVariable* meta_variable_ = nullptr; //governed meta-variable

    bool sub_range_active_ = false;
};

/**
 * Histogram generator specialized on the governed variables data type.
 */
template <typename T>
class HistogramGeneratorT : public HistogramGenerator
{
public:
    HistogramGeneratorT() = default;
    virtual ~HistogramGeneratorT() = default;

    /**
     */
    void reset() override final
    {
        //don't forget to invoke base
        HistogramGenerator::reset();

        histograms_.clear();

        distinct_values_ = {};
        data_min_        = {};
        data_max_        = {};
    }

protected:
    /**
     * Returns the buffers distinct (unique) values.
     */
    std::set<T> distinctValues(NullableVector<T>& data) const
    {
        //extract distinct values by default
        return data.distinctValues();
    }

    /**
     * Scans the buffer and extracts needed data for histogram generation.
     */
    bool scanBuffer_impl(const std::string& db_content, Buffer& buffer) override final
    {
        auto variable = currentVariable(db_content);
        if (!variable)
            return false;

        std::string current_var_name = variable->name();

        if (!buffer.has<T>(current_var_name))
            return false;

        NullableVector<T>& data = buffer.get<T>(current_var_name);
        
        //keep track of min max values
        bool min_max_set = true;
        T data_min, data_max;

        std::tie(min_max_set, data_min, data_max) = data.minMaxValues();

        if (!min_max_set)
            return false;
            
        if (!data_min_.has_value() || data_min < data_min_.value())
            data_min_ = data_min;
        if (!data_max_.has_value() || data_max > data_max_.value())
            data_max_ = data_max;

        //collect distinct values if supported/desired for template type
        if (!std::is_floating_point<T>::value &&
            !std::is_same<boost::posix_time::ptime, T>::value)
        {
            auto distinct_values = distinctValues(data);
            if (!distinct_values.empty())
            {
                if (distinct_values_.has_value())
                    distinct_values_.value().insert(distinct_values.begin(), distinct_values.end());
                else
                    distinct_values_ = distinct_values;
            }
        }
        
        return true;
    }

    /**
     * Generates a configuration depending on the collected information (e.g. distinct values, extrema, etc.).
     */
    Config generateConfig() const
    {
        //no data range or no distinct values stored -> return default config
        if (!data_min_.has_value() || 
            !data_max_.has_value() || 
            !distinct_values_.has_value() || 
             distinct_values_.value().empty())
            return Config();

        //default config also for floating point numbers and timestamps
        if (std::is_floating_point<T>::value ||
            std::is_same<boost::posix_time::ptime, T>::value)
            return Config();

        Config config;

        unsigned int num_distinct_values = distinct_values_.value().size();

        //distinct values in data less than bins -> reduce bins and generate histogram from discrete categories
        if (std::is_same<T, std::string>::value || num_distinct_values < config.num_bins)
        {
            config.num_bins = num_distinct_values;
            config.type     = HistogramType::Category;
        }

        return config;
    }

    /**
     * Prepares the data structures for a refill.
     */
    void prepareForRefill() override final
    {
        HistogramGenerator::prepareForRefill();

        //reset histograms
        for (auto& elem : histograms_)
            elem.second.resetBins();

        initIntermediateData();
    }

    /**
     * Inits the intermediate data structures based on the current configuration.
     */
    void initIntermediateData()
    {
        intermediate_data_ = {};

        for (auto& elem : scanned_content_)
        {
            auto& d = intermediate_data_[ elem ];
            d.init(config_.num_bins);
        }
    }

    /**
     * Prepares the histogram for data collection.
     */
    bool prepareHistograms() override final
    {
        config_            = Config();
        intermediate_data_ = {};
        histograms_        = {};
        results_           = {};

        //no data range available -> no good
        if (!data_min_.has_value() || !data_max_.has_value())
            return false;

        //generate histogram creation config
        config_ = generateConfig();

        //no previously scanned buffers?
        if (scanned_content_.empty())
            return true;

        //for all previously scanned buffers...
        for (auto& elem : scanned_content_)
        {  
            auto& h = histograms_[ elem ];

            //initialize histogram using config
            if (config_.type == HistogramType::Range)
            {
                h.createFromRange(config_.num_bins, data_min_.value(), data_max_.value());
            }
            else
            {
                std::vector<T> categories(distinct_values_.value().begin(), distinct_values_.value().end());
                h.createFromCategories(categories);
            }
        }

        //update number of bins of config to actual bins
        config_.num_bins = (int)histograms_.rbegin()->second.numBins();

        //initialize intermediate data
        initIntermediateData();
            
        return true;
    }

    /**
     * Add buffer content to histogram.
     */
    bool addBuffer(const std::string& db_content, Buffer& buffer) override final
    {
        //adding buffer needs previously initialized result and histogram for dbcontent type
        if (intermediate_data_.find(db_content) == intermediate_data_.end() ||
            histograms_.find(db_content) == histograms_.end())
            return false;

        auto variable = currentVariable(db_content);

        //no variable set?
        if (!variable)
            return false;

        //no data range?
        if (!data_min_.has_value() || !data_max_.has_value())
            return false;

        std::string current_var_name = variable->name();

        //buffer does not obtain variable?
        if (!buffer.has<T>(current_var_name))
            return false;

        NullableVector<T>& data = buffer.get<T>(current_var_name);

        assert (buffer.has<bool>(DBContent::selected_var.name()));
        NullableVector<bool>& selected_vec = buffer.get<bool>(DBContent::selected_var.name());

        auto& histogram   = histograms_       [ db_content ];
        auto& interm_data = intermediate_data_[ db_content ];

        //histogram badly configured?
        if (histogram.numBins() < 1)
            return false;

        //add variable content
        for (unsigned int cnt=0; cnt < data.size(); ++cnt)
        {
            bool selected = !selected_vec.isNull(cnt) && selected_vec.get(cnt);
            bool is_null  = data.isNull(cnt);

            //value null?
            if (is_null)
            {
                if (selected)
                {
                    ++interm_data.null_selected_count;
                }
                else 
                {
                    ++interm_data.null_count;
                }
                continue;
            }

            //find bin
            //@TODO: we use the histogram as a bin finder and store the counts externally,
            //but we could also add some "extra data" to each histogram bin in the future,
            //in order to track multiple per-bin counts inside the histogram.
            int bin_idx = histogram.findBin(data.get(cnt));

            if (bin_idx < 0)   // is non-insertable?
            {
                ++interm_data.not_inserted_count;
            }
            else if (selected) // is selected?
            {
                ++interm_data.bin_data.at(bin_idx).selected;
            }
            else //just your typical valid-unselected-joe
            {
                ++interm_data.bin_data.at(bin_idx).count;
            }
        }

        return true;
    }

    /**
     * Extracts histogram data and stores it to the results.
     */
    bool finalizeResults() override final
    {
        results_ = {};

        results_.valid_counts.assign(config_.num_bins, 0);
        results_.selected_counts.assign(config_.num_bins, 0);

        for (auto& elem : intermediate_data_)
        {
            auto variable = currentVariable(elem.first);

            const auto& h = histograms_[ elem.first ];
            const auto& d = elem.second;
            auto&       r = results_.db_content_results[ elem.first ];

            r.bins.resize(h.numBins());

            r.null_count          = d.null_count;
            r.null_selected_count = d.null_selected_count;
            r.not_inserted_count  = d.not_inserted_count;

            for (size_t i = 0; i < h.numBins(); ++i)
            {
                const auto& h_bin    = h.getBin(i);
                const auto& bin_data = d.bin_data.at(i);
                auto&       res_bin  = r.bins.at(i);

                //extract bin data
                res_bin.count    = bin_data.count;
                res_bin.selected = bin_data.selected;

                //extract label
                if (variable)
                    res_bin.label = h_bin.label(variable);

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
        }

        assert(subRangeActive() || results_.not_inserted_count == 0);

        return true;
    }

    /**
     */
    bool selectBuffer(const std::string& db_content, 
                      Buffer& buffer,
                      unsigned int bin0, 
                      unsigned int bin1,
                      bool select_min_max,
                      bool select_null, 
                      bool add_to_selection) override final
    {
        //no histogram yet for db content?
        auto it = histograms_.find(db_content);
        if (it == histograms_.end())
            return true;
        
        //no variable?
        auto variable = currentVariable(db_content);
        if (!variable)
            return false;

        std::string current_var_name = variable->name();

        //buffer does not obtain variable?
        if (!buffer.has<T>(current_var_name))
            return false;

        NullableVector<T>& data = buffer.get<T>(current_var_name);

        //selected vector?
        assert (buffer.has<bool>(DBContent::selected_var.name()));
        NullableVector<bool>& selected_vec = buffer.get<bool>(DBContent::selected_var.name());

        unsigned int select_cnt = 0;

        for (unsigned int cnt=0; cnt < data.size(); ++cnt)
        {
            //check null case
            if (data.isNull(cnt))
            {
                if (select_null)
                {
                    selected_vec.set(cnt, true);
                    ++select_cnt;
                }
                else if (!add_to_selection)
                {
                    selected_vec.set(cnt, false);
                }
                continue;
            }

            if (!select_min_max)
            {
                //leave value "as is"
                if (!add_to_selection || selected_vec.isNull(cnt))
                    selected_vec.set(cnt, false);
                continue;
            }

            //find bin for data
            int bin_idx = it->second.findBin(data.get(cnt));
            if (bin_idx < 0)
                continue;

            //bin inside selection range?
            bool select = ((unsigned int)bin_idx >= bin0 && (unsigned int)bin_idx <= bin1);
            if (!select && add_to_selection && !selected_vec.isNull(cnt))
                select = selected_vec.get(cnt);

            selected_vec.set(cnt, select);

            if (select)
                ++select_cnt;
        }

        loginf << "HistogramGeneratorT::selectBuffer: content = " << db_content << ", selected " << select_cnt;

        return true;
    }

    /**
     * Rearranges the bins to show the given subrange of bins.
     * This works a little like prepareHistograms(), but reuses the current configuration to init the new histograms.
     */
    bool zoomToSubrange(unsigned int bin0, unsigned int bin1) override final
    {
        if (!hasValidResult())
            return false;

        for (auto& elem : histograms_)
        {
            auto& h = elem.second;

            if (config_.type == HistogramType::Category)
            {
                //recreate histograms from subcategories 
                std::vector<T> categories;
                for (unsigned int i = bin0; i <= bin1; ++i)
                    categories.push_back(h.getBin(i).min_value);

                h.createFromCategories(categories);

                config_.num_bins = h.numBins();
            }
            else // HistogramType::Range
            {
                //recreate histogram from subrange
                const auto& b0 = h.getBin(bin0);
                const auto& b1 = h.getBin(bin1);

                auto min_value = b0.min_value;
                auto max_value = b1.max_value;

                h.createFromRange(DefaultNumBins, min_value, max_value);

                config_.num_bins = h.numBins();
            }
        }

        //reinit the intermediate data to new histogram size
        initIntermediateData();

        return true;
    }

private:
    boost::optional<T>           data_min_;
    boost::optional<T>           data_max_;
    boost::optional<std::set<T>> distinct_values_;

    std::map<std::string, HistogramT<T>> histograms_;
};

/**
 * No distinct value generation for these data types.
 */
template<>
inline std::set<boost::posix_time::ptime> HistogramGeneratorT<boost::posix_time::ptime>::distinctValues(NullableVector<boost::posix_time::ptime>& data) const
{
    return {};
}
