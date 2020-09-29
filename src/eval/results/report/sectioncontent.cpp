#include "eval/results/report/sectioncontent.h"
#include "eval/results/report/section.h"

namespace EvaluationResultsReport
{
    SectionContent::SectionContent(const string& name, Section* parent_section)
        : name_(name), parent_section_(parent_section)
    {
        assert (parent_section_);
    }

    string SectionContent::name() const
    {
        return name_;
    }
}
