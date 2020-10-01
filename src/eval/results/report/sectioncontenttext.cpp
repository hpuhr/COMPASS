#include "eval/results/report/sectioncontenttext.h"
#include "evaluationmanager.h"

#include <QLabel>
#include <QVBoxLayout>

#include <cassert>

namespace EvaluationResultsReport
{
SectionContentText::SectionContentText(const string& name, Section* parent_section, EvaluationManager& eval_man)
    : SectionContent(name, parent_section, eval_man)
{

}

void SectionContentText::addText (const string& text)
{
    texts_.push_back(text);
}

void SectionContentText::addToLayout (QVBoxLayout* layout)
{
    assert (layout);

    for (auto& text : texts_)
    {
        QLabel* label = new QLabel((text+"\n\n").c_str());
        label->setWordWrap(true);

        layout->addWidget(label);
    }
}

}
