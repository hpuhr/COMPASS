#ifndef SECTIONCONTENTTABLE_H
#define SECTIONCONTENTTABLE_H

#include "eval/results/report/sectioncontent.h"
#include "viewabledataconfig.h"

#include <QVariant>
#include <QAbstractItemModel>

#include "json.hpp"

#include <vector>

class ViewableDataConfig;

class QSortFilterProxyModel;

namespace EvaluationResultsReport
{
    using namespace std;

    class SectionContentTable : public QAbstractItemModel, public SectionContent
    {
        Q_OBJECT

    public slots:
        void currentRowChangedSlot(const QModelIndex& current, const QModelIndex& previous);
        void doubleClickedSlot(const QModelIndex& index);

    public:
        SectionContentTable(const string& name, unsigned int num_columns,
                            vector<string> headings, Section* parent_section, EvaluationManager& eval_man);

        void addRow (vector<QVariant> row, unique_ptr<nlohmann::json::object_t> viewable_data,
                     const string& reference);

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
        vector<unique_ptr<nlohmann::json::object_t>> viewable_data_;
        vector<string> references_;

        QSortFilterProxyModel* proxy_model_ {nullptr};
    };

}
#endif // SECTIONCONTENTTABLE_H
