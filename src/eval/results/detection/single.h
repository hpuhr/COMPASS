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

#include "eval/results/probabilitybase.h"

#include "timeperiod.h"

#include <boost/optional.hpp>

namespace EvaluationRequirementResult
{

/**
*/
class SingleDetection : public SingleProbabilityBase
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
                    TimePeriodCollection ref_periods,
                    const std::vector<dbContent::TargetPosition>& ref_updates);

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    unsigned int sumUIs() const;
    unsigned int missedUIs() const;

    enum DetailKey
    {
        MissOccurred,        //bool
        DiffTOD,             //float
        RefExists,           //bool
        MissedUIs,           //unsigned int
        MaxGapUIs,           //unsigned int
        NoRefUIs,            //unsigned int
        RefUpdateStartIndex, //unsigned int
        RefUpdateEndIndex    //unsigned int
    };

    virtual std::map<std::string, std::vector<LayerDefinition>> gridLayers() const override;
    virtual void addValuesToGrid(Grid2D& grid, const std::string& layer) const override;

protected:
    virtual boost::optional<double> computeResult_impl() const override;
    virtual bool hasIssues_impl() const override;

    virtual std::vector<std::string> targetTableHeadersCustom() const override;
    virtual std::vector<QVariant> targetTableValuesCustom() const override;
    virtual std::vector<TargetInfo> targetInfos() const override;
    virtual std::vector<std::string> detailHeaders() const override;
    virtual std::vector<QVariant> detailValues(const EvaluationDetail& detail,
                                               const EvaluationDetail* parent_detail) const override;

    virtual bool detailIsOk(const EvaluationDetail& detail) const override;
    virtual void addAnnotationForDetail(nlohmann::json& annotations_json, 
                                        const EvaluationDetail& detail, 
                                        TargetAnnotationType type,
                                        bool is_ok) const override;
    unsigned int sum_uis_    {0};
    unsigned int missed_uis_ {0};

    TimePeriodCollection                   ref_periods_;
    std::vector<dbContent::TargetPosition> ref_updates_;
};

}
