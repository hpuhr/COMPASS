#ifndef EVALUATIONRESULTSREPORTROOTITEM_H
#define EVALUATIONRESULTSREPORTROOTITEM_H

#include "eval/results/report/treeitem.h"

#include <memory>

namespace EvaluationResultsReport
{

    class Section;

    class RootItem : public TreeItem
    {
    public:
        RootItem();

        virtual TreeItem *child(int row) override;
        virtual int childCount() const override;
        virtual int columnCount() const override;
        virtual QVariant data(int column) const override;
        virtual int row() const override;

        std::shared_ptr<Section> rootSection();

        Section& getSection (const std::string& id); // bla:bla2

    protected:
        std::shared_ptr<Section> root_section_;
    };

}

#endif // EVALUATIONRESULTSREPORTROOTITEM_H
