#include "eval/results/report/sectioncontenttable.h"
#include "evaluationmanager.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QSortFilterProxyModel>

#include <cassert>

using namespace std;

namespace EvaluationResultsReport
{

SectionContentTable::SectionContentTable(const string& name, unsigned int num_columns,
                                         vector<string> headings, Section* parent_section,
                                         EvaluationManager& eval_man)
    : SectionContent(name, parent_section, eval_man), num_columns_(num_columns), headings_(headings)
{

}

void SectionContentTable::addRow (vector<QVariant> row, std::unique_ptr<nlohmann::json::object_t> viewable_data)
{
    assert (row.size() == num_columns_);
    assert (viewable_data_.size() == rows_.size());

    if (viewable_data)
        viewable_data_.push_back(move(viewable_data));
    else
        viewable_data_.push_back(nullptr);

    rows_.push_back(row);
}

void SectionContentTable::addToLayout (QVBoxLayout* layout)
{
    assert (layout);

    proxy_model_ = new QSortFilterProxyModel();
    proxy_model_->setSourceModel(this);

    QTableView* table_view = new QTableView();
    table_view->setModel(proxy_model_);
    table_view->setSortingEnabled(true);
    table_view->sortByColumn(0, Qt::AscendingOrder);
    table_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    table_view->setWordWrap(true);
    table_view->reset();

    connect(table_view->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &SectionContentTable::currentRowChanged);

    table_view->resizeColumnsToContents();
    table_view->resizeRowsToContents();

    layout->addWidget(table_view);

//    for (auto& text : texts_)
//    {
//        QLabel* label = new QLabel((text+"\n\n").c_str());
//        label->setWordWrap(true);

//        layout->addWidget(label);
//    }
}

int SectionContentTable::rowCount(const QModelIndex& parent) const
{
    return rows_.size();
}

int SectionContentTable::columnCount(const QModelIndex& parent) const
{
    return num_columns_;
}

QVariant SectionContentTable::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
        case Qt::DisplayRole:
            {
                logdbg << "SectionContentTable: data: display role: row " << index.row() << " col " << index.column();

                assert (index.row() >= 0);
                assert (index.row() < rows_.size());
                assert (index.column() < num_columns_);

                return rows_.at(index.row()).at(index.column());
            }
        default:
            {
                return QVariant();
            }
    }
}

QVariant SectionContentTable::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        assert (section < num_columns_);
        return headings_.at(section).c_str();
    }

    return QVariant();
}

QModelIndex SectionContentTable::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

QModelIndex SectionContentTable::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

Qt::ItemFlags SectionContentTable::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    assert (index.column() < headings_.size());

    return QAbstractItemModel::flags(index);
}

void SectionContentTable::currentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (!current.isValid())
    {
        loginf << "SectionContentTable: currentRowChanged: invalid index";
        return;
    }

    auto const source_index = proxy_model_->mapToSource(current);
    assert (source_index.isValid());

    assert (source_index.row() >= 0);
    assert (source_index.row() < rows_.size());

    if (viewable_data_.at(source_index.row()))
    {
        loginf << "SectionContentTable: currentRowChanged: index has associated viewable";

        eval_man_.setViewableDataConfig(*viewable_data_.at(source_index.row()).get());
    }
}


}
