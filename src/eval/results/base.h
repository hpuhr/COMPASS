#ifndef EVALUATIONREQUIREMENTRESULTBASE_H
#define EVALUATIONREQUIREMENTRESULTBASE_H

#include <memory>
#include <vector>

class EvaluationTargetData;
class EvaluationManager;

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
         std::shared_ptr<EvaluationRequirement::Base> requirement, EvaluationManager& eval_man);

    std::shared_ptr<EvaluationRequirement::Base> requirement() const;

    virtual void print() = 0;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) = 0;

    std::string type() const;
    std::string resultId() const;
    std::string reqGrpId() const;

protected:
    std::string type_;
    std::string result_id_;
    std::string req_grp_id_;

    std::shared_ptr<EvaluationRequirement::Base> requirement_;

    EvaluationManager& eval_man_;

    EvaluationResultsReport::SectionContentTable& getReqOverviewTable (
            std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

    std::string getRequirementSectionID ();

    EvaluationResultsReport::Section& getRequirementSection (
            std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
};

}

#endif // EVALUATIONREQUIREMENTRESULTBASE_H
