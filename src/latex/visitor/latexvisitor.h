#ifndef LATEXVISITOR_H
#define LATEXVISITOR_H

#include "global.h"

#include <string>

class ViewPoint;
class ListBoxView;
class OSGView;
class LatexDocument;

class LatexVisitor
{
public:
    LatexVisitor(LatexDocument& report, bool group_by_type, bool add_overview_table, bool add_overview_screenshot);

    virtual void visit(const ViewPoint* e);
    virtual void visit(ListBoxView* e);
#if USE_EXPERIMENTAL_SOURCE == true
    virtual void visit(OSGView* e);
#endif

    void imagePrefix(const std::string& image_prefix);

protected:
    LatexDocument& report_;

    bool group_by_type_ {true};
    bool add_overview_table_ {true};
    bool add_overview_screenshot_ {true};

    bool screenshot_folder_created_ {false};

    std::string current_section_name_;
    std::string image_prefix_;
};

#endif // LATEXVISITOR_H
