#include "latexcontent.h"
#include "latexsection.h"
#include "latextable.h"
#include "lateximage.h"

#include <sstream>
#include <cassert>

using namespace std;

LatexContent::LatexContent()
{}

std::string LatexContent::toString()
{
    stringstream ss;

    for (auto& cont_it : content_)
        ss << cont_it << R"(\ \\)" << "\n";

    for (auto& cont_it : sub_content_)
        ss << cont_it->toString() << "\n";

    return ss.str();
}

LatexSection* LatexContent::findSubSection (const std::string& heading)
{
    LatexSection* tmp;

    for (auto& cont_it : sub_content_)
    {
        tmp = dynamic_cast<LatexSection*>(cont_it.get());

        if (tmp && tmp->heading() == heading)
            return tmp;
    }

    return nullptr;
}

LatexTable* LatexContent::findTable (const std::string& name)
{
    LatexTable* tmp;

    for (auto& cont_it : sub_content_)
    {
        tmp = dynamic_cast<LatexTable*>(cont_it.get());

        if (tmp && tmp->name() == name)
            return tmp;
    }

    return nullptr;
}

LatexImage* LatexContent::findImage (const std::string& filename)
{
    LatexImage* tmp;

    for (auto& cont_it : sub_content_)
    {
        tmp = dynamic_cast<LatexImage*>(cont_it.get());

        if (tmp && tmp->filename() == filename)
            return tmp;
    }

    return nullptr;
}
