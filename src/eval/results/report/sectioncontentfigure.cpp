/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "eval/results/report/sectioncontentfigure.h"
#include "eval/results/report/section.h"
#include "evaluationmanager.h"
#include "latexvisitor.h"
#include "logger.h"
#include "stringconv.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

using namespace Utils;

namespace EvaluationResultsReport
{

    SectionContentFigure::SectionContentFigure(const string& name, const string& caption,
                                               std::unique_ptr<nlohmann::json::object_t> viewable_data,
                                               Section* parent_section, EvaluationManager& eval_man)
        : SectionContent(name, parent_section, eval_man), caption_(caption), viewable_data_(move(viewable_data))
    {
        assert (viewable_data_);
    }

    void SectionContentFigure::addToLayout (QVBoxLayout* layout)
    {
        assert (layout);

        QHBoxLayout* fig_layout = new QHBoxLayout();

        fig_layout->addWidget(new QLabel(("Figure: "+caption_).c_str()));

        fig_layout->addStretch();

        QPushButton* view_button = new QPushButton("View");
        connect (view_button, &QPushButton::clicked, this, &SectionContentFigure::viewSlot);
        fig_layout->addWidget(view_button);

        layout->addLayout(fig_layout);
    }

    void SectionContentFigure::accept(LatexVisitor& v) const
    {
        loginf << "SectionContentFigure: accept";
        v.visit(this);
    }

    void SectionContentFigure::viewSlot()
    {
        loginf << "SectionContentFigure: viewSlot";
        view();
    }

    void SectionContentFigure::view () const
    {
        eval_man_.setViewableDataConfig(*viewable_data_);
    }

    std::string SectionContentFigure::getSubPath() const
    {
        assert (parent_section_);

        string path = parent_section_->compoundResultsHeading();

        boost::replace_all(path, ":", "/");
        boost::replace_all(path, " ", "_");

        return path+"/";
    }
}
