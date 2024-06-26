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
    class DubiousTrack;
}

namespace EvaluationRequirementResult
{

/**
*/
class SingleDubiousTrack : public SingleDubiousBase
{
public:
    SingleDubiousTrack(const std::string& result_id, 
                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                       const SectorLayer& sector_layer,
                       unsigned int utn, 
                       const EvaluationTargetData* target, 
                       EvaluationManager& eval_man,
                       const EvaluationDetails& details,
                       unsigned int num_updates,
                       unsigned int num_pos_outside, 
                       unsigned int num_pos_inside, 
                       unsigned int num_pos_inside_dubious,
                       unsigned int num_tracks, 
                       unsigned int num_tracks_dubious);
    virtual ~SingleDubiousTrack();

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    virtual std::map<std::string, std::vector<LayerDefinition>> gridLayers() const override;
    virtual void addValuesToGrid(Grid2D& grid, const std::string& layer) const override;

    unsigned int numTracks() const;
    unsigned int numTracksDubious() const;

    float trackDurationAll() const;
    float trackDurationNondub() const;
    float trackDurationDubious() const;

protected:
    EvaluationRequirement::DubiousTrack* req ();

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

    virtual DetailNestingMode detailNestingMode() const { return DetailNestingMode::Nested; } 

    unsigned int num_tracks_        {0};
    unsigned int num_tracks_dubious_{0};

    mutable float track_duration_all_     {0};
    mutable float track_duration_nondub_  {0};
    mutable float track_duration_dubious_ {0};
};

}
