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

#include "latexdocument.h"
#include "logger.h"
#include "files.h"
#include "stringconv.h"
#include "latexsection.h"

#include <memory>

#include <fstream>
#include <sstream>

#include <QDateTime>

using namespace std;
using namespace Utils;

LatexDocument::LatexDocument(const std::string& path, const std::string& filename)
    : path_(path), filename_(filename)
{
    loginf << "LatexDocument: constructor: path '" << path_ << "' filename '" << filename_ << "'";
}


void LatexDocument::write()
{
    loginf << "LatexDocument: write: path '" << path_ << "' filename '" << filename_ << "'";

    Files::createMissingDirectories(path_);
    ofstream file (path_+filename_);

    file << toString();

    loginf << "LatexDocument: write: done";
}

std::string LatexDocument::toString()
{
    stringstream ss;

    ss << R"(\documentclass[twoside,a4paper]{article}
          \usepackage{geometry}
          \geometry{legalpaper, margin=1.5cm}

          \usepackage{graphicx}
          \usepackage{float}
          \usepackage[export]{adjustbox}

          \usepackage{makeidx}
          \usepackage{ifthen}
          \usepackage{listings}
          \usepackage{hyperref}
          \usepackage{tabularx}
          \usepackage{ltablex}
          \usepackage{pdflscape}

          \usepackage{xcolor}
          \definecolor{lbcolor}{rgb}{0.9,0.9,0.9}
          \definecolor{darkgreen}{rgb}{0.0, 0.5, 0.13}

          \lstset{
            basicstyle=\ttfamily,
            columns=fullflexible,
            %frame=single,
            breaklines=true,
            backgroundcolor=\color{lbcolor},
            %postbreak=\mbox{\textcolor{red}{$\hookrightarrow$}\space},
            postbreak=\mbox{$\hookrightarrow$\space},
          }

          \def\bs{\char'134 } % backslash in \tt font.
          \newcommand{\ie}{i.\,e.,}
          \newcommand{\eg}{e.\,g..}
          \DeclareRobustCommand\cs[1]{\texttt{\char`\\#1}})" << "\n";

    if (title_.size())
        ss << R"(\title{)" << title_ << "}"  << "\n";

    if (author_.size())
        ss << R"(\author{)" << author_ << "}"  << "\n";

    ss << R"(\date{)" << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss").toStdString() << "}\n";

    ss << R"(\emergencystretch1em  %

          \makeindex

          \setcounter{tocdepth}{3}

          \begin{document})" << "\n";

    if (title_.size())
        ss << R"(\maketitle)" << "\n";

    if (abstract_.size())
        ss << R"(\begin{abstract})" << "\n" << abstract_ << "\n" << R"(\end{abstract}
                                                            \ \\)"  << "\n";

    ss << R"(\newpage

          \tableofcontents

          \newpage)" << "\n";

    ss << LatexContent::toString() << "\n";

    ss << R"(\printindex

    \end{document})" << "\n";

    return ss.str();
}

LatexSection& LatexDocument::getSection (const std::string& id)
{
    assert (id.size());
    std::vector<std::string> parts = String::split(id, ':');
    assert (parts.size());

    loginf << "LatexDocument: getSection: id '"+id+"' parts " << parts.size();

    //std::string& top = parts.at(0);

    LatexSection* tmp;

    for (unsigned int cnt=0; cnt < parts.size(); ++cnt)
    {
        std::string& heading = parts.at(cnt);

        if (cnt == 0) // first
        {
            if (!hasSubSection(heading))
                addSubSection(heading);

            tmp = &getSubSection(heading);
        }
        else // previous section
        {
            assert (tmp);

            if (!tmp->hasSubSection(heading))
                tmp->addSubSection(heading);

            tmp = &tmp->getSubSection(heading);
        }
    }

    assert (tmp);
    return *tmp;
}

bool LatexDocument::hasSubSection (const std::string& heading)
{
    return findSubSection(heading) != nullptr;
}

LatexSection& LatexDocument::getSubSection (const std::string& heading)
{
    LatexSection* tmp = findSubSection (heading);
    assert (tmp);
    return *tmp;
}

void LatexDocument::addSubSection (const std::string& heading)
{
    assert (!hasSubSection(heading));
    sub_content_.push_back(unique_ptr<LatexSection>(new LatexSection(LatexSectionLevel::SECTION, heading)));
    assert (hasSubSection(heading));
}

std::string LatexDocument::path() const
{
    return path_;
}

std::string LatexDocument::filename() const
{
    return filename_;
}

std::string LatexDocument::title() const
{
    return title_;
}

void LatexDocument::title(const std::string& title)
{
    title_ = title;
}

std::string LatexDocument::author() const
{
    return author_;
}

void LatexDocument::author(const std::string& author)
{
    author_ = author;
}

std::string LatexDocument::abstract() const
{
    return abstract_;
}

void LatexDocument::abstract(const std::string& abstract)
{
    abstract_ = abstract;
}

