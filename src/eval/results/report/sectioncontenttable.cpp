#include "eval/results/report/sectioncontenttable.h"
#include "eval/results/base.h"
#include "eval/results/single.h"
#include "evaluationmanager.h"
#include "latexvisitor.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QMenu>

#include <cassert>
#include <type_traits>

using namespace std;

namespace EvaluationResultsReport
{

    SectionContentTable::SectionContentTable(const string& name, unsigned int num_columns,
                                             vector<string> headings, Section* parent_section,
                                             EvaluationManager& eval_man, bool sortable)
        : SectionContent(name, parent_section, eval_man), num_columns_(num_columns), headings_(headings),
          sortable_(sortable)
    {
    }

    void SectionContentTable::addRow (vector<QVariant> row, EvaluationRequirementResult::Base* result_ptr,
                                      QVariant annotation)
    // std::unique_ptr<nlohmann::json::object_t> viewable_data,
    //const string& reference,
    //bool use, int utn)
    {
        assert (row.size() == num_columns_);
        //assert (viewable_data_.size() == rows_.size());

        rows_.push_back(row);
        result_ptrs_.push_back(result_ptr);
        annotations_.push_back(annotation);

        //        if (viewable_data)
        //            viewable_data_.push_back(move(viewable_data));
        //        else
        //            viewable_data_.push_back(nullptr);

        //references_.push_back(reference);
        //use_.push_back(use);
        //utns_.push_back(utn);

        assert (annotations_.size() == rows_.size());
        assert (result_ptrs_.size() == rows_.size());
        //assert (viewable_data_.size() == rows_.size());
        //assert (references_.size() == rows_.size());
        //assert (use_.size() == rows_.size());
        //assert (utns_.size() == rows_.size());
    }

