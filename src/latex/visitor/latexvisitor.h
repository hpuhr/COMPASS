#ifndef LATEXVISITOR_H
#define LATEXVISITOR_H

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
    virtual void visit(OSGView* e);

    void imagePrefix(const std::string& image_prefix);

protected:
    LatexDocument& report_;
    bool screenshot_folder_created_ {false};

    std::string current_section_name_;
    std::string image_prefix_;
};

#endif // LATEXVISITOR_H
