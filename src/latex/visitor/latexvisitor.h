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
    LatexVisitor(LatexDocument& report);

    virtual void visit(ViewPoint* e);
    virtual void visit(ListBoxView* e);
#if USE_EXPERIMENTAL_SOURCE == true
    virtual void visit(OSGView* e);
#endif

    void imagePrefix(const std::string& image_prefix);

protected:
    LatexDocument& report_;
    bool screenshot_folder_created_ {false};

    std::string current_section_name_;
    std::string image_prefix_;
};

#endif // LATEXVISITOR_H
