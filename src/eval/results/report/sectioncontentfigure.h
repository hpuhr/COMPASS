#ifndef EVALUATIONRESULTSREPORTSECTIONCONTENTFIGURE_H
#define EVALUATIONRESULTSREPORTSECTIONCONTENTFIGURE_H

#include "eval/results/report/sectioncontent.h"

#include "json.hpp"

namespace EvaluationResultsReport
{
    using namespace std;

class SectionContentFigure : public SectionContent
{
public:
    SectionContentFigure(const string& name, const string& caption,
                         nlohmann::json::object_t viewable_data,
                         Section* parent_section, EvaluationManager& eval_man); // const string& path

    virtual void addToLayout (QVBoxLayout* layout) override;

    virtual void accept(LatexVisitor& v) const override;

protected:
    string caption_;
    nlohmann::json::object_t viewable_data_;
};

}
#endif // EVALUATIONRESULTSREPORTSECTIONCONTENTFIGURE_H
