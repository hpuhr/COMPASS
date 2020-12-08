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

#ifndef EVALUATIONREQUIREMENTEXTRATRACKRESULT_H
#define EVALUATIONREQUIREMENTEXTRATRACKRESULT_H

#include "eval/results/single.h"
#include "eval/requirement/extra/track.h"

namespace EvaluationRequirementResult
{

class SingleTrack : public Single
{
public:
    SingleTrack(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer, unsigned int utn, const EvaluationTargetData* target,
            EvaluationManager& eval_man,
            bool ignore, unsigned int num_inside, unsigned int num_extra,  unsigned int num_ok,
            std::vector<EvaluationRequirement::ExtraTrackDetail> details);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    bool ignore() const;

    unsigned int numInside() const;
    unsigned int numExtra() const;
    unsigned int numOK() const;

    const std::vector<EvaluationRequirement::ExtraTrackDetail>& details() const;

protected:
    bool ignore_ {false};
    unsigned int num_inside_ {0};
    unsigned int num_extra_ {0};
    unsigned int num_ok_ {0};
    std::vector<EvaluationRequirement::ExtraTrackDetail> details_;

    bool has_prob_ {false};
    float prob_{0};

    void updateProb();
    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::Section& section, const std::string& table_name);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable ();
};

}

#endif // EVALUATIONREQUIREMENTEXTRATRACKRESULT_H
