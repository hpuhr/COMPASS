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

#ifndef EVALUATIONREQUIREMENTDETECTIONRESULT_H
#define EVALUATIONREQUIREMENTDETECTIONRESULT_H

#include "eval/results/single.h"
#include "timeperiod.h"

#include <boost/optional.hpp>

namespace EvaluationRequirementResult
{

class SingleDetection : public Single
{
public:
    SingleDetection(const std::string& result_id, 
                    std::shared_ptr<EvaluationRequirement::Base> requirement,
                    const SectorLayer& sector_layer, 
                    unsigned int utn, 
                    const EvaluationTargetData* target,
                    EvaluationManager& eval_man,
                    const EvaluationDetails& details,
                    int sum_uis, 
                    int missed_uis, 
                    TimePeriodCollection ref_periods);

    //virtual void print() override;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    int sumUIs() const;
    int missedUIs() const;

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    enum DetailKey
    {
        MissOccurred, //bool
        DiffTOD,      //float
        RefExists,    //bool
        MissedUIs,    //unsigned int
        MaxGapUIs,    //unsigned int
        NoRefUIs     //unsigned int
    };

    void addAnnotations(nlohmann::json::object_t& viewable) override;

    bool hasFailed() const;

protected:
    void updatePD();

    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::Section& section, const std::string& table_name);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable ();

    int sum_uis_    {0};
    int missed_uis_ {0};

    TimePeriodCollection ref_periods_;

    boost::optional<float> pd_;
};

}

#endif // EVALUATIONREQUIREMENTDETECTIONRESULT_H
