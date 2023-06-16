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

#ifndef EVALUATIONREQUIREMENSINGLEDUBIOSTRACK_H
#define EVALUATIONREQUIREMENSINGLEDUBIOSTRACK_H

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

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    unsigned int numTracks() const;
    unsigned int numTracksDubious() const;

    float trackDurationAll() const;
    float trackDurationNondub() const;
    float trackDurationDubious() const;

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    EvaluationRequirement::DubiousTrack* req ();

    void addAnnotations(nlohmann::json::object_t& viewable, bool overview, bool add_ok) override;

protected:
    void update();

    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::Section& section, const std::string& table_name);
    void addTargetDetailsToTableADSB (EvaluationResultsReport::Section& section, const std::string& table_name);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    boost::optional<float> p_dubious_track_;

    unsigned int num_tracks_             {0};
    unsigned int num_tracks_dubious_     {0};

    float track_duration_all_     {0};
    float track_duration_nondub_  {0};
    float track_duration_dubious_ {0};
};

}

#endif // EVALUATIONREQUIREMENSINGLEDUBIOSTRACK_H
