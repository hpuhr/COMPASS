#include "latexsection.h"

#include <cassert>
#include <sstream>

using namespace std;

LatexSection::LatexSection(LatexSectionLevel level, const std::string& heading)
    : level_(level), heading_(heading)
{
}

LatexSectionLevel LatexSection::level() const
{
    return level_;
}

std::string LatexSection::heading() const
{
    return heading_;
}

bool LatexSection::hasSubSection (const std::string& heading)
{
    return findSubSection(heading) != nullptr;
}

LatexSection& LatexSection::getSubSection (const std::string& heading)
{
    LatexSection* tmp = findSubSection (heading);
    assert (tmp);
    return *tmp;
}

void LatexSection::addSubSection (const std::string& heading)
{
    assert (!hasSubSection(heading));
    assert (level_ <= LatexSectionLevel::PARAGRAPH);
    unsigned int next_level = static_cast<unsigned int>(level_) + 1;
    sub_content_.push_back(unique_ptr<LatexSection>(
                           new LatexSection(static_cast<LatexSectionLevel>(next_level), heading)));
    assert (hasSubSection(heading));
}

void LatexSection::addText (const std::string& latex_str)
{
    content_.push_back(latex_str);
}

std::string LatexSection::toString()
{
    stringstream ss;

    if (level_ == LatexSectionLevel::SECTION)
        ss << R"(\section{)" << heading_ << "}\n\n";
    else if (level_ == LatexSectionLevel::SUBSECTION)
        ss << R"(\subsection{)" << heading_ << "}\n\n";
    else if (level_ == LatexSectionLevel::SUBSUBSECTION)
        ss << R"(\subsubsection{)" << heading_ << "}\n\n";
    else if (level_ == LatexSectionLevel::PARAGRAPH)
        ss << R"(\parapgraph{)" << heading_ << "}\n\n";
    else if (level_ == LatexSectionLevel::SUBPARAGRAPH)
        ss << R"(\subparapgraph{)" << heading_ << "}\n\n";
    else
        throw std::runtime_error ("LatexSection: toString: unkown section level");

    ss << LatexContent::toString();

    return ss.str();
}
