#include "latextable.h"
#include "stringconv.h"

#include <cassert>
#include <sstream>

using namespace std;
using namespace Utils;

LatexTable::LatexTable(const std::string& name, unsigned int num_columns,
                       std::vector<std::string> headings, std::string heading_alignment)
 : name_(name), num_columns_(num_columns), headings_(headings), heading_alignment_(heading_alignment)
{
    assert (headings_.size() == num_columns_);

    if (!heading_alignment_.size())
    {
        stringstream ss;

        ss << " | ";
        for (unsigned int cnt=0; cnt < num_columns_; ++cnt)
            ss << "l | ";

        heading_alignment_ = ss.str();
    }

    assert (String::split(heading_alignment_, '|').size() == num_columns_+1);
}

void LatexTable::addRow (std::vector<std::string> row)
{
    assert (row.size() == num_columns_);
    rows_.push_back(row);
}

std::string LatexTable::toString()
{
    assert (!content_.size());

    stringstream ss;

    ss << R"(\begin{center})" << "\n";
    ss << R"(\begin{table}[H])" << "\n";
    ss << R"(\begin{tabularx}{\textwidth}{)" << heading_alignment_ << " }\n";
    ss << R"(\hline)" << "\n";
    ss << getLine(headings_, true);

    for (auto& row : rows_)
        ss << getLine(row);

    ss << R"(\end{tabularx})" << "\n";
    ss << R"(\end{table})" << "\n";
    ss << R"(\end{center})" << "\n";
    ss << R"(\ \\)" << "\n";

    return ss.str();
}

std::string LatexTable::name() const
{
    return name_;
}

std::string LatexTable::getLine (const std::vector<std::string>& row, bool bold)
{
    stringstream ss;

    for (unsigned int cnt=0; cnt < row.size(); ++cnt)
    {
        if (cnt != 0)
            ss << " & ";

        if (bold)
            ss << R"(\textbf{)";

        ss << String::latexString(row.at(cnt));

        if (bold)
            ss << "}";
    }

    ss << R"( \\ \hline)" << "\n";

    return ss.str();
}
