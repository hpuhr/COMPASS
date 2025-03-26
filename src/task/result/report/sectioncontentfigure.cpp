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

#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/section.h"

#include "section_id.h"

#include "taskmanager.h"
//#include "latexvisitor.h"

#include "logger.h"
#include "stringconv.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QThread>

namespace ResultReport
{

/**
 */
SectionContentFigure::SectionContentFigure(const std::string& name, 
                                           const std::string& caption,
                                           std::function<std::shared_ptr<nlohmann::json::object_t>(void)> viewable_fnc,
                                           Section* parent_section,
                                           TaskManager& task_man,
                                           int render_delay_msec)
:   SectionContent    (name, parent_section, task_man)
,   caption_          (caption)
,   render_delay_msec_(render_delay_msec)
,   viewable_fnc_     (viewable_fnc)
{
   //assert (viewable_data_);
}

/**
 */
void SectionContentFigure::addToLayout(QVBoxLayout* layout)
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

/**
 */
void SectionContentFigure::accept(LatexVisitor& v)
{
    loginf << "SectionContentFigure: accept";
    //@TODO
    //v.visit(this);
}

/**
 */
void SectionContentFigure::viewSlot()
{
    loginf << "SectionContentFigure: viewSlot";
    view();
}

void SectionContentFigure::view() const
{
    //@TODO
    //task_man_.setViewableDataConfig(*viewable_fnc_());

    if (render_delay_msec_ > 0)
    {
        boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
        while ((boost::posix_time::microsec_clock::local_time() - start_time).total_milliseconds()
               < render_delay_msec_)
        {
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            QThread::msleep(10);
        }
    }
}

std::string SectionContentFigure::getSubPath() const
{
    assert (parent_section_);

    return EvaluationResultsReport::SectionID::sectionID2Path(parent_section_->compoundResultsHeading());
}

}
