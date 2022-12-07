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

#include "histogramgenerator.h"
#include "histogram.h"
#include "histograminitializer.h"

#include <string>
#include <map>

namespace EvaluationRequirementResult
{
    class Base;
    class SingleExtraData;
    class JoinedExtraData;

    class SingleExtraTrack;
    class JoinedExtraTrack;

    class SingleDubiousTrack;
    class JoinedDubiousTrack;

    class SingleDubiousTarget;
    class JoinedDubiousTarget;

    class SingleDetection;
    class JoinedDetection;

    class SinglePositionDistance;
    class JoinedPositionDistance;

    class SinglePositionAlong;
    class JoinedPositionAlong;

    class SinglePositionAcross;
    class JoinedPositionAcross;

    class SinglePositionLatency;
    class JoinedPositionLatency;

    class SingleSpeed;
    class JoinedSpeed;

    class SingleIdentificationCorrect;
    class JoinedIdentificationCorrect;
    class SingleIdentificationFalse;
    class JoinedIdentificationFalse;

    class SingleModeAPresent;
    class JoinedModeAPresent;
    class SingleModeAFalse;
    class JoinedModeAFalse;

    class SingleModeCPresent;
    class JoinedModeCPresent;
    class SingleModeCFalse;
    class JoinedModeCFalse;
}

/**
 * Evaluation result based histogram generator.
 */
class HistogramGeneratorResults : public HistogramGenerator 
{
public:
    HistogramGeneratorResults(const std::string& eval_grpreq, const std::string& eval_id);
    virtual ~HistogramGeneratorResults() = default;

    virtual bool hasData() const override final;

protected:
    virtual void reset_impl() override final;
    virtual bool generateHistograms_impl() override final;
    virtual bool refill_impl() override final;
    virtual bool select_impl(unsigned int bin0, unsigned int bin1) override final;
    virtual bool zoom_impl(unsigned int bin0, unsigned int bin1) override final;

private:
    void collectIntermediateData();
    void updateFromResult(std::shared_ptr<EvaluationRequirementResult::Base> result);

    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleExtraData> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedExtraData> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleExtraTrack> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedExtraTrack> result);

    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleDetection> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedDetection> result);

    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SinglePositionDistance> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedPositionDistance> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SinglePositionAlong> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedPositionAlong> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SinglePositionAcross> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedPositionAcross> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SinglePositionLatency> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedPositionLatency> result);

    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleSpeed> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedSpeed> result);

    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleIdentificationCorrect> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedIdentificationCorrect> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleIdentificationFalse> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedIdentificationFalse> result);

    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeAPresent> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeAPresent> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeAFalse> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeAFalse> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeCPresent> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeCPresent> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeCFalse> result);
    void updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeCFalse> result);

    void addStaticResult(const std::vector<std::string>& ids, const std::vector<unsigned int>& counts);

    /**
     */
    template<typename T>
    void collectIntermediateData(const std::string& db_content, const HistogramT<T>& histogram)
    {
        auto& data = intermediate_data_[ db_content ];
        data.bin_data.resize(histogram.numBins());

        data.not_inserted_count = histogram.unassignedCount();
        data.bins_are_sorted    = histogram.configuration().sorted_bins;

        for (size_t i = 0; i < histogram.numBins(); ++i)
        {
            const auto& bin  = histogram.getBin(i);
            auto&       dbin = data.bin_data[ i ];

            dbin.count = bin.count;
            dbin.labels.label     = bin.label(nullptr);
            dbin.labels.label_min = bin.labelMin(nullptr);
            dbin.labels.label_max = bin.labelMax(nullptr);
        }
    }

    /**
     */
    template<typename T>
    void collectIntermediateData(const std::map<std::string, HistogramT<T>>& histograms)
    {
        for (const auto& elem : histograms)
            collectIntermediateData(elem.first, elem.second);
    }

    /**
     */
    template<typename T>
    bool zoomHistogram(HistogramT<T>& histogram,
                       unsigned int bin0, 
                       unsigned int bin1)
    {
        return histogram.zoom(bin0, bin1);
    }

    /**
     */
    template<typename T>
    bool zoomHistograms(std::map<std::string, HistogramT<T>>& histograms, 
                        unsigned int bin0, 
                        unsigned int bin1)
    {
        if (!hasValidResult())
            return false;

        for (auto& elem : histograms)
            if (!zoomHistogram(elem.second, bin0, bin1))
                return false;

        return true;
    }

    /**
     */
    template<typename T>
    void addFloatingPointResult(const std::string& dbcontent_name, std::shared_ptr<T> result)
    {
        const std::vector<double>& values = result->values();

        //note: histograms are created on-the-fly if they do not exist, otherwise they are reused
        bool init_histogram = histograms_fp_.empty();

        auto& h = histograms_fp_[ dbcontent_name ];

        //init histogram?
        if (init_histogram)
        {
            //loginf << "Creating histogram from flotaing point values";

            HistogramInitializer<double> init;
            init.scan(values);

            auto config = init.currentConfiguration();
            init.initHistogram(h, config);
        }

        //loginf << "Adding values";
        
        //add result data
        h.add(values);

        //for (auto v : values)
        //    loginf << "   " << v;
    }

    /**
     */
    template<typename T, typename Tsub, typename Tbase>
    void addFloatingPointResults(const std::string& dbcontent_name, std::shared_ptr<T> result)
    {
        std::vector<std::shared_ptr<Tbase>>& results = result->results();

        //note: histograms are created on-the-fly if they do not exist, otherwise they are reused
        bool init_histogram = histograms_fp_.empty();

        auto& h = histograms_fp_[ dbcontent_name ];

        //init histogram?
        if (init_histogram)
        {
            //loginf << "Creating histogram from flotaing point values";

            HistogramInitializer<double> init;

            for (auto& result_it : results) // calculate global min max
            {
                std::shared_ptr<Tsub> single_result = std::static_pointer_cast<Tsub>(result_it);

                if (single_result->use())
                    init.scan(single_result->values());
            }

            auto config = init.currentConfiguration();
            init.initHistogram(h, config);
        }

        //loginf << "Adding values";

        //add result data
        for (auto& result_it : results)
        {
            std::shared_ptr<Tsub> single_result = std::static_pointer_cast<Tsub>(result_it);

            if (single_result->use())
            {
                h.add(single_result->values());

                //for (auto v : single_result->values())
                //    loginf << "   " << v;
            }
        }
    }

    std::string eval_grpreq_;
    std::string eval_id_;

    //@TODO: this could get us in trouble if we need more and more types of histograms
    //how to integrate this more gracefully?
    // -> maybe generate the needed histograms on the fly and do not even hold them as members?
    std::map<std::string, HistogramT<std::string>> histograms_static_; //static category histogram
    std::map<std::string, HistogramT<double>>      histograms_fp_;     //floating point histogram
};
