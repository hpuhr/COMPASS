#ifndef SECTIONCONTENT_H
#define SECTIONCONTENT_H

#include <string>

class QWidget;
class QVBoxLayout;

namespace EvaluationResultsReport
{
    using namespace std;

    class Section;

    class SectionContent
    {
    public:
        SectionContent(const string& name, Section* parent_section);

        string name() const;

        virtual void addToLayout (QVBoxLayout* layout) = 0; // add content to layout, do not store/delete after

    protected:
        string name_;
        Section* parent_section_ {nullptr};

    };

}

#endif // SECTIONCONTENT_H
