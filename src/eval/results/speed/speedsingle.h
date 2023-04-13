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

#ifndef EVALUATIONREQUIREMENSINGLESPEED_H
#define EVALUATIONREQUIREMENSINGLESPEED_H


#include "eval/results/single.h"

#include <boost/optional.hpp>

namespace EvaluationRequirement
{
    class Speed;
}

namespace EvaluationRequirementResult
{

class SingleSpeed : public Single
{
public:
    SingleSpeed(const std::string& result_id, 
                std::shared_ptr<EvaluationRequirement::Base> requirement,
                const SectorLayer& sector_layer,
                unsigned int utn, 
                const EvaluationTargetData* target, 
                EvaluationManager& eval_man,
                const EvaluationDetails& details,
                unsigned int num_pos, 
                unsigned int num_no_ref,
                unsigned int num_pos_outside, 
                unsigned int num_pos_inside, 
                unsigned int num_no_tst_value,
                unsigned int num_comp_failed, 
                unsigned int num_comp_passed,
                vector<double> values);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    unsigned int numPos() const;
    unsigned int numNoRef() const;
    unsigned int numPosOutside() const;
    unsigned int numPosInside() const;
    unsigned int numNoTstValues() const;
    unsigned int numCompFailed() const;
    unsigned int numCompPassed() const;

    const vector<double>& values() const;

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    EvaluationRequirement::Speed* req ();

    enum DetailKey
    {
        Offset,         //float
        CheckPassed,    //bool
        PosInside,      //bool
        NumPos,         //unsigned int
        NumNoRef,       //unsigned int
        NumInside,      //unsigned int
        NumOutside,     //unsigned int
        NumCheckFailed, //unsigned int
        NumCheckPassed //unsigned int
    };

    void addAnnotations(nlohmann::json::object_t& viewable) override;

protected:
    void update();

    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::Section& section, const std::string& table_name);
    void addTargetDetailsToTableADSB (EvaluationResultsReport::Section& section, const std::string& table_name);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable ();

    unsigned int num_pos_          {0};
    unsigned int num_no_ref_       {0};
    unsigned int num_pos_outside_  {0};
    unsigned int num_pos_inside_   {0};
    unsigned int num_no_tst_value_ {0};
    unsigned int num_comp_failed_  {0};
    unsigned int num_comp_passed_  {0};

    vector<double> values_;

    double value_min_ {0};
    double value_max_ {0};
    double value_avg_ {0};
    double value_var_ {0};

    boost::optional<float> p_passed_;
};

}

#endif // EVALUATIONREQUIREMENSINGLESPEED_H
