#ifndef EVALUATIONRESULTSREPORTSECTION_H
#define EVALUATIONRESULTSREPORTSECTION_H

#include "eval/results/report/treeitem.h"

#include <memory>
#include <vector>

namespace EvaluationResultsReport
{
    using namespace std;

    class RootItem;

    class Section : public TreeItem
    {
    public:
        Section(string heading, TreeItem* parent_item);

        virtual TreeItem *child(int row) override;
        virtual int childCount() const override;
        virtual int columnCount() const override;
        virtual QVariant data(int column) const override;
        virtual int row() const override;

        string heading() const;

        bool hasSubSection (const std::string& heading);
        Section& getSubSection (const std::string& heading);
        void addSubSection (const std::string& heading);

    protected:
        string heading_;

        vector<shared_ptr<Section>> sub_sections_;

        Section* findSubSection (const std::string& heading); // nullptr if not found
    };

}


#endif // EVALUATIONRESULTSREPORTSECTION_H
