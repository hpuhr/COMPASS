#include "latexcontent.h"

#include <sstream>

using namespace std;

LatexContent::LatexContent()
{
}

std::string LatexContent::toString()
{
    stringstream ss;

    for (auto& cont_it : content_)
        ss << cont_it->toString() << "\n";

    return ss.str();
}
