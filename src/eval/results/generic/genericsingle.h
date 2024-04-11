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

#include "eval/results/single.h"
#include "eval/results/joined.h"
#include "eval/results/base.h"

#include <memory>
#include <string>

#include <boost/optional.hpp>

namespace EvaluationRequirement
{
class Generic;
}

namespace EvaluationRequirementResult
{

class SingleGeneric : public Single
{
  public:
    SingleGeneric(const std::string& result_type, const std::string& result_id,
                  std::shared_ptr<EvaluationRequirement::Base> requirement,
                  const SectorLayer& sector_layer,
                  unsigned int utn,
                  const EvaluationTargetData* target,
                  EvaluationManager& eval_man,
                  const EvaluationDetails& details,
                  unsigned int num_updates,
                  unsigned int num_no_ref_pos,
                  unsigned int num_no_ref,
                  unsigned int num_pos_outside,
                  unsigned int num_pos_inside,
                  unsigned int num_unknown,
                  unsigned int num_correct,
                  unsigned int num_false);

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

    void addAnnotations(nlohmann::json::object_t& viewable, bool overview, bool add_ok) override;

    virtual std::map<std::string, std::vector<LayerDefinition>> gridLayers() const override;
    virtual std::vector<Eigen::Vector3d> getGridValues(const std::string& layer) const override;

    unsigned int numUpdates() const;
    unsigned int numNoRefPos() const;
    unsigned int numNoRefValue() const;
    unsigned int numPosOutside() const;
    unsigned int numPosInside() const;
    unsigned int numUnknown() const;
    unsigned int numCorrect() const;
    unsigned int numFalse() const;

    enum DetailKey
    {
        RefExists,    //bool
        PosInside,    //bool
        IsNotOk,      //bool
        NumUpdates,   //unsigned int
        NumNoRef,     //unsigned int
        NumInside,    //unsigned ints
        NumOutside,   //unsigned int
        NumUnknownID, //unsigned int
        NumCorrectID, //unsigned int
        NumFalseID   //unsigned int
    };

  protected:
    unsigned int num_updates_     {0};
    unsigned int num_no_ref_pos_  {0};
    unsigned int num_no_ref_val_  {0};
    unsigned int num_pos_outside_ {0};
    unsigned int num_pos_inside_  {0};
    unsigned int num_unknown_     {0};
    unsigned int num_correct_     {0};
    unsigned int num_false_       {0};

    boost::optional<float> p_false_;

    void updateProbabilities();
    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::Section& section, const std::string& table_name);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable (bool add_highlight=false);

    EvaluationRequirement::Generic& genericRequirement() const;
};

}


