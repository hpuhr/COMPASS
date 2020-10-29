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
                 bool wait_on_map_loading);

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
    bool wait_on_map_loading_ {true};

    bool screenshot_folder_created_ {false};

    std::string current_section_name_;
    std::string image_prefix_;
};

#endif // LATEXVISITOR_H
