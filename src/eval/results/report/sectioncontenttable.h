#ifndef EVALUATIONRESULTSREPORTSECTIONCONTENTTABLE_H
#define EVALUATIONRESULTSREPORTSECTIONCONTENTTABLE_H

#include "eval/results/report/sectioncontent.h"

#include <QVariant>
#include <QAbstractItemModel>

#include "json.hpp"

#include <vector>

class ViewableDataConfig;

class QSortFilterProxyModel;
class QTableView;

namespace EvaluationRequirementResult
{
    class Base;
}

namespace EvaluationResultsReport
{
    using namespace std;

    class SectionContentTable : public QAbstractItemModel, public SectionContent
    {
        Q_OBJECT

    public slots:
        void currentRowChangedSlot(const QModelIndex& current, const QModelIndex& previous);
        void doubleClickedSlot(const QModelIndex& index);
        void customContextMenuSlot(const QPoint& p);
        void addUTNSlot ();
        void removeUTNSlot ();

    public:
        SectionContentTable(const string& name, unsigned int num_columns,
                            vector<string> headings, Section* parent_section, EvaluationManager& eval_man,
                            bool sortable=true, unsigned int sort_column=0, Qt::SortOrder order=Qt::AscendingOrder);

        void addRow (vector<QVariant> row, EvaluationRequirementResult::Base* result_ptr,
                     QVariant annotation = {});

        virtual void addToLayout (QVBoxLayout* layout) override;

        virtual void accept(LatexVisitor& v) const override;

        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;

        Qt::ItemFlags flags(const QModelIndex &index) const override;

        vector<string> headings() const;
        std::vector<std::string> sortedRowStrings(unsigned int row) const;

        bool hasReference (unsigned int row) const;
        std::string reference (unsigned int row) const;

    protected:
        unsigned int num_columns_ {0};
        vector<string> headings_;

        bool sortable_ {true};
        unsigned int sort_column_ {0};
        Qt::SortOrder order_ {Qt::AscendingOrder};

        vector<vector<QVariant>> rows_;
        vector<EvaluationRequirementResult::Base*> result_ptrs_;
        vector<QVariant> annotations_;

        mutable QSortFilterProxyModel* proxy_model_ {nullptr};
        mutable QTableView* table_view_ {nullptr}; // for reset
    };

}
#endif // EVALUATIONRESULTSREPORTSECTIONCONTENTTABLE_H
