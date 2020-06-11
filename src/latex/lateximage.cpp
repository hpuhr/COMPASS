#include "lateximage.h"

#include <cassert>
#include <sstream>

using namespace std;

LatexImage::LatexImage(const std::string& filename, const std::string& caption)
    : filename_(filename), caption_(caption)
{
}

std::string LatexImage::toString()
{
    assert (!content_.size());

    stringstream ss;

    ss << R"(\begin{figure}[H])" << "\n";
    //    \hspace*{-1cm}
    ss << R"(\includegraphics[width=18cm]{)" << filename_ << "}\n";
    ss << R"(\caption{)" << caption_ << "}\n";
    ss << R"(\end{figure})" << "\n";

    return ss.str();
}

std::string LatexImage::filename() const
{
    return filename_;
}
