#include "eval/results/report/section.h"
#include "logger.h"

namespace EvaluationResultsReport
{

    Section::Section(string heading, TreeItem* parent_item)
        : TreeItem(heading, parent_item), heading_(heading)
    {
    }

   TreeItem* Section::child(int row)
    {
        if (row < 0 || row >= sub_sections_.size())
            return nullptr;

        return sub_sections_.at(row).get();
    }

    int Section::childCount() const
    {
        return sub_sections_.size();
    }

    int Section::columnCount() const
    {
        return 1;
    }

    QVariant Section::data(int column) const
    {
        assert (column == 0);

        return heading_.c_str();
    }

    int Section::row() const
    {
        return 0;
    }

    string Section::heading() const
    {
        return heading_;
    }

    bool Section::hasSubSection (const std::string& heading)
    {
        return findSubSection(heading) != nullptr;
    }

    Section& Section::getSubSection (const std::string& heading)
    {
        assert (hasSubSection(heading));

        Section* tmp = findSubSection (heading);
        assert (tmp);
        return *tmp;
    }

    void Section::addSubSection (const std::string& heading)
    {
        loginf << "Section " << heading_ << ": addSubSection: adding " << heading;

        assert (!hasSubSection(heading));
        sub_sections_.push_back(make_shared<Section>(heading, this));
        assert (hasSubSection(heading));
    }

    Section* Section::findSubSection (const std::string& heading)
    {
        for (auto& sec_it : sub_sections_)
        {
            if (sec_it->heading() == heading)
                return sec_it.get();
        }

        return nullptr;
    }

}
