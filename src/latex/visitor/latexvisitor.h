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

#ifndef LATEXVISITOR_H
#define LATEXVISITOR_H

#include "global.h"

#include <string>

class ViewPoint;
class ListBoxView;
class OSGView;
class LatexDocument;

namespace EvaluationResultsReport
{
    class Section;
    class SectionContentTable;
    class SectionContentText;
    class SectionContentFigure;
}

class LatexVisitor
{
public:
    LatexVisitor(LatexDocument& report, bool group_by_type, bool add_overview_table, bool add_overview_screenshot,
                 bool include_target_details, bool wait_on_map_loading);

    virtual void visit(const ViewPoint* e);
    virtual void visit(ListBoxView* e);
#if USE_EXPERIMENTAL_SOURCE == true
    virtual void visit(OSGView* e);
#endif

    virtual void visit(const EvaluationResultsReport::Section* e);
    virtual void visit(const EvaluationResultsReport::SectionContentTable* e);
    virtual void visit(const EvaluationResultsReport::SectionContentText* e);
    virtual void visit(const EvaluationResultsReport::SectionContentFigure* e);

    void imagePrefix(const std::string& image_prefix);

protected:
    LatexDocument& report_;

    bool group_by_type_ {true};
    bool add_overview_table_ {true};
    bool add_overview_screenshot_ {true};
    bool include_target_details_ {false};
    bool wait_on_map_loading_ {true};

    bool ignore_listbox_views_ {false};
    bool screenshot_folder_created_ {false};

    std::string current_section_name_;
    std::string image_prefix_;
};

#endif // LATEXVISITOR_H
