
#include "histogramgenerator.h"
#include "logger.h"

#include <QApplication>

/**
 */
void HistogramGenerator::setVariable(dbContent::Variable* variable)
{
    reset();

    variable_ = variable;
}

/**
 */
void HistogramGenerator::setMetaVariable(dbContent::MetaVariable* variable)
{
    reset();

    meta_variable_ = variable;
}

/**
 */
void HistogramGenerator::reset()
{
    sub_range_active_  = false;
    scanned_content_   = {};
    intermediate_data_ = {};
    results_           = {};
}

/**
 */
bool HistogramGenerator::hasValidResult() const
{
    return (config_.num_bins > 0 && 
            !results_.db_content_results.empty() &&
            results_.db_content_results.size() == intermediate_data_.size());
}

/**
 */
dbContent::Variable* HistogramGenerator::currentVariable(const std::string& db_content) const
{
    dbContent::Variable* data_var = nullptr;

    if (meta_variable_)
    {
        if (!meta_variable_->existsIn(db_content))
        {
            logwrn << "HistogramGenerator: currentVariable: meta var does not exist in dbo";
            return nullptr;
        }
        data_var = &meta_variable_->getFor(db_content);
    }
    else
    {
        data_var = variable_;

        if (data_var && data_var->dbContentName() != db_content)
            return nullptr;
    }

    return data_var;
}

/**
 */
void HistogramGenerator::updateFromBuffers(std::map<std::string, std::shared_ptr<Buffer>>& data)
{
    //reset content
    reset();

    loginf << "HistogramGenerator: Scanning buffers...";

    //scan all buffers (min-max etc.)
    for (auto& elem : data)
    {
        bool ok = scanBuffer(elem.first, *elem.second);

        if (!ok)
            logwrn << "HistogramGenerator: Could not scan buffer of DBContent " << elem.first;
    }

    loginf << "HistogramGenerator: Preparing histograms...";

    //prepare histograms
    prepareHistograms();

    loginf << "HistogramGenerator: Adding buffers...";

    //add all buffers
    for (auto& elem : data)
    {
        bool ok = addBuffer(elem.first, *elem.second);

        if (!ok)
            logwrn << "HistogramGenerator: Could not add buffer of DBContent " << elem.first;
    }

    loginf << "HistogramGenerator: Finalizing results...";

    //create results
    finalizeResults();

    loginf << "HistogramGenerator: Generated histograms!";
}

/**
 */
void HistogramGenerator::updateFromEvaluation(const std::string& eval_grpreq, const std::string& eval_id)
{
    //@TODO
}

/**
 */
bool HistogramGenerator::scanBuffer(const std::string& db_content, Buffer& buffer)
{
    if (!scanBuffer_impl(db_content, buffer))
        return false;

    //register scanned content
    scanned_content_.insert(db_content);

    return true;
}

/**
 * Selects the given sub range of bins in the data (the histograms should be regenerated afterwards).
 */
bool HistogramGenerator::select(Data& data, unsigned int bin0, unsigned int bin1)
{
    //selection needs a valid result
    if (!hasValidResult())
        return false;

    //determine min and max index
    unsigned int min_index = std::min(bin0, bin1);
    unsigned int max_index = std::max(bin0, bin1);

    //determine some options
    auto num_bins = currentBins();

    bool select_null    = max_index == num_bins;
    bool select_min_max = !(min_index == max_index && max_index == num_bins); // not b´oth num bins

    if (select_null)
        max_index -= 1;

    bool add_to_selection = QApplication::keyboardModifiers() & Qt::ControlModifier;

    bool ok = true;

    //run selection on all buffers
    for (auto& elem : data)
    {
        ok = ok && selectBuffer(elem.first, *elem.second, bin0, bin1, select_min_max, select_null, add_to_selection);
    }

    return ok;
}

/**
 * Zooms to the given subrange of bins and refills the data.
 */
bool HistogramGenerator::zoom(Data& data, unsigned int bin0, unsigned int bin1)
{
    loginf << "HistogramGenerator: Zooming to bin range...";

    if (!hasValidResult())
        return false;

    unsigned int min_index = std::min(bin0, bin1);
    unsigned int max_index = std::max(bin0, bin1);

    min_index = std::min(min_index, config_.num_bins - 1);
    max_index = std::min(max_index, config_.num_bins - 1);

    //try to rearrange the histograms using the given zoom range
    if(!zoomToSubrange(min_index, max_index))
        return false;

    sub_range_active_ = true;

    loginf << "HistogramGenerator: Readding buffers...";

    //readd all buffers
    for (auto& elem : data)
    {
        bool ok = addBuffer(elem.first, *elem.second);

        if (!ok)
            logwrn << "HistogramGenerator: Could not readd buffer of DBContent " << elem.first;
    }

    loginf << "HistogramGenerator: Finalizing results for zoom level...";

    //create results
    finalizeResults();

    loginf << "HistogramGenerator: Zooming finished!";

    return true;
}

/**
 */
void HistogramGenerator::print() const
{
    for (const auto& elem : results_.db_content_results)
    {
        const auto& r = elem.second;

        std::cout << "* DBContent = " << elem.first << std::endl;

        std::cout << "  ";
        for (const auto& bin : r.bins)
            std::cout << "[" << bin.label << ": " << bin.count << "]"; 
        std::cout << std::endl;

        std::cout << "  null:            " << r.null_count << std::endl;
        std::cout << "  null + selected: " << r.null_selected_count << std::endl;
        std::cout << "  not inserted:    " << r.not_inserted_count << std::endl;
        
        std::cout << std::endl;
    }
}
