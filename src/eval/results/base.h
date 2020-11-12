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

#ifndef EVALUATIONREQUIREMENTRESULTBASE_H
#define EVALUATIONREQUIREMENTRESULTBASE_H

#include <QVariant>

#include "json.hpp"

#include <memory>
#include <vector>

class EvaluationTargetData;
class EvaluationManager;
class SectorLayer;

namespace EvaluationRequirement {
    class Base;
}

namespace EvaluationResultsReport {
    class Section;
    class RootItem;
    class SectionContentTable;
}

namespace EvaluationRequirementResult
{

class Base
{
public:
    Base(const std::string& type, const std::string& result_id,
         std::shared_ptr<EvaluationRequirement::Base> requirement, const SectorLayer& sector_layer,
         EvaluationManager& eval_man);

    std::shared_ptr<EvaluationRequirement::Base> requirement() const;

    virtual void print() = 0;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) = 0;

    std::string type() const;
    std::string resultId() const;
    std::string reqGrpId() const;

    virtual bool isSingle() const = 0;
    virtual bool isJoined() const = 0;

    bool use() const;
    void use(bool use);

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation);
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation);

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation);
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation);

    const static std::string req_overview_table_name_;

protected:
    std::string type_;
    std::string result_id_;
    std::string req_grp_id_;

    bool use_ {true};

    std::shared_ptr<EvaluationRequirement::Base> requirement_;
    const SectorLayer& sector_layer_;

    EvaluationManager& eval_man_;

    EvaluationResultsReport::SectionContentTable& getReqOverviewTable (
            std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

    virtual std::string getRequirementSectionID ();
    virtual std::string getRequirementSumSectionID ();

    EvaluationResultsReport::Section& getRequirementSection (
            std::shared_ptr<EvaluationResultsReport::RootItem> root_item);


};

}

#endif // EVALUATIONREQUIREMENTRESULTBASE_H
