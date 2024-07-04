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

#include "eval/results/dubious/dubiousbase.h"

namespace EvaluationRequirement
{
    class DubiousTarget;
}

namespace EvaluationRequirementResult
{

/**
*/
class SingleDubiousTarget : public SingleDubiousBase
{
public:
    SingleDubiousTarget(const std::string& result_id, 
                        std::shared_ptr<EvaluationRequirement::Base> requirement,
                        const SectorLayer& sector_layer,
                        unsigned int utn, 
                        const EvaluationTargetData* target, 
                        EvaluationManager& eval_man,
                        const EvaluationDetails& details,
                        unsigned int num_updates,
                        unsigned int num_pos_outside, 
                        unsigned int num_pos_inside, 
                        unsigned int num_pos_inside_dubious);

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    virtual std::map<std::string, std::vector<LayerDefinition>> gridLayers() const override;
    virtual void addValuesToGrid(Grid2D& grid, const std::string& layer) const override;

protected:
    EvaluationRequirement::DubiousTarget* req ();

    virtual boost::optional<double> computeResult_impl() const override;
    virtual unsigned int numIssues() const override;

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

    virtual DetailNestingMode detailNestingMode() const { return DetailNestingMode::SingleNested; } 
};

/**
*/
class JoinedDubiousTarget : public JoinedDubiousBase
{
public:
    JoinedDubiousTarget(const std::string& result_id, 
                        std::shared_ptr<EvaluationRequirement::Base> requirement,
                        const SectorLayer& sector_layer, 
                        EvaluationManager& eval_man);
protected:
    virtual unsigned int numIssues() const override;
    virtual unsigned int numUpdates() const override;

    virtual void clearResults_impl() override;
    virtual void accumulateSingleResult(const std::shared_ptr<Single>& single_result, bool first, bool last) override;
    virtual boost::optional<double> computeResult_impl() const override;

    virtual std::vector<SectorInfo> sectorInfos() const override;

private:
    unsigned int num_utns_        {0};
    unsigned int num_utns_dubious_{0};
};

}
