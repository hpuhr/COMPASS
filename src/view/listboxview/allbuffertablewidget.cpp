/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */


//#include <iostream>

#include <QTableView>
#include <QVBoxLayout>
#include <QClipboard>
#include <QKeyEvent>
#include <QApplication>
#include <QFileDialog>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "buffer.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "logger.h"
#include "allbuffertablewidget.h"
#include "allbuffertablemodel.h"
#include "viewselection.h"
#include "listboxviewdatasource.h"

//using namespace Utils;

AllBufferTableWidget::AllBufferTableWidget(ListBoxView& view, ListBoxViewDataSource& data_source,
                                     QWidget* parent, Qt::WindowFlags f)
: QWidget (parent, f), view_(view), data_source_(data_source)
{
    setAutoFillBackground(true);

    QVBoxLayout *layout = new QVBoxLayout ();

    table_ = new QTableView (this);
    model_ = new AllBufferTableModel (this, data_source_);
    table_->setModel(model_);

    connect (model_, SIGNAL(exportDoneSignal(bool)), this, SLOT(exportDoneSlot(bool)));

    layout->addWidget (table_);
    table_->show();

    setLayout (layout);

}

AllBufferTableWidget::~AllBufferTableWidget()
{
}



void AllBufferTableWidget::clear ()
{
    assert (model_);

    model_->clearData();
}

void AllBufferTableWidget::show (std::shared_ptr<Buffer> buffer) //, DBOVariableSet *variables, bool database_view
{
    assert (buffer);

    logdbg  << "AllBufferTableWidget: show: buffer size " << buffer->size() << " properties "
            << buffer->properties().size();
    assert (table_);
    assert (model_);

    model_->setData(buffer);
    table_->resizeColumnsToContents();

    logdbg  << " AllBufferTableWidget: show: end";
}

void AllBufferTableWidget::exportSlot(bool overwrite)
{
    loginf << "AllBufferTableWidget: exportSlot";

    QString file_name;
    if (overwrite)
    {
        file_name = QFileDialog::getSaveFileName(this, "Save All as CSV", "",
                                                 tr("Comma-separated values (*.csv);;All Files (*)"));
    }
    else
    {
        file_name = QFileDialog::getSaveFileName(this, "Save All as CSV", "",
                                                 tr("Comma-separated values (*.csv);;All Files (*)"), nullptr,
                                                 QFileDialog::DontConfirmOverwrite);
    }

    if (file_name.size())
    {
        loginf << "AllBufferTableWidget: exportSlot: export filename " << file_name.toStdString();
        assert (model_);
        model_->saveAsCSV(file_name.toStdString(), overwrite);
    }
    else
    {
        emit exportDoneSignal (true);
    }
}

void AllBufferTableWidget::exportDoneSlot (bool cancelled)
{
    emit exportDoneSignal (cancelled);
}

void AllBufferTableWidget::showOnlySelectedSlot (bool value)
{
    loginf << "AllBufferTableWidget: showOnlySelectedSlot: " << value;

    assert (model_);
    model_->showOnlySelected(value);
    assert (table_);
    table_->resizeColumnsToContents();
}

void AllBufferTableWidget::usePresentationSlot (bool use_presentation)
{
    assert (model_);
    model_->usePresentation(use_presentation);
    assert (table_);
    table_->resizeColumnsToContents();
}

void AllBufferTableWidget::showAssociationsSlot (bool value)
{
    assert (model_);
    model_->showAssociations(value);
    assert (table_);
    table_->resizeColumnsToContents();
}


void AllBufferTableWidget::resetModel()
{
    assert (model_);
    model_->reset();
}

void AllBufferTableWidget::updateToSelection ()
{
    assert (model_);
    model_->updateToSelection();
    assert (table_);
    table_->resizeColumnsToContents();
}

void AllBufferTableWidget::resizeColumns()
{
    assert (table_);
    table_->resizeColumnsToContents();
}

ListBoxView &AllBufferTableWidget::view() const
{
    return view_;
}

//void AllBufferTableWidget::itemChanged (QTableWidgetItem *item)
//{
//    if (selection_checkboxes_.find (item) != selection_checkboxes_.end())
//    {
//        unsigned int id = selection_checkboxes_[item];
//        bool checked = item->checkState() == Qt::Checked;
//        logdbg  << "AllBufferTableWidget: itemChanged: id " << id << " checked " << checked;

