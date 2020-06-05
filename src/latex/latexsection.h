#ifndef LATEXSECTION_H
#define LATEXSECTION_H

#include "latexcontent.h"

enum class LatexSectionLevel
{
    SECTION=0,
    SUBSECTION,
    SUBSUBSECTION,
    PARAGRAPH,
    SUBPARAGRAPH,
};

class LatexSection : public LatexContent
{
public:
    LatexSection(LatexSectionLevel level, const std::string& heading);

    LatexSectionLevel level() const;

    std::string heading() const;

    bool hasSubSection (const std::string& heading);
    LatexSection& getSubSection (const std::string& heading);
    void addSubSection (const std::string& heading);

    void addText (const std::string& latex_str);

    virtual std::string toString() override;

protected:
    LatexSectionLevel level_;
    std::string heading_;
};

#endif // LATEXSECTION_H
