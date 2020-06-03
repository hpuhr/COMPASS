#include "latexdocument.h"
#include "logger.h"
#include "files.h"

#include <fstream>
#include <sstream>

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
          \geometry{legalpaper, margin=2cm}

          \usepackage{graphicx}
          \usepackage{float}
          \usepackage[export]{adjustbox}

          \usepackage{makeidx}
          \usepackage{ifthen}
          \usepackage{listings}
          \usepackage{hyperref}
          \usepackage{tabularx}

          \usepackage{color}
          \definecolor{lbcolor}{rgb}{0.9,0.9,0.9}

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

    ss << R"(\date{}
          \emergencystretch1em  %

          \makeindex

          \setcounter{tocdepth}{2}

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