//        if (checked) // add
//        {
//            ViewSelectionEntries entries;
//            entries.push_back(ViewSelectionEntry (ViewSelectionId(type_, id), ViewSelectionEntry::TYPE_BILLBOARD));
//            ViewSelection::getInstance().addSelection(entries);
//        }
//        else // remove
//        {
//            ViewSelectionEntries selection_entries = ViewSelection::getInstance().getEntries();
//            ViewSelectionEntries::iterator it;

//            bool found=false;
//            for (it = selection_entries.begin(); it != selection_entries.end(); it++)
//            {
//                if (it->id_.first == type_ && it->id_.second == id)
//                {
//                    found = true;
//                    selection_entries.erase (it);
//                    break;
//                }
//            }

//            if (found)
//            {
//                logdbg  << "AllBufferTableWidget: itemChanged: unselecting type " << type_ << " id " << id;
//                ViewSelection::getInstance().clearSelection();
//                ViewSelection::getInstance().setSelection(selection_entries);
//            }
//            else
//            {
//                logwrn  << "AllBufferTableWidget: itemChanged: unselect failed for type " << type_ << " id " << id;
//            }
//        }
//    }
//    else
//        logerr << "AllBufferTableWidget: itemChanged: unknown table item";
//}

//void AllBufferTableWidget::keyPressEvent ( QKeyEvent * event )
//{
//    logdbg  << "AllBufferTableWidget: keyPressEvent: got keypressed";

//    assert (table_);

    //TODO
//    if (event->modifiers()  & Qt::ControlModifier)
//    {
//        if (event->key() == Qt::Key_C)
//        {
//            QAbstractItemModel *abmodel = table_->model();
//            QItemSelectionModel * model = table_->selectionModel();
//            QModelIndexList list = model->selectedIndexes();

//            qSort(list);

//            if(list.size() < 1)
//                return;

//            int min_col=0, max_col=0, min_row=0, max_row=0;

//            for(int i = 0; i < list.size(); i++)
//            {
//                QModelIndex index = list.at(i);

//                int row = index.row();
//                int col = index.column();

//                if (i==0)
//                {
//                    min_col=col;
//                    max_col=col;
//                    min_row=row;
//                    max_row=row;
//                }

//                if (row < min_row)
//                    min_row=row;
//                if (row > max_row)
//                    max_row=row;

//                if (col < min_col)
//                    min_col=col;
//                if (col > max_col)
//                    max_col=col;
//            }

//            int rows = max_row-min_row+1;
//            int cols = max_col-min_col+1;

//            if (rows < 1)
//            {
//                logwrn  << "AllBufferTableWidget: keyPressEvent: too few rows " << rows;
//                return;
//            }
//            logdbg  << "AllBufferTableWidget: keyPressEvent: rows " << rows;

//            if (cols < 1)
//            {
//                logwrn  << "AllBufferTableWidget: keyPressEvent: too few cols " << cols;
//                return;
//            }
//            logdbg  << "AllBufferTableWidget: keyPressEvent: cols " << cols;

//            std::vector < std::vector <std::string> > table_strings (rows,std::vector<std::string> (cols));

//            for(int i = 0; i < list.size(); i++)
//            {
//                QModelIndex index = list.at(i);
//                QVariant data = abmodel->data(index);
//                QString text = data.toString();

//                QTableWidgetItem *item = table_->item (index.row(), index.column());
//                if( item->checkState() == Qt::Checked )
//                    text = "X";

//                table_strings.at(index.row()-min_row).at(index.column()-min_col) = "\""+text.toStdString()+"\"";
//            }

//            QString copy_table;
//            for(int i = 0; i < (int)table_strings.size(); i++)
//            {
//                std::vector <std::string> &row_strings = table_strings.at(i);
//                for(int j = 0; j < (int)row_strings.size(); j++)
//                {
//                    if (j != 0)
//                        copy_table.append('\t');
//                    copy_table.append(row_strings.at(j).c_str());
//                }
//                copy_table.append('\n');
//            }

//            // make col indexes
//            std::set<unsigned int> col_indexes;
//            for (int i = min_col; i <= max_col; i++)
//                col_indexes.insert(i);

//            //set header
//            std::set<unsigned int>::iterator it;
//            QString header_string;
//            for (it = col_indexes.begin(); it != col_indexes.end(); it++)
//            {
//                if (it != col_indexes.begin())
//                    header_string.append('\t');
//                assert (*it < (unsigned int) header_list_.size());
//                header_string.append(header_list_.at(*it));
//            }
//            header_string.append('\n');

//            QClipboard *clipboard = QApplication::clipboard();
//            clipboard->setText(header_string+copy_table);
//        }
//    }
//}
