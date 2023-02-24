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

#include "histogramgeneratorresults.h"
#include "histograminitializer.h"
#include "evaluationmanager.h"
#include "compass.h"

#include "eval/results/extra/datasingle.h"
#include "eval/results/extra/datajoined.h"
#include "eval/results/extra/tracksingle.h"
#include "eval/results/extra/trackjoined.h"

#include "eval/results/dubious/dubioustracksingle.h"
#include "eval/results/dubious/dubioustrackjoined.h"
#include "eval/results/dubious/dubioustargetsingle.h"
#include "eval/results/dubious/dubioustargetjoined.h"

#include "eval/results/detection/joined.h"
#include "eval/results/detection/single.h"
#include "eval/results/position/distancejoined.h"
#include "eval/results/position/distancesingle.h"
#include "eval/results/position/alongsingle.h"
#include "eval/results/position/alongjoined.h"
#include "eval/results/position/acrosssingle.h"
#include "eval/results/position/acrossjoined.h"
#include "eval/results/position/latencysingle.h"
#include "eval/results/position/latencyjoined.h"

#include "eval/results/speed/speedjoined.h"
#include "eval/results/speed/speedsingle.h"
#include "eval/results/trackangle/trackanglejoined.h"
#include "eval/results/trackangle/trackanglesingle.h"

#include "eval/results/identification/correctsingle.h"
#include "eval/results/identification/correctjoined.h"
#include "eval/results/identification/falsesingle.h"
#include "eval/results/identification/falsejoined.h"

#include "eval/results/mode_a/presentsingle.h"
#include "eval/results/mode_a/presentjoined.h"
#include "eval/results/mode_a/falsesingle.h"
#include "eval/results/mode_a/falsejoined.h"
#include "eval/results/mode_c/presentsingle.h"
#include "eval/results/mode_c/presentjoined.h"
#include "eval/results/mode_c/falsesingle.h"
#include "eval/results/mode_c/falsejoined.h"

using namespace EvaluationRequirementResult;

/**
 */
HistogramGeneratorResults::HistogramGeneratorResults(const std::string& eval_grpreq, 
                                                     const std::string& eval_id)
:   eval_grpreq_(eval_grpreq)
,   eval_id_    (eval_id    )
{
}

/**
 */
bool HistogramGeneratorResults::hasData() const
{
    //check if stored result id exists in evaluation manager
    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();
    if (!eval_man.hasResults())
        return false;

    if (eval_grpreq_.empty() || eval_id_.empty())
        return false;

    const auto& results = eval_man.results();

    // check if ids are in result
    if (!results.count(eval_grpreq_) || !results.at(eval_grpreq_).count(eval_id_))
    {
        logwrn << "HistogramGeneratorResults::refill_impl: ids set but not in results";
        return false;
    }

    return true;
}

/**
 */
void HistogramGeneratorResults::reset_impl()
{
    //clear histograms
    histograms_static_ = {};
    histograms_fp_     = {};
}

/**
 */
bool HistogramGeneratorResults::generateHistograms_impl()
{
    //nothing to do, histograms are generated on the fly
    return true;
}

/**
 */
bool HistogramGeneratorResults::refill_impl() 
{
    //reset existing histograms
    for (auto& elem : histograms_static_)
        elem.second.resetBins();
    for (auto& elem : histograms_fp_)
        elem.second.resetBins();

    //note: result validity is already checked via hasData() in base class
    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();
    const auto&        results  = eval_man.results();

    //get desired result
    std::shared_ptr<EvaluationRequirementResult::Base> result = results.at(eval_grpreq_).at(eval_id_);
    assert (result);

    //update counts
    updateFromResult(result);

    //collect data
    collectIntermediateData();

    return true;
}

/**
 */
void HistogramGeneratorResults::collectIntermediateData()
{
    intermediate_data_ = {};

    if (!histograms_fp_.empty())
        collectIntermediateData(histograms_fp_);
    else if (!histograms_static_.empty())
        collectIntermediateData(histograms_static_);
}

/**
 */
bool HistogramGeneratorResults::select_impl(unsigned int bin0, unsigned int bin1)
{
    //no result selection implemented yet
    return true;
}

/**
 */
bool HistogramGeneratorResults::zoom_impl(unsigned int bin0, unsigned int bin1)
{
    if (!histograms_fp_.empty())
    {
        return zoomHistograms<double>(histograms_fp_, bin0, bin1);
    }
    else if (!histograms_static_.empty())
    {
        return zoomHistograms<std::string>(histograms_static_, bin0, bin1);
    }
    return false;
}

