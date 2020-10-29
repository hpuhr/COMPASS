#include "latextext.h"

LatexText::LatexText(const std::string& latex_text)
    : latex_text_(latex_text)
{

}

std::string LatexText::toString()
{
    return "\n"+latex_text_;
}
