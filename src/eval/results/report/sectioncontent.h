#ifndef EVALUATIONRESULTSREPORTSECTIONCONTENT_H
#define EVALUATIONRESULTSREPORTSECTIONCONTENT_H

#include <string>

class EvaluationManager;

class QWidget;
class QVBoxLayout;
class LatexVisitor;

namespace EvaluationResultsReport
{
    using namespace std;

    class Section;

    class SectionContent
    {
    public:
        SectionContent(const string& name, Section* parent_section, EvaluationManager& eval_man);

        string name() const;

        virtual void addToLayout (QVBoxLayout* layout) = 0; // add content to layout

        virtual void accept(LatexVisitor& v) const = 0;

    protected:
        string name_;
        Section* parent_section_ {nullptr};
        EvaluationManager& eval_man_;

    };

}

#endif // EVALUATIONRESULTSREPORTSECTIONCONTENT_H
