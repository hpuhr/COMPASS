/*
 * BufferTableWidget.cpp
 *
 *  Created on: Nov 12, 2012
 *      Author: sk
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
#include "buffertablewidget.h"
#include "buffertablemodel.h"
#include "viewselection.h"
//#include "Data.h"

//using namespace Utils;

BufferTableWidget::BufferTableWidget(DBObject &object, QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), object_(object), table_ (nullptr), model_(nullptr), variables_(nullptr)
{
    setAutoFillBackground(true);

    QVBoxLayout *layout = new QVBoxLayout ();
//    table_ = new QTableWidget ();
//    table_->setAlternatingRowColors(true);

    table_ = new QTableView (this);
    model_ = new BufferTableModel (this, object_);
    table_->setModel(model_);

    //connect( table_, SIGNAL( itemClicked( QTableWidgetItem * )), this, SLOT( itemChanged ( QTableWidgetItem * )));

    layout->addWidget (table_);
    table_->show();

    setLayout (layout);

}

BufferTableWidget::~BufferTableWidget()
{
}

//void BufferTableWidget::itemChanged (QTableWidgetItem *item)
//{
//    if (selection_checkboxes_.find (item) != selection_checkboxes_.end())
//    {
//        unsigned int id = selection_checkboxes_[item];
//        bool checked = item->checkState() == Qt::Checked;
//        logdbg  << "BufferTableWidget: itemChanged: id " << id << " checked " << checked;

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
//                logdbg  << "BufferTableWidget: itemChanged: unselecting type " << type_ << " id " << id;
//                ViewSelection::getInstance().clearSelection();
//                ViewSelection::getInstance().setSelection(selection_entries);
//            }
//            else
//            {
//                logwrn  << "BufferTableWidget: itemChanged: unselect failed for type " << type_ << " id " << id;
//            }
//        }
//    }
//    else
//        logerr << "BufferTableWidget: itemChanged: unknown table item";
//}

void BufferTableWidget::keyPressEvent ( QKeyEvent * event )
{
    logdbg  << "BufferTableWidget: keyPressEvent: got keypressed";

    assert (table_);

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
//                logwrn  << "BufferTableWidget: keyPressEvent: too few rows " << rows;
//                return;
//            }
//            logdbg  << "BufferTableWidget: keyPressEvent: rows " << rows;

//            if (cols < 1)
//            {
//                logwrn  << "BufferTableWidget: keyPressEvent: too few cols " << cols;
//                return;
//            }
//            logdbg  << "BufferTableWidget: keyPressEvent: cols " << cols;

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
}

void BufferTableWidget::clear ()
{
    assert (model_);

    model_->clearData();
}

void BufferTableWidget::show (std::shared_ptr<Buffer> buffer) //, DBOVariableSet *variables, bool database_view
{
    assert (buffer);

    logdbg  << "BufferTableWidget: show: object " << object_.name() << " buffer size " << buffer->size() << " properties " << buffer->properties().size();
    assert (table_);
    assert (model_);

    model_->setData(buffer);
    table_->resizeColumnsToContents();

//    ViewSelectionEntries &selection_entries = ViewSelection::getInstance().getEntries();
//    ViewSelectionEntries::iterator it;
//    std::map <unsigned int, QTableWidgetItem*>::iterator it2;

//    logdbg  << "BufferTableWidget: show: selection size " << selection_entries.size();

//    for (it = selection_entries.begin(); it != selection_entries.end(); it++)
//    {
//        ViewSelectionEntry &entry=*it;
//        if (entry.isDBO())
//        {
//            if ((unsigned int) entry.id_.first == type_)
//            {
//                //loginf << "BufferTableWidget: show: check at " << entry.id_.second << " is correct type"<< endl;
//                it2 = selected_items.find(entry.id_.second);
//                if (it2 != selected_items.end())
//                {
//                    it2->second->setCheckState ( Qt::Checked );
//                }
//            }
//        }
//    }

    logdbg  << " BufferTableWidget: show: end";
}

void BufferTableWidget::exportSlot()
{
    loginf << "BufferTableWidget: exportSlot: object " << object_.name();

    QString file_name = QFileDialog::getSaveFileName(this,
            ("Save "+object_.name()+" as CSV").c_str(), "",
            tr("Comma-separated values (*.csv);;All Files (*)"));

    if (file_name.size())
    {

    }
}
