#include "latexcontent.h"
#include "latexsection.h"

#include <sstream>
#include <cassert>

using namespace std;

LatexContent::LatexContent()
{}

std::string LatexContent::toString()
{
    stringstream ss;

    for (auto& cont_it : content_)
        ss << cont_it << "\n";

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


