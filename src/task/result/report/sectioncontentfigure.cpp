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
#include "task/result/report/report.h"
#include "task/result/report/sectionid.h"

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

const std::string SectionContentFigure::FieldFigureType      = "figure_type";
const std::string SectionContentFigure::FieldCaption         = "caption";
const std::string SectionContentFigure::FieldRenderDelayMSec = "render_delay_msec";
const std::string SectionContentFigure::FieldViewable        = "viewable";

/**
 * Ctor passing an existing viewable.
 */
SectionContentFigure::SectionContentFigure(unsigned int id,
                                           FigureType figure_type,
                                           const std::string& name, 
                                           const SectionContentViewable& viewable,
                                           Section* parent_section)
:   SectionContent(Type::Figure, id, name, parent_section)
,   fig_type_     (figure_type)
{
    setViewable(viewable);
}

/**
 */
SectionContentFigure::SectionContentFigure(Section* parent_section)
:   SectionContent(Type::Figure, parent_section)
{
}

/**
 */
void SectionContentFigure::setViewable(const SectionContentViewable& viewable)
{
    caption_           = viewable.caption;
    render_delay_msec_ = viewable.render_delay_msec;
    viewable_fnc_      = viewable.viewable_func;
}

/**
 */
void SectionContentFigure::setViewableFunc(const SectionContentViewable::ViewableFunc& func)
{
    viewable_fnc_ = func;
}

/**
 */
void SectionContentFigure::addToLayout(QVBoxLayout* layout)
{
    assert (layout);

    QHBoxLayout* fig_layout = new QHBoxLayout();

    fig_layout->addWidget(new QLabel(("Figure: " + name_).c_str()));
    fig_layout->addStretch();

    QPushButton* view_button = new QPushButton("View");
    QObject::connect (view_button, &QPushButton::clicked, [ this ] () { this->view(); });
    fig_layout->addWidget(view_button);

    layout->addLayout(fig_layout);
}

/**
 */
void SectionContentFigure::accept(LatexVisitor& v)
{
    loginf << "SectionContentFigure: accept";

    //do not add content figures to latex
    if (fig_type_ == FigureType::Content)
        return;

    //@TODO
    //v.visit(this);
}

/**
 */
void SectionContentFigure::view() const
{
    loginf << "SectionContentFigure: view: viewing figure '" << name();

    auto content = viewableContent();
    if (!content || content->empty())
    {
        loginf << "SectionContentFigure: view: no content";
        report_->unsetCurrentViewable();
        return;
    }

    //view content
    report_->setCurrentViewable(*content);

    //execute render delay?
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

/**
 */
std::string SectionContentFigure::getSubPath() const
{
    assert (parent_section_);

    return ResultReport::SectionID::sectionID2Path(parent_section_->compoundResultsHeading());
}

/**
 */
std::shared_ptr<nlohmann::json::object_t> SectionContentFigure::viewableContent() const
{
    //load on-demand content if needed (@TODO: eeewwwwww)
    auto this_unconst = const_cast<SectionContentFigure*>(this);
    this_unconst->loadOnDemandIfNeeded();

    //still no viewable? => return empty data
    if (!viewable_fnc_)
        return std::shared_ptr<nlohmann::json::object_t>();

    //call viewable function
    return viewable_fnc_();
}

/**
 */
void SectionContentFigure::clearContent_impl()
{
    viewable_fnc_ = {};
}

/**
 */
void SectionContentFigure::toJSON_impl(nlohmann::json& root_node) const
{
    root_node[ FieldFigureType      ] = fig_type_;
    root_node[ FieldCaption         ] = caption_;
    root_node[ FieldRenderDelayMSec ] = render_delay_msec_;

    nlohmann::json jviewable = nlohmann::json::object();
    if (!isOnDemand())
    {
        auto viewable = viewableContent();
        if (viewable)
            jviewable = *viewable;
    }

    root_node[ FieldViewable ] = jviewable;
}

/**
 */
bool SectionContentFigure::fromJSON_impl(const nlohmann::json& j)
{
    if (!j.is_object()                    ||
        !j.contains(FieldFigureType)      ||
        !j.contains(FieldCaption)         ||
        !j.contains(FieldRenderDelayMSec) ||
        !j.contains(FieldViewable))
    {
        logerr << "SectionContentFigure: fromJSON: Error: Section content figure does not obtain needed fields";
        return false;
    }

    fig_type_          = j[ FieldFigureType      ];
    caption_           = j[ FieldCaption         ];
    render_delay_msec_ = j[ FieldRenderDelayMSec ];

    std::shared_ptr<nlohmann::json::object_t> jviewable(new nlohmann::json::object_t);
    *jviewable = j[ FieldViewable ];

    viewable_fnc_ = [ = ] () { return jviewable; };

    return true;
}

}