/**
 */
void HistogramGeneratorResults::addStaticResult(const std::vector<std::string>& ids, 
                                                const std::vector<unsigned int>& counts)
{
    if (ids.empty())
        return;

    assert(ids.size() == counts.size());

    string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();

    //note: histograms are created on-the-fly if they do not exist, otherwise they are reused
    bool init_histogram = histograms_static_.empty();

    auto& h = histograms_static_[dbcontent_name];

    if (init_histogram)
    {
        h.createFromCategories(ids);

        //loginf << "Creating result from categories";
        //for (const auto& id : ids)
        //    loginf << "   " << id;
    }

    //loginf << "Adding values";

    for (size_t i = 0; i < ids.size(); ++i)
    {
        h.add(ids[ i ], counts[ i ]);
        //loginf << "   " << ids[ i ] << " : " << counts[ i ];
    }
}

/**
 */
void HistogramGeneratorResults::updateFromResult(std::shared_ptr<EvaluationRequirementResult::Base> result)
{
    loginf << "HistogramGeneratorResults: updateFromResult";

    if (result->type() == "SingleExtraData")
        updateCountResult(static_pointer_cast<SingleExtraData>(result));
    else if (result->type() == "JoinedExtraData")
        updateCountResult(static_pointer_cast<JoinedExtraData>(result));
    else if (result->type() == "SingleExtraTrack")
        updateCountResult(static_pointer_cast<SingleExtraTrack>(result));
    else if (result->type() == "JoinedExtraTrack")
        updateCountResult(static_pointer_cast<JoinedExtraTrack>(result));

    else if (result->type() == "SingleDubiousTrack")
        loginf << "SingleDubiousTrack not yet implemented in histogram view"; //updateCountResult(static_pointer_cast<SingleExtraTrack>(result)); TODO
    else if (result->type() == "JoinedDubiousTrack")
        loginf << "JoinedDubiousTrack not yet implemented in histogram view"; //updateCountResult(static_pointer_cast<JoinedExtraTrack>(result));
    else if (result->type() == "SingleDubiousTarget")
        loginf << "SingleDubiousTarget not yet implemented in histogram view"; //updateCountResult(static_pointer_cast<SingleExtraTrack>(result)); TODO
    else if (result->type() == "JoinedDubiousTarget")
        loginf << "JoinedDubiousTarget not yet implemented in histogram view"; //updateCountResult(static_pointer_cast<JoinedExtraTrack>(result));
    
    else if (result->type() == "SingleDetection")
        updateCountResult(static_pointer_cast<SingleDetection>(result));
    else if (result->type() == "JoinedDetection")
        updateCountResult(static_pointer_cast<JoinedDetection>(result));
    else if (result->type() == "SinglePositionDistance")
        updateCountResult(static_pointer_cast<SinglePositionDistance>(result));
    else if (result->type() == "JoinedPositionDistance")
        updateCountResult(static_pointer_cast<JoinedPositionDistance>(result));
    else if (result->type() == "SinglePositionAlong")
        updateCountResult(static_pointer_cast<SinglePositionAlong>(result));
    else if (result->type() == "JoinedPositionAlong")
        updateCountResult(static_pointer_cast<JoinedPositionAlong>(result));
    else if (result->type() == "SinglePositionAcross")
        updateCountResult(static_pointer_cast<SinglePositionAcross>(result));
    else if (result->type() == "JoinedPositionAcross")
        updateCountResult(static_pointer_cast<JoinedPositionAcross>(result));
    else if (result->type() == "SinglePositionLatency")
        updateCountResult(static_pointer_cast<SinglePositionLatency>(result));
    else if (result->type() == "JoinedPositionLatency")
        updateCountResult(static_pointer_cast<JoinedPositionLatency>(result));

    else if (result->type() == "SingleSpeed")
        updateCountResult(static_pointer_cast<SingleSpeed>(result));
    else if (result->type() == "JoinedSpeed")
        updateCountResult(static_pointer_cast<JoinedSpeed>(result));
    else if (result->type() == "SingleTrackAngle")
        updateCountResult(static_pointer_cast<SingleTrackAngle>(result));
    else if (result->type() == "JoinedTrackAngle")
        updateCountResult(static_pointer_cast<JoinedTrackAngle>(result));

    else if (result->type() == "SingleIdentificationCorrect")
        updateCountResult(static_pointer_cast<SingleIdentificationCorrect>(result));
    else if (result->type() == "JoinedIdentificationCorrect")
        updateCountResult(static_pointer_cast<JoinedIdentificationCorrect>(result));
    else if (result->type() == "SingleIdentificationFalse")
        updateCountResult(static_pointer_cast<SingleIdentificationFalse>(result));
    else if (result->type() == "JoinedIdentificationFalse")
        updateCountResult(static_pointer_cast<JoinedIdentificationFalse>(result));

    else if (result->type() == "SingleModeAPresent")
        updateCountResult(static_pointer_cast<SingleModeAPresent>(result));
    else if (result->type() == "JoinedModeAPresent")
        updateCountResult(static_pointer_cast<JoinedModeAPresent>(result));
    else if (result->type() == "SingleModeAFalse")
        updateCountResult(static_pointer_cast<SingleModeAFalse>(result));
    else if (result->type() == "JoinedModeAFalse")
        updateCountResult(static_pointer_cast<JoinedModeAFalse>(result));

    else if (result->type() == "SingleModeCPresent")
        updateCountResult(static_pointer_cast<SingleModeCPresent>(result));
    else if (result->type() == "JoinedModeCPresent")
        updateCountResult(static_pointer_cast<JoinedModeCPresent>(result));
    else if (result->type() == "SingleModeCFalse")
        updateCountResult(static_pointer_cast<SingleModeCFalse>(result));
    else if (result->type() == "JoinedModeCFalse")
        updateCountResult(static_pointer_cast<JoinedModeCFalse>(result));
    else
        throw runtime_error("HistogramGeneratorResults: updateFromResult: unknown result type '"+result->type()+"'");
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleExtraData> result)
{
    logdbg << "HistogramGeneratorResults::updateCountResult: single extra data";

    assert (result);

    addStaticResult({ "#Check", 
                      "#OK", 
                      "#Extra" }, 
                    { result->numOK() + result->numExtra(), 
                      result->numOK(), 
                      result->numExtra() });
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedExtraData> result)
{
    logdbg << "HistogramGeneratorResults: showResult: joined extra data";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleExtraData>(result_it));

        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleExtraData>(result_it));
    }
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleExtraTrack> result)
{
    logdbg << "HistogramGeneratorResults::updateCountResult: single extra track";

    assert (result);

    addStaticResult({ "#Check", 
                      "#OK", 
                      "#Extra" }, 
                    { result->numOK() + result->numExtra(), 
                      result->numOK(), 
                      result->numExtra() });
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedExtraTrack> result)
{
    logdbg << "HistogramGeneratorResults: showResult: joined track";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleExtraTrack>(result_it));

        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleExtraTrack>(result_it));
    }
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleDetection> result)
{
    logdbg << "HistogramGeneratorResults: showResult: single detection";

    assert (result);

    addStaticResult({ "#EUIs", 
                      "#MUIs" }, 
                    { (unsigned int)result->sumUIs(), 
                      (unsigned int)result->missedUIs() });
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedDetection> result)
{
    logdbg << "HistogramGeneratorResults: showResult: joined detection";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleDetection>(result_it));

        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleDetection>(result_it));
    }
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<SinglePositionDistance> result)
{
    assert (result);
    std::string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    addFloatingPointResult(dbcontent_name, result);
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<JoinedPositionDistance> result)
{
    assert (result);
    std::string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    addFloatingPointResults<JoinedPositionDistance, SinglePositionDistance, Base>(dbcontent_name, result);
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<SinglePositionAlong> result)
{
    assert (result);
    std::string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    addFloatingPointResult(dbcontent_name, result);
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<JoinedPositionAlong> result)
{
    assert (result);
    std::string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    addFloatingPointResults<JoinedPositionAlong, SinglePositionAlong, Base>(dbcontent_name, result);
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<SinglePositionAcross> result)
{
    assert (result);
    std::string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    addFloatingPointResult(dbcontent_name, result);
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<JoinedPositionAcross> result)
{
    assert (result);
    std::string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    addFloatingPointResults<JoinedPositionAcross, SinglePositionAcross, Base>(dbcontent_name, result);
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<SinglePositionLatency> result)
{
    assert (result);
    std::string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    addFloatingPointResult(dbcontent_name, result);
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<JoinedPositionLatency> result)
{
    assert (result);
    std::string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    addFloatingPointResults<JoinedPositionLatency, SinglePositionLatency, Base>(dbcontent_name, result);
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<SingleSpeed> result)
{
    assert (result);
    std::string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    addFloatingPointResult(dbcontent_name, result);
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<JoinedSpeed> result)
{
    assert (result);
    std::string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    addFloatingPointResults<JoinedSpeed, SingleSpeed, Base>(dbcontent_name, result);
}

