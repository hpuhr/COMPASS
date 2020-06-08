#ifndef LATEXTABLE_H
#define LATEXTABLE_H

#include "latexcontent.h"

#include <vector>
#include <string>

class LatexTable : public LatexContent
{
public:
    LatexTable(const std::string& name, unsigned int num_columns,
               std::vector<std::string> headings, std::string heading_alignment="");

    void addRow (std::vector<std::string> row); // not latefied yet

    virtual std::string toString() override;

    std::string name() const;

    void setWideTable(bool wide_table);

protected:
    std::string name_;
    unsigned int num_columns_;
    std::vector<std::string> headings_;
    std::string heading_alignment_;
    bool wide_table_ {false};

    std::vector<std::vector<std::string>> rows_;

    std::string getLine (const std::vector<std::string>& row, bool bold=false);
};

#endif // LATEXTABLE_H
