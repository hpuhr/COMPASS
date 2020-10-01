#ifndef SECTIONCONTENT_H
#define SECTIONCONTENT_H

#include <string>

class EvaluationManager;

class QWidget;
class QVBoxLayout;

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

    protected:
        string name_;
        Section* parent_section_ {nullptr};
        EvaluationManager& eval_man_;

    };

}

#endif // SECTIONCONTENT_H
