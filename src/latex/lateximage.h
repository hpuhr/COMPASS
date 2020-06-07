#ifndef LATEXIMAGE_H
#define LATEXIMAGE_H

#include "latexcontent.h"

#include <string>

class LatexImage : public LatexContent
{
public:
    LatexImage(const std::string& filename, const std::string& caption); // filename must be unique
    virtual std::string toString() override;

    std::string filename() const;

protected:
    std::string filename_;
    std::string caption_;
};

#endif // LATEXIMAGE_H
