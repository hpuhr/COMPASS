#include "eval/results/report/sectioncontent.h"
#include "eval/results/report/section.h"

namespace EvaluationResultsReport
{
    SectionContent::SectionContent(const string& name, Section* parent_section, EvaluationManager& eval_man)
        : name_(name), parent_section_(parent_section), eval_man_(eval_man)
    {
        assert (parent_section_);
    }

    string SectionContent::name() const
    {
        return name_;
    }
}
