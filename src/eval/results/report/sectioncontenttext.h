#ifndef SECTIONCONTENTTEXT_H
#define SECTIONCONTENTTEXT_H

#include "eval/results/report/sectioncontent.h"

#include <vector>

namespace EvaluationResultsReport
{
    using namespace std;

    class SectionContentText : public SectionContent
    {
    public:
        SectionContentText(const string& name, Section* parent_section, EvaluationManager& eval_man);

        void addText (const string& text);

        virtual void addToLayout (QVBoxLayout* layout) override;

    protected:
        vector<string> texts_;

    };


}
#endif // SECTIONCONTENTTEXT_H
