#ifndef LATEXSECTION_H
#define LATEXSECTION_H

#include "latexcontent.h"

enum class LatexSectionLevel
{
    CHAPTER,
    SECTION,
    SUBSECTION,
    SUBSUBSECTION,
    PARAGRAPH
};

class LatexSection : public LatexContent
{
public:
    LatexSection(LatexSectionLevel level, const std::string& heading);

protected:
    LatexSectionLevel level_;
    std::string heading_;
};

#endif // LATEXSECTION_H
