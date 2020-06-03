#ifndef LATEXCONTENT_H
#define LATEXCONTENT_H

#include <string>
#include <memory>
#include <vector>

class LatexContent
{
public:
    LatexContent();

    virtual std::string toString();

protected:
    std::vector<std::unique_ptr<LatexContent>> content_;
};

#endif // LATEXCONTENT_H
