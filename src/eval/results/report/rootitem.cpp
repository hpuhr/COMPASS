#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"

#include "stringconv.h"

using namespace std;
using namespace Utils;

namespace EvaluationResultsReport
{

    RootItem::RootItem()
        : TreeItem("Report", nullptr)
    {
        root_section_ = make_shared<Section>("Results", this);
    }

   TreeItem* RootItem::child(int row)
    {
        if (row < 0 || row > 0)
            return nullptr;

        return root_section_.get();
    }

    int RootItem::childCount() const
    {
        return 1;
    }

    int RootItem::columnCount() const
    {
        return 1;
    }

    QVariant RootItem::data(int column) const
    {
        assert (column == 0);

        return name_.c_str();
    }

    int RootItem::row() const
    {
        return 0;
    }

    std::shared_ptr<Section> RootItem::rootSection()
    {
        return root_section_;
    }

    Section& RootItem::getSection (const std::string& id)
    {
        loginf << "RootItem: getSection: id '" << id << "'";

        assert (id.size());
        std::vector<std::string> parts = String::split(id, ':');
        assert (parts.size());

        Section* tmp;

        for (unsigned int cnt=0; cnt < parts.size(); ++cnt)
        {
            std::string& heading = parts.at(cnt);

            if (cnt == 0) // first
            {
                if (!root_section_->hasSubSection(heading))
                    root_section_->addSubSection(heading);

                tmp = &root_section_->getSubSection(heading);
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
}
