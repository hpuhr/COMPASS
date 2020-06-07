#ifndef LATEXCONTENT_H
#define LATEXCONTENT_H

#include <string>
#include <memory>
#include <vector>

class LatexSection;
class LatexTable;

class LatexContent
{
public:
    LatexContent();

    virtual std::string toString();

protected:
    std::vector<std::string> content_; // main content as latex strings

    std::vector<std::unique_ptr<LatexContent>> sub_content_;

    LatexSection* findSubSection (const std::string& heading); // bla, nullptr if not found
    LatexTable* findTable (const std::string& name); // bla, nullptr if not found
};

#endif // LATEXCONTENT_H
