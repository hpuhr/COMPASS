#include "eval/results/report/sectioncontentfigure.h"
#include "evaluationmanager.h"
#include "latexvisitor.h"
#include "logger.h"

namespace EvaluationResultsReport
{

    SectionContentFigure::SectionContentFigure(const string& name, const string& caption,
                                               nlohmann::json::object_t viewable_data,
                                               Section* parent_section, EvaluationManager& eval_man)
        : SectionContent(name, parent_section, eval_man), caption_(caption), viewable_data_(viewable_data)
    {

    }

    void SectionContentFigure::addToLayout (QVBoxLayout* layout)
    {
        assert (layout);
    }

    void SectionContentFigure::accept(LatexVisitor& v) const
    {
        loginf << "SectionContentFigure: accept";
        v.visit(this);
    }
}
