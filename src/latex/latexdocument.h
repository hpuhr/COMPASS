#ifndef LATEXDOCUMENT_H
#define LATEXDOCUMENT_H

#include "latexcontent.h"

class LatexDocument : public LatexContent
{
public:
    LatexDocument(const std::string& path, const std::string& filename); // path has to end with /

    void write();

    std::string title() const;
    void title(const std::string& title);

    std::string author() const;
    void author(const std::string& author);

    std::string abstract() const;
    void abstract(const std::string& abstract);

    virtual std::string toString() override;

protected:
    std::string path_;
    std::string filename_;

    std::string title_;
    std::string author_;
    std::string abstract_;
};

#endif // LATEXDOCUMENT_H
