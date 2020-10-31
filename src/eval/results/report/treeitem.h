#ifndef EVALUATIONRESULTSREPORTTREEITEM_H
#define EVALUATIONRESULTSREPORTTREEITEM_H

#include <QVariant>

#include <memory>

namespace EvaluationResultsReport
{
    using namespace std;

    class TreeItem
    {
    public:
        TreeItem(const string& name, TreeItem* parent_item);

        virtual TreeItem *child(int row) = 0;
        virtual int childCount() const = 0;
        virtual int columnCount() const = 0;
        virtual QVariant data(int column) const = 0;
        virtual int row() const = 0;
        TreeItem* parentItem();

        string name() const;
        string id() const; // (parent_id):name

    protected:
        string name_;
        string id_;

        TreeItem* parent_item_ {nullptr};
    };
}

#endif // EVALUATIONRESULTSREPORTTREEITEM_H
