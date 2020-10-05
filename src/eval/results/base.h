#ifndef EVALUATIONREQUIREMENTRESULTBASE_H
#define EVALUATIONREQUIREMENTRESULTBASE_H

#include <memory>
#include <vector>

class EvaluationRequirement;
class EvaluationTargetData;
class EvaluationManager;

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
    Base(std::shared_ptr<EvaluationRequirement> requirement, EvaluationManager& eval_man);

    std::shared_ptr<EvaluationRequirement> requirement() const;

    virtual void print() = 0;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) = 0;

protected:
    std::shared_ptr<EvaluationRequirement> requirement_;

    EvaluationManager& eval_man_;

    EvaluationResultsReport::SectionContentTable& getReqOverviewTable (
            std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

    std::string getRequirementSectionID ();

    EvaluationResultsReport::Section& getRequirementSection (
            std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
};

}

#endif // EVALUATIONREQUIREMENTRESULTBASE_H
