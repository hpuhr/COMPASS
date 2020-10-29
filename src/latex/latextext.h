#ifndef LATEXTEXT_H
#define LATEXTEXT_H

#include "latexcontent.h"

class LatexText : public LatexContent
{
public:
    LatexText(const std::string& latex_text);

    virtual std::string toString() override;

protected:
    std::string latex_text_;
};

#endif // LATEXTEXT_H
