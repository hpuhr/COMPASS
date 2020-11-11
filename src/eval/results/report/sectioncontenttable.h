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

#ifndef EVALUATIONRESULTSREPORTSECTIONCONTENTTABLE_H
#define EVALUATIONRESULTSREPORTSECTIONCONTENTTABLE_H

#include "eval/results/report/sectioncontent.h"

#include <QVariant>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QBrush>

#include "json.hpp"

#include <vector>

class ViewableDataConfig;

class QPushButton;
class QTableView;

namespace EvaluationRequirementResult
{
    class Base;
}

namespace EvaluationResultsReport
{
    using namespace std;

    class TableQSortFilterProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:

        TableQSortFilterProxyModel(QObject* parent=nullptr)
            : QSortFilterProxyModel(parent)
        {

        }

        bool showUnused() const
        {
            return show_unused_;
        }

        void showUnused(bool value)
        {
            //emit layoutAboutToBeChanged();

            show_unused_ = value;

            //emit layoutChanged();
        }

    protected:
        bool show_unused_ {true};

        bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
        {
            //return sourceModel()->row(source_row)[0].checkState() == Qt::Checked;

            QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

            if (show_unused_)
                return true;
            else
                return (sourceModel()->data(index, Qt::BackgroundRole) != QBrush(Qt::lightGray));
        }
    };


    class SectionContentTable : public QAbstractItemModel, public SectionContent
    {
        Q_OBJECT

    public slots:
        void currentRowChangedSlot(const QModelIndex& current, const QModelIndex& previous);
        void doubleClickedSlot(const QModelIndex& index);
        void customContextMenuSlot(const QPoint& p);
        void addUTNSlot ();
        void removeUTNSlot ();
        void showFullUTNSlot ();
        void showSurroundingDataSlot ();

        void toggleShowUnusedSlot();

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

        bool showUnused() const;
        void showUnused(bool value);

    protected:
        unsigned int num_columns_ {0};
        vector<string> headings_;

        bool sortable_ {true};
        unsigned int sort_column_ {0};
        Qt::SortOrder order_ {Qt::AscendingOrder};

        bool show_unused_ {false};

        vector<vector<QVariant>> rows_;
        vector<EvaluationRequirementResult::Base*> result_ptrs_;
        vector<QVariant> annotations_;

        mutable QPushButton* toogle_show_unused_button_ {nullptr};

        mutable TableQSortFilterProxyModel* proxy_model_ {nullptr};
        mutable QTableView* table_view_ {nullptr}; // for reset
    };

}
#endif // EVALUATIONRESULTSREPORTSECTIONCONTENTTABLE_H
