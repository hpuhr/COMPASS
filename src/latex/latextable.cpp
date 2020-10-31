/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "latextable.h"
#include "stringconv.h"

#include <cassert>
#include <sstream>

using namespace std;
using namespace Utils;

LatexTable::LatexTable(const std::string& name, unsigned int num_columns,
                       std::vector<std::string> headings, std::string heading_alignment,
                       bool convert_to_latex)
 : name_(name), num_columns_(num_columns), headings_(headings), heading_alignment_(heading_alignment),
   convert_to_latex_(convert_to_latex)
{
    assert (headings_.size() == num_columns_);

    if (!heading_alignment_.size())
    {
        stringstream ss;

        ss << "|";
        for (unsigned int cnt=0; cnt < num_columns_; ++cnt)
                ss << " l |";

        heading_alignment_ = ss.str();
    }

    loginf << "LatexTable: constructor: name " << name << " num cols " << num_columns_
           << " heading alignment '" << heading_alignment_ << "'";

    assert (String::split(heading_alignment_, '|').size() == num_columns_+1);
}

void LatexTable::addRow (std::vector<std::string> row)
{
    assert (row.size() == num_columns_);
    rows_.push_back(row);
}

std::string LatexTable::toString()
{
    //assert (!content_.size());

    stringstream ss;

    if (wide_table_)
    {
        ss << R"(\newgeometry{margin=1cm})" << "\n";
        ss << R"(\begin{landscape})" << "\n";
        ss << R"(\thispagestyle{empty})" << "\n";
    }

    if (wide_table_)
        ss << R"(\begin{tabularx}{\linewidth}[H]{)" << heading_alignment_ << " }\n";
    else
        ss << R"(\begin{tabularx}{\textwidth}[H]{)" << heading_alignment_ << " }\n";

    ss << R"(\hline)" << "\n";
    ss << getLine(headings_, true);

    for (auto& row : rows_)
        ss << getLine(row);

    ss << R"(\end{tabularx})" << "\n";

    ss << R"(\ \\)" << "\n";

    if (wide_table_)
    {
        ss << R"(\end{landscape})" << "\n";
        ss << R"(\restoregeometry)" << "\n";
    }

    return ss.str();
}

std::string LatexTable::name() const
{
    return name_;
}

void LatexTable::setWideTable(bool wide_table)
{
    wide_table_ = wide_table;
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

        if (convert_to_latex_)
            ss << String::latexString(row.at(cnt));
        else
            ss << row.at(cnt);

        if (bold)
            ss << "}";
    }

    ss << R"( \\ \hline)" << "\n";

    return ss.str();
}
