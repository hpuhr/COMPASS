#ifndef SECTIONCONTENTTABLE_H
#define SECTIONCONTENTTABLE_H

#include "eval/results/report/sectioncontent.h"

#include <QVariant>
#include <QAbstractItemModel>

#include <vector>

namespace EvaluationResultsReport
{
    using namespace std;

    class SectionContentTable : public SectionContent, public QAbstractItemModel
    {
    public:
        SectionContentTable(const string& name, unsigned int num_columns,
                            vector<string> headings, Section* parent_section);

        void addRow (vector<QVariant> row);

        virtual void addToLayout (QVBoxLayout* layout) override;

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;

        Qt::ItemFlags flags(const QModelIndex &index) const override;

    protected:
        unsigned int num_columns_ {0};
        vector<string> headings_;

        vector<vector<QVariant>> rows_;
    };

}
#endif // SECTIONCONTENTTABLE_H
