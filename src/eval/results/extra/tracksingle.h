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

#include <boost/optional.hpp>

namespace EvaluationRequirementResult
{

/**
*/
class SingleExtraTrack : public SingleProbabilityBase
{
public:
    SingleExtraTrack(const std::string& result_id, 
                     std::shared_ptr<EvaluationRequirement::Base> requirement,
                     const SectorLayer& sector_layer, 
                     unsigned int utn, 
                     const EvaluationTargetData* target,
                     EvaluationManager& eval_man,
                     const EvaluationDetails& details,
                     bool ignore, 
                     unsigned int num_inside, 
                     unsigned int num_extra,  
                     unsigned int num_ok);

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    virtual std::map<std::string, std::vector<LayerDefinition>> gridLayers() const override;
    virtual void addValuesToGrid(Grid2D& grid, const std::string& layer) const override;

    unsigned int numInside() const;
    unsigned int numExtra() const;
    unsigned int numOK() const;

    enum DetailKey
    {
        TrackNum, //unsigned int
        Inside,   //bool
        Extra    //bool
    };

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
    unsigned int num_inside_ {0};
    unsigned int num_extra_  {0};
    unsigned int num_ok_     {0};
};

}
