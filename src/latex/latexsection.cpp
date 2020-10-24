#include "latexsection.h"
#include "latextable.h"
#include "lateximage.h"

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

bool LatexSection::hasTable (const std::string& name)
{
    return findTable(name) != nullptr;
}

LatexTable& LatexSection::getTable (const std::string& name)
{
    LatexTable* tmp = findTable (name);
    assert (tmp);
    return *tmp;
}

void LatexSection::addTable (const std::string& name, unsigned int num_columns,
                             std::vector<std::string> headings, std::string heading_alignment,
                             bool convert_to_latex)
{
    assert (!hasTable(name));
    sub_content_.push_back(unique_ptr<LatexTable>(
                           new LatexTable(name, num_columns, headings, heading_alignment, convert_to_latex)));
    assert (hasTable(name));
}

bool LatexSection::hasImage (const std::string& filename)
{
    return findImage(filename) != nullptr;
}

LatexImage& LatexSection::getImage (const std::string& filename)
{
    LatexImage* tmp = findImage (filename);
    assert (tmp);
    return *tmp;
}

void LatexSection::addImage (const std::string& filename, const std::string& caption)
{
    assert (!hasImage(filename));
    sub_content_.push_back(unique_ptr<LatexImage>(
                           new LatexImage(filename, caption)));
    assert (hasImage(filename));
}

std::string LatexSection::toString()
{
    stringstream ss;

    if (level_ == LatexSectionLevel::SECTION)
        ss << R"(\section{)" << heading_ << "}\n";
    else if (level_ == LatexSectionLevel::SUBSECTION)
        ss << R"(\subsection{)" << heading_ << "}\n";
    else if (level_ == LatexSectionLevel::SUBSUBSECTION)
        ss << R"(\subsubsection{)" << heading_ << "}\n";
    else if (level_ == LatexSectionLevel::PARAGRAPH)
        ss << R"(\paragraph{)" << heading_ << "}\n";
    else if (level_ == LatexSectionLevel::SUBPARAGRAPH)
        ss << R"(\subparagraph{)" << heading_ << "}\n";
    else
        throw std::runtime_error ("LatexSection: toString: unkown section level");

    if (label_.size())
        ss << R"(\label{)" << label_ << "}\n";

    ss << "\n";

    ss << LatexContent::toString();

    return ss.str();
}

std::string LatexSection::label() const
{
    return label_;
}

void LatexSection::label(const std::string& label)
{
    label_ = label;
}
