/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "eval/results/report/sectioncontenttable.h"
#include "eval/results/base.h"
#include "eval/results/single.h"
#include "evaluationmanager.h"
#include "latexvisitor.h"
#include "logger.h"
#include "stringconv.h"

#include <QVBoxLayout>
#include <QTableView>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QMenu>

#include <cassert>
#include <type_traits>

using namespace std;
using namespace Utils;

namespace EvaluationResultsReport
{

    SectionContentTable::SectionContentTable(const string& name, unsigned int num_columns,
                                             vector<string> headings, Section* parent_section,
                                             EvaluationManager& eval_man, bool sortable,
                                             unsigned int sort_column, Qt::SortOrder order)
        : SectionContent(name, parent_section, eval_man), num_columns_(num_columns), headings_(headings),
          sortable_(sortable), sort_column_(sort_column), order_(order)
    {
    }

    void SectionContentTable::addRow (vector<QVariant> row, EvaluationRequirementResult::Base* result_ptr,
                                      QVariant annotation)
    {
        assert (row.size() == num_columns_);

        rows_.push_back(row);
        result_ptrs_.push_back(result_ptr);
        annotations_.push_back(annotation);

        assert (annotations_.size() == rows_.size());
        assert (result_ptrs_.size() == rows_.size());
    }

    void SectionContentTable::addToLayout (QVBoxLayout* layout)
    {
        assert (layout);

        if (!proxy_model_)
        {
            proxy_model_ = new QSortFilterProxyModel();
            proxy_model_->setSourceModel(this);
        }

        if (!table_view_)
        {
            table_view_ = new QTableView();
            table_view_->setModel(proxy_model_);

            if (sortable_)
            {
                table_view_->setSortingEnabled(true);
                table_view_->sortByColumn(sort_column_, order_);
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
        }

        table_view_->resizeColumnsToContents();
        table_view_->resizeRowsToContents();

        layout->addWidget(table_view_);

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

        if (!table_view_)
        {
            table_view_ = new QTableView();
            table_view_->setModel(proxy_model_);

            if (sortable_)
            {
                table_view_->setSortingEnabled(true);
                table_view_->sortByColumn(sort_column_, order_);
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
            // get string can convert to latex
            result.push_back(String::latexString(proxy_model_->data(index).toString().toStdString()));
        }

        return result;
    }

    bool SectionContentTable::hasReference (unsigned int row) const
    {
        if (!proxy_model_)
        {
            proxy_model_ = new QSortFilterProxyModel();
            SectionContentTable* tmp = const_cast<SectionContentTable*>(this); // hacky
            assert (tmp);
            proxy_model_->setSourceModel(tmp);
        }

        assert (row < proxy_model_->rowCount());
        assert (row < rows_.size());

        QModelIndex index = proxy_model_->index(row, 0);
        assert (index.isValid());

        auto const source_index = proxy_model_->mapToSource(index);
        assert (source_index.isValid());

        unsigned int row_index = source_index.row();

        return result_ptrs_.at(row_index)
                && result_ptrs_.at(row_index)->hasReference(*this, annotations_.at(row_index));
    }

    std::string SectionContentTable::reference (unsigned int row) const
    {
        if (!proxy_model_)
        {
            proxy_model_ = new QSortFilterProxyModel();
            SectionContentTable* tmp = const_cast<SectionContentTable*>(this); // hacky
            assert (tmp);
            proxy_model_->setSourceModel(tmp);
        }

        assert (row < proxy_model_->rowCount());
        assert (row < rows_.size());

        QModelIndex index = proxy_model_->index(row, 0);
        assert (index.isValid());

        auto const source_index = proxy_model_->mapToSource(index);
        assert (source_index.isValid());

        unsigned int row_index = source_index.row();

        assert (result_ptrs_.at(row_index)
                && result_ptrs_.at(row_index)->hasReference(*this, annotations_.at(row_index)));

        string tmp = result_ptrs_.at(row_index)->reference(*this, annotations_.at(row_index));
        //e.g. "Report:Results:"+getRequirementSectionID();
        assert (tmp.size() >= 14);

        if (tmp == "Report:Results")
            return "";

        assert (tmp.rfind("Report:Results:", 0) == 0);
        tmp.erase(0,15);

        return tmp;;
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

            QAction* action = new QAction("Show Full UTN", this);
            connect (action, &QAction::triggered, this, &SectionContentTable::showFullUTNSlot);
            action->setData(utn);
            menu.addAction(action);

            QAction* action2 = new QAction("Show Surrounding Data", this);
            connect (action2, &QAction::triggered, this, &SectionContentTable::showSurroundingDataSlot);
            action2->setData(utn);
            menu.addAction(action2);

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

        eval_man_.useUTN(utn, true, true);
    }

    void SectionContentTable::removeUTNSlot ()
    {
        QAction* action = dynamic_cast<QAction*> (QObject::sender());
        assert (action);

        unsigned int utn = action->data().toUInt();

        loginf << "SectionContentTable: removeUTNSlot: utn " << utn;

        eval_man_.useUTN(utn, false, true);
    }

    void SectionContentTable::showFullUTNSlot ()
    {
        QAction* action = dynamic_cast<QAction*> (QObject::sender());
        assert (action);

        unsigned int utn = action->data().toUInt();

        loginf << "SectionContentTable: showFullUTNSlot: utn " << utn;

        eval_man_.showFullUTN(utn);
    }

    void SectionContentTable::showSurroundingDataSlot ()
    {
        QAction* action = dynamic_cast<QAction*> (QObject::sender());
        assert (action);

        unsigned int utn = action->data().toUInt();

        loginf << "SectionContentTable: showSurroundingDataSlot: utn " << utn;

        eval_man_.showSurroundingData(utn);
    }

}
