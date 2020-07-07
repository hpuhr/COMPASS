#ifndef LATEXCONTENT_H
#define LATEXCONTENT_H

#include <string>
#include <memory>
#include <vector>

class LatexSection;
class LatexTable;
class LatexImage;

class LatexContent
{
public:
    LatexContent();

    virtual std::string toString();

protected:
    std::vector<std::string> content_; // main content as latex strings

    std::vector<std::unique_ptr<LatexContent>> sub_content_;

    LatexSection* findSubSection (const std::string& heading); // nullptr if not found
    LatexTable* findTable (const std::string& name); // nullptr if not found
    LatexImage* findImage (const std::string& filename); // nullptr if not found
};

#endif // LATEXCONTENT_H