void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<SingleTrackAngle> result)
{
    assert (result);
    std::string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    addFloatingPointResult(dbcontent_name, result);
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<JoinedTrackAngle> result)
{
    assert (result);
    std::string dbcontent_name = COMPASS::instance().evaluationManager().dbContentNameTst();
    addFloatingPointResults<JoinedTrackAngle, SingleTrackAngle, Base>(dbcontent_name, result);
}


/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SingleIdentificationCorrect> result)
{
    logdbg << "HistogramGeneratorResults: showResult: single identification correct";

    assert (result);

    addStaticResult({ "#NoRef", 
                      "#CID", 
                      "#NCID" }, 
                    { result->numNoRefId(), 
                      result->numCorrect(), 
                      result->numNotCorrect() });
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::JoinedIdentificationCorrect> result)
{
    logdbg << "HistogramGeneratorResults: updateFromResult: joined identification correct";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleIdentificationCorrect>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleIdentificationCorrect>(result_it));
    }
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::SingleIdentificationFalse> result)
{
    logdbg << "HistogramGeneratorResults: showResult: single identification false";

    assert (result);

    addStaticResult({ "#NoRef", 
                      "#Unknown", 
                      "#Correct",
                      "#False" }, 
                    { (unsigned int)result->numNoRefValue(), 
                      (unsigned int)result->numUnknown(), 
                      (unsigned int)result->numCorrect(),
                      (unsigned int)result->numFalse() });
}

