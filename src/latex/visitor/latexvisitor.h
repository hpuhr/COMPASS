#ifndef LATEXVISITOR_H
#define LATEXVISITOR_H

class ViewPoint;
class LatexDocument;

class LatexVisitor
{
public:
    LatexVisitor(LatexDocument& report);

    virtual void visit(ViewPoint* e);

protected:
    LatexDocument& report_;
};

#endif // LATEXVISITOR_H
