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

class LatexTable;
class LatexImage;

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

    bool hasTable (const std::string& name);
    LatexTable& getTable (const std::string& name);
    void addTable (const std::string& name, unsigned int num_columns,
                   std::vector<std::string> headings, std::string heading_alignment="",
                   bool convert_to_latex=true);

    bool hasImage (const std::string& filename);
    LatexImage& getImage (const std::string& filename);
    void addImage (const std::string& filename, const std::string& caption);

    virtual std::string toString() override;

    std::string label() const;
    void label(const std::string& label);

protected:
    LatexSectionLevel level_;
    std::string heading_;

    std::string label_;
};

#endif // LATEXSECTION_H