    void SectionContentTable::addToLayout (QVBoxLayout* layout)
    {
        assert (layout);

        if (!proxy_model_)
        {
            proxy_model_ = new QSortFilterProxyModel();
            proxy_model_->setSourceModel(this);
        }

        table_view_ = new QTableView();
        table_view_->setModel(proxy_model_);

        if (sortable_)
        {
            table_view_->setSortingEnabled(true);
            table_view_->sortByColumn(0, Qt::AscendingOrder);
        }

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

    void SectionContentTable::accept(LatexVisitor& v) const
    {
        loginf << "SectionContentTable: accept";
        v.visit(this);
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

                    unsigned int row_index = index.row();

                    if (result_ptrs_.at(row_index) && !result_ptrs_.at(row_index)->use())
                        return QBrush(Qt::lightGray);
                    else
                        return QVariant();

                }
            case Qt::ForegroundRole:
                {
                    assert (index.row() >= 0);
                    assert (index.row() < rows_.size());
                    assert (index.column() < num_columns_);

                    QVariant data = rows_.at(index.row()).at(index.column());

                    if (data.userType() == QMetaType::QString)
                    {
                        if (data == "Passed")
                            return QVariant(QColor(Qt::darkGreen));
                        else if (data == "Failed")
                            return QVariant(QColor(Qt::red));
                    }
                    if (data.userType() == QMetaType::Bool)
                    {
                        if (data == true)
                            return QVariant(QColor(Qt::darkGreen));
                        else if (data == false)
                            return QVariant(QColor(Qt::red));
                    }
                    return QVariant(QColor(Qt::black));
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

    vector<string> SectionContentTable::headings() const
    {
        return headings_;
    }

    std::vector<std::string> SectionContentTable::sortedRowStrings(unsigned int row) const
    {
        if (!proxy_model_)
        {
            proxy_model_ = new QSortFilterProxyModel();
            SectionContentTable* tmp = const_cast<SectionContentTable*>(this); // hacky
            assert (tmp);
            proxy_model_->setSourceModel(tmp);
        }

        logdbg << "SectionContentTable: sortedRowStrings: row " << row << " rows " << proxy_model_->rowCount()
               << " data rows " << rows_.size();
        assert (row < proxy_model_->rowCount());
        assert (row < rows_.size());

        vector<string> result;

        for (unsigned int col=0; col < num_columns_; ++col)
        {
            QModelIndex index = proxy_model_->index(row, col);
            assert (index.isValid());
            result.push_back(proxy_model_->data(index).toString().toStdString());
        }

        return result;
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

        unsigned int row_index = source_index.row();

        if (result_ptrs_.at(row_index)
                && result_ptrs_.at(row_index)->hasViewableData(*this, annotations_.at(row_index)))
        {
            loginf << "SectionContentTable: currentRowChangedSlot: index has associated viewable";

            std::unique_ptr<nlohmann::json::object_t> viewable =
                    result_ptrs_.at(row_index)->viewableData(*this, annotations_.at(row_index));
            assert (viewable);

            eval_man_.setViewableDataConfig(*viewable.get());
        }

        //        if (viewable_data_.at(source_index.row()))
        //        {
        //            loginf << "SectionContentTable: currentRowChangedSlot: index has associated viewable";

        //            eval_man_.setViewableDataConfig(*viewable_data_.at(source_index.row()).get());
        //        }
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

        unsigned int row_index = source_index.row();

        if (result_ptrs_.at(row_index) && result_ptrs_.at(row_index)->hasReference(*this, annotations_.at(row_index)))
        {
            string reference = result_ptrs_.at(row_index)->reference(*this, annotations_.at(row_index));
            assert (reference.size());

            loginf << "SectionContentTable: currentRowChangedSlot: index has associated reference '"
                   << reference << "'";

            eval_man_.showResultId(reference);
        }
    }

    void SectionContentTable::customContextMenuSlot(const QPoint& p)
    {
        logdbg << "SectionContentTable: customContextMenuSlot";

        assert (table_view_);

        QModelIndex index = table_view_->indexAt(p);
        assert (index.isValid());

        auto const source_index = proxy_model_->mapToSource(index);
        assert (source_index.isValid());

        loginf << "SectionContentTable: customContextMenuSlot: row " << index.row() << " src " << source_index.row();

        assert (source_index.row() >= 0);
        assert (source_index.row() < rows_.size());

        unsigned int row_index = source_index.row();

        if (result_ptrs_.at(row_index) && result_ptrs_.at(row_index)->isSingle())
        {
            EvaluationRequirementResult::Single* single_result =
                    static_cast<EvaluationRequirementResult::Single*>(result_ptrs_.at(row_index));
            assert (single_result);

            QMenu menu;

            unsigned int utn = single_result->utn();
            loginf << "SectionContentTable: customContextMenuSlot: utn " << utn;

            assert (eval_man_.getData().hasTargetData(utn));

            const EvaluationTargetData& target_data = eval_man_.getData().targetData(utn);

            if (target_data.use())
            {
                QAction* action = new QAction("Remove", this);
                connect (action, &QAction::triggered, this, &SectionContentTable::removeUTNSlot);
                action->setData(utn);

                menu.addAction(action);
            }
            else
            {
                QAction* action = new QAction("Add", this);
                connect (action, &QAction::triggered, this, &SectionContentTable::addUTNSlot);
                action->setData(utn);

                menu.addAction(action);
            }

            menu.exec(table_view_->viewport()->mapToGlobal(p));
        }
        else
            loginf << "SectionContentTable: customContextMenuSlot: no associated utn";
    }

    void SectionContentTable::addUTNSlot ()
    {
        QAction* action = dynamic_cast<QAction*> (QObject::sender());
        assert (action);

        unsigned int utn = action->data().toUInt();

        loginf << "SectionContentTable: addUTNSlot: utn " << utn;

        eval_man_.setUseTargetData(utn, true);
    }

    void SectionContentTable::removeUTNSlot ()
    {
        QAction* action = dynamic_cast<QAction*> (QObject::sender());
        assert (action);

        unsigned int utn = action->data().toUInt();

        loginf << "SectionContentTable: removeUTNSlot: utn " << utn;

        eval_man_.setUseTargetData(utn, false);
    }

}
