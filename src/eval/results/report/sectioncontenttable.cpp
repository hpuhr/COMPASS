#include "eval/results/report/sectioncontenttable.h"
#include "evaluationmanager.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QMenu>

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

    void SectionContentTable::addRow (vector<QVariant> row, std::unique_ptr<nlohmann::json::object_t> viewable_data,
                                      const string& reference, bool use, int utn)
    {
        assert (row.size() == num_columns_);
        assert (viewable_data_.size() == rows_.size());

        rows_.push_back(row);

        if (viewable_data)
            viewable_data_.push_back(move(viewable_data));
        else
            viewable_data_.push_back(nullptr);

        references_.push_back(reference);
        use_.push_back(use);
        utns_.push_back(utn);
    }

    void SectionContentTable::addToLayout (QVBoxLayout* layout)
    {
        assert (layout);

        proxy_model_ = new QSortFilterProxyModel();
        proxy_model_->setSourceModel(this);

        table_view_ = new QTableView();
        table_view_->setModel(proxy_model_);
        table_view_->setSortingEnabled(true);
        table_view_->sortByColumn(0, Qt::AscendingOrder);
        table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
        table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
        table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
        table_view_->setContextMenuPolicy(Qt::CustomContextMenu);
        table_view_->setWordWrap(true);
        table_view_->reset();

        connect(table_view_, &QTableView::customContextMenuRequested,
                this, &SectionContentTable::customContextMenuSlot);

        connect(table_view_->selectionModel(), &QItemSelectionModel::currentRowChanged,
                this, &SectionContentTable::currentRowChangedSlot);
        connect(table_view_, &QTableView::doubleClicked,
                this, &SectionContentTable::doubleClickedSlot);

        table_view_->resizeColumnsToContents();
        table_view_->resizeRowsToContents();

        layout->addWidget(table_view_);

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
            case Qt::BackgroundRole:
                {
                    assert (index.row() >= 0);
                    assert (index.row() < rows_.size());

                    if (!use_.at(index.row()))
                        return QBrush(Qt::lightGray);
                    else
                        return QVariant();

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

    void SectionContentTable::currentRowChangedSlot(const QModelIndex& current, const QModelIndex& previous)
    {
        if (!current.isValid())
        {
            loginf << "SectionContentTable: currentRowChangedSlot: invalid index";
            return;
        }

        auto const source_index = proxy_model_->mapToSource(current);
        assert (source_index.isValid());

        assert (source_index.row() >= 0);
        assert (source_index.row() < rows_.size());

        if (viewable_data_.at(source_index.row()))
        {
            loginf << "SectionContentTable: currentRowChangedSlot: index has associated viewable";

            eval_man_.setViewableDataConfig(*viewable_data_.at(source_index.row()).get());
        }
    }

    void SectionContentTable::doubleClickedSlot(const QModelIndex& index)
    {
        if (!index.isValid())
        {
            loginf << "SectionContentTable: doubleClickedSlot: invalid index";
            return;
        }

        auto const source_index = proxy_model_->mapToSource(index);
        assert (source_index.isValid());

        assert (source_index.row() >= 0);
        assert (source_index.row() < rows_.size());

        loginf << "SectionContentTable: doubleClickedSlot: row " << source_index.row();

        if (references_.at(source_index.row()) != "")
        {
            loginf << "SectionContentTable: currentRowChangedSlot: index has associated reference ' "
                   << references_.at(source_index.row()) << "'";

            eval_man_.showResultId(references_.at(source_index.row()));
        }
    }

    void SectionContentTable::customContextMenuSlot(const QPoint& p)
    {
        logdbg << "SectionContentTable: customContextMenuSlot";

        assert (table_view_);

        QModelIndex index = table_view_->indexAt(p);
        if (index.isValid())
        {

            logdbg << "SectionContentTable: customContextMenuSlot: row " << index.row();

            assert (index.row() >= 0);
            assert (index.row() < rows_.size());

            if (utns_.at(index.row() != -1))
            {
                QMenu menu;

                QAction* action = new QAction("Remove", this);
                connect (action, &QAction::triggered, this, &SectionContentTable::removeUTNSlot);
                action->setData(utns_.at(index.row()));

                menu.addAction(action);

                menu.exec(table_view_->viewport()->mapToGlobal(p));
            }
        }
    }

    void SectionContentTable::removeUTNSlot ()
    {
        QAction* action = dynamic_cast<QAction*> (QObject::sender());
        assert (action);

        unsigned int utn = action->data().toUInt();

        loginf << "SectionContentTable: removeUTNSlot: utn " << utn;
    }

}
