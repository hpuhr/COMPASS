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


#include "eval/results/single.h"
#include "eval/requirement/dubious/dubioustrack.h"

namespace EvaluationRequirementResult
{

class SingleDubiousTrack : public Single
{
public:
    SingleDubiousTrack(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            unsigned int num_updates,
            unsigned int num_pos_outside, unsigned int num_pos_inside, unsigned int num_tracks,
            unsigned int num_tracks_dubious, std::string dubious_reason,
            EvaluationRequirement::DubiousTrackDetail detail);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    unsigned int numUpdates() const;
    unsigned int numPosOutside() const;
    unsigned int numPosInside() const;
    unsigned int numTracks() const;
    unsigned int numTracksDubious() const;

    EvaluationRequirement::DubiousTrackDetail detail() const;

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    EvaluationRequirement::DubiousTrack* req ();


protected:
    unsigned int num_updates_ {0};
    unsigned int num_pos_outside_ {0};
    unsigned int num_pos_inside_ {0};
    unsigned int num_tracks_ {0};
    unsigned int num_tracks_dubious_ {0};
    std::string dubious_reason_;

    EvaluationRequirement::DubiousTrackDetail detail_;

    bool has_p_dubious_ {false};
    float p_dubious_{0};

    void update();

    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::Section& section, const std::string& table_name);
    void addTargetDetailsToTableADSB (EvaluationResultsReport::Section& section, const std::string& table_name);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable ();
};

}

#endif // EVALUATIONREQUIREMENSINGLEDUBIOSTRACK_H