/**
 */
void HistogramGeneratorResults::updateCountResult (
        std::shared_ptr<EvaluationRequirementResult::JoinedIdentificationFalse> result)
{
    logdbg << "HistogramGeneratorResults: updateFromResult: joined identification false";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleIdentificationFalse>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleIdentificationFalse>(result_it));
    }
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeAPresent> result)
{
    logdbg << "HistogramGeneratorResults: showResult: single mode a present";

    assert (result);

    addStaticResult({ "#NoRefId", 
                      "#Present", 
                      "#Missing"}, 
                    { (unsigned int)result->numNoRefId(), 
                      (unsigned int)result->numPresent(), 
                      (unsigned int)result->numMissing() });
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeAPresent> result)
{
    logdbg << "HistogramGeneratorResults: showResult: joined mode 3/a present";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleModeAPresent>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleModeAPresent>(result_it));
    }
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeAFalse> result)
{
    logdbg << "HistogramGeneratorResults: showResult: single mode a false";

    assert (result);

    addStaticResult({ "#NoRef", 
                      "#Unknown", 
                      "#Correct",
                      "#False" }, 
                    { (unsigned int)result->numNoRefValue(), 
                      (unsigned int)result->numUnknown(), 
                      (unsigned int)result->numCorrect(),
                      (unsigned int)result->numFalse() });
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeAFalse> result)
{
    logdbg << "HistogramGeneratorResults: showResult: joined mode 3/a false";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleModeAFalse>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleModeAFalse>(result_it));
    }
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeCPresent> result)
{
    logdbg << "HistogramGeneratorResults: showResult: single mode a present";

    assert (result);

    addStaticResult({ "#NoRefC", 
                      "#Present", 
                      "#Missing" }, 
                    { (unsigned int)result->numNoRefC(), 
                      (unsigned int)result->numPresent(), 
                      (unsigned int)result->numMissing() });
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeCPresent> result)
{
    logdbg << "HistogramGeneratorResults: showResult: joined mode 3/a present";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleModeCPresent>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleModeCPresent>(result_it));
    }
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::SingleModeCFalse> result)
{
    logdbg << "HistogramGeneratorResults: showResult: single mode c";

    assert (result);

    addStaticResult({ "#NoRef", 
                      "#Unknown", 
                      "#Correct",
                      "#False" }, 
                    { (unsigned int)result->numNoRefValue(), 
                      (unsigned int)result->numUnknown(), 
                      (unsigned int)result->numCorrect(),
                      (unsigned int)result->numFalse() });
}

/**
 */
void HistogramGeneratorResults::updateCountResult (std::shared_ptr<EvaluationRequirementResult::JoinedModeCFalse> result)
{
    logdbg << "HistogramGeneratorResults: showResult: joined mode c";

    assert (result);

    std::vector<std::shared_ptr<Base>>& results = result->results();

    for (auto& result_it : results)
    {
        assert (static_pointer_cast<SingleModeCFalse>(result_it));
        if (result_it->use())
            updateCountResult (static_pointer_cast<SingleModeCFalse>(result_it));
    }
}


