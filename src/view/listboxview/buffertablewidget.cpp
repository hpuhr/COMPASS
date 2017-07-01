/*
 * BufferTableWidget.cpp
 *
 *  Created on: Nov 12, 2012
 *      Author: sk
 */

#include <QTableWidget>
#include <QVBoxLayout>
#include <QClipboard>
#include <QKeyEvent>
#include <QApplication>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "Buffer.h"
#include "DBOVariable.h"
#include "DBOVariableSet.h"
#include "DBObjectManager.h"
#include "Logger.h"
#include "BufferTableWidget.h"
#include "ViewSelection.h"
#include "Data.h"

using namespace Utils::Data;

BufferTableWidget::BufferTableWidget(QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), table_ (0), variables_(0)
{
    createGUIElements();
}

BufferTableWidget::~BufferTableWidget()
{
}

void BufferTableWidget::itemChanged (QTableWidgetItem *item)
{
    if (selection_checkboxes_.find (item) != selection_checkboxes_.end())
    {
        unsigned int id = selection_checkboxes_[item];
        bool checked = item->checkState() == Qt::Checked;
        logdbg  << "BufferTableWidget: itemChanged: id " << id << " checked " << checked;

        if (checked) // add
        {
            ViewSelectionEntries entries;
            entries.push_back(ViewSelectionEntry (ViewSelectionId(type_, id), ViewSelectionEntry::TYPE_BILLBOARD));
            ViewSelection::getInstance().addSelection(entries);
        }
        else // remove
        {
            ViewSelectionEntries selection_entries = ViewSelection::getInstance().getEntries();
            ViewSelectionEntries::iterator it;

            bool found=false;
            for (it = selection_entries.begin(); it != selection_entries.end(); it++)
            {
                if (it->id_.first == type_ && it->id_.second == id)
                {
                    found = true;
                    selection_entries.erase (it);
                    break;
                }
            }

            if (found)
            {
                logdbg  << "BufferTableWidget: itemChanged: unselecting type " << type_ << " id " << id;
                ViewSelection::getInstance().clearSelection();
                ViewSelection::getInstance().setSelection(selection_entries);
            }
            else
            {
                logwrn  << "BufferTableWidget: itemChanged: unselect failed for type " << type_ << " id " << id;
            }
        }
    }
    else
        logerr << "BufferTableWidget: itemChanged: unknown table item";
}

void BufferTableWidget::createGUIElements ()
{
    QVBoxLayout *layout = new QVBoxLayout ();
    table_ = new QTableWidget ();
    table_->setAlternatingRowColors(true);

    connect( table_, SIGNAL( itemClicked( QTableWidgetItem * )), this, SLOT( itemChanged ( QTableWidgetItem * )));

    layout->addWidget (table_);
    setLayout (layout);
}

void BufferTableWidget::keyPressEvent ( QKeyEvent * event )
{
    logdbg  << "BufferTableWidget: keyPressEvent: got keypressed";

    assert (table_);

    if (event->modifiers()  & Qt::ControlModifier)
    {
        if (event->key() == Qt::Key_C)
        {
            QAbstractItemModel *abmodel = table_->model();
            QItemSelectionModel * model = table_->selectionModel();
            QModelIndexList list = model->selectedIndexes();

            qSort(list);

            if(list.size() < 1)
                return;

            int min_col=0, max_col=0, min_row=0, max_row=0;

            for(int i = 0; i < list.size(); i++)
            {
                QModelIndex index = list.at(i);

                int row = index.row();
                int col = index.column();

                if (i==0)
                {
                    min_col=col;
                    max_col=col;
                    min_row=row;
                    max_row=row;
                }

                if (row < min_row)
                    min_row=row;
                if (row > max_row)
                    max_row=row;

                if (col < min_col)
                    min_col=col;
                if (col > max_col)
                    max_col=col;
            }

            int rows = max_row-min_row+1;
            int cols = max_col-min_col+1;

            if (rows < 1)
            {
                logwrn  << "BufferTableWidget: keyPressEvent: too few rows " << rows;
                return;
            }
            logdbg  << "BufferTableWidget: keyPressEvent: rows " << rows;

            if (cols < 1)
            {
                logwrn  << "BufferTableWidget: keyPressEvent: too few cols " << cols;
                return;
            }
            logdbg  << "BufferTableWidget: keyPressEvent: cols " << cols;

            std::vector < std::vector <std::string> > table_strings (rows,std::vector<std::string> (cols));

            for(int i = 0; i < list.size(); i++)
            {
                QModelIndex index = list.at(i);
                QVariant data = abmodel->data(index);
                QString text = data.toString();

                QTableWidgetItem *item = table_->item (index.row(), index.column());
                if( item->checkState() == Qt::Checked )
                    text = "X";

                table_strings.at(index.row()-min_row).at(index.column()-min_col) = "\""+text.toStdString()+"\"";
            }

            QString copy_table;
            for(int i = 0; i < (int)table_strings.size(); i++)
            {
                std::vector <std::string> &row_strings = table_strings.at(i);
                for(int j = 0; j < (int)row_strings.size(); j++)
                {
                    if (j != 0)
                        copy_table.append('\t');
                    copy_table.append(row_strings.at(j).c_str());
                }
                copy_table.append('\n');
            }

            // make col indexes
            std::set<unsigned int> col_indexes;
            for (int i = min_col; i <= max_col; i++)
                col_indexes.insert(i);

            //set header
            std::set<unsigned int>::iterator it;
            QString header_string;
            for (it = col_indexes.begin(); it != col_indexes.end(); it++)
            {
                if (it != col_indexes.begin())
                    header_string.append('\t');
                assert (*it < (unsigned int) header_list_.size());
                header_string.append(header_list_.at(*it));
            }
            header_string.append('\n');

            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(header_string+copy_table);
        }
    }
}

void BufferTableWidget::show (Buffer *buffer, DBOVariableSet *variables, bool database_view)
{
    loginf  << " BufferTableWidget: show: start";
    assert (table_);
    table_->clear();

    selection_checkboxes_.clear();

    if (buffer == 0)
    {
        logwrn << "BufferTableWidget: show: no buffer";
        return;
    }
    if (variables == 0)
    {
        logwrn << "BufferTableWidget: show: empty variables";
        return;
    }
    if (buffer->getFirstWrite())
    {
        logwrn << "BufferTableWidget: show: empty buffer";
        return;
    }

    QFont font_italic;
    font_italic.setItalic(true);

    variables_ = variables;

    //assert (variables_->getSize() == buffer->getPropertyList()->getNumProperties());

    table_->setRowCount(buffer->getSize());
    table_->setColumnCount(variables_->getSize()+1);

    header_list_.clear();
    std::vector <unsigned int> test_keys;

    header_list_.append (tr("Selected"));
    for (unsigned int cnt=0; cnt < variables_->getSize(); cnt++)
    {
        header_list_.append(variables_->getVariable(cnt)->getName().c_str());
    }
    table_->setHorizontalHeaderLabels (header_list_);

    std::vector<void*>* output_adresses;

    std::map <unsigned int, QTableWidgetItem*> selected_items;
    std::string id_name;
    unsigned int id_index;

    type_ = buffer->getDBOType();
    logdbg  << "BufferTableWidget: show: buffer type " << type_;
    assert (DBObjectManager::getInstance().existsDBOVariable (DBO_UNDEFINED, "id"));
    id_name = DBObjectManager::getInstance().getDBOVariable (DBO_UNDEFINED, "id")->getFor (type_)->id_;
    id_index = buffer->getPropertyList()->getPropertyIndex (id_name);

    PropertyList *proplist = buffer->getPropertyList();

    bool null=false;
    buffer->setIndex(0);
    for (unsigned int row=0; row < buffer->getSize(); row++)
    {
        if (row != 0)
            buffer->incrementIndex();

        output_adresses = buffer->getAdresses();

        QTableWidgetItem *newItem;

        for (unsigned int col=0; col < variables_->getSize(); col++)
        {
            std::string value_representation;

            std::string col_name = variables->getVariable(col)->id_;
            assert (proplist->hasProperty(col_name));
            unsigned int col_num =proplist->getPropertyIndex(col_name);

            null = isNan(variables_->getVariable(col)->data_type_int_, output_adresses->at( col_num ));

            if (!null)
            {
                std::string value = variables_->getVariable(col)->getValueFrom( output_adresses->at( col_num ) );
                if (database_view)
                    value_representation= value;
                else
                    value_representation= variables_->getVariable(col)->getRepresentationFromValue( value );
            }

            newItem = new QTableWidgetItem(value_representation.c_str());
            if (null)
                newItem->setBackgroundColor(Qt::lightGray);
            newItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            table_->setItem(row, col+1, newItem);
        }

        unsigned int id = *((unsigned int*) output_adresses->at( id_index ));

        newItem = new QTableWidgetItem();
        newItem->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
        newItem->setCheckState ( Qt::Unchecked );
        table_->setItem(row, 0, newItem);
        selection_checkboxes_ [newItem] = id;

        test_keys.push_back (id);
        assert (selected_items.find(id) == selected_items.end());
        selected_items [id] = newItem;

    }

    ViewSelectionEntries &selection_entries = ViewSelection::getInstance().getEntries();
    ViewSelectionEntries::iterator it;
    std::map <unsigned int, QTableWidgetItem*>::iterator it2;

    logdbg  << "BufferTableWidget: show: selection size " << selection_entries.size();

    for (it = selection_entries.begin(); it != selection_entries.end(); it++)
    {
        ViewSelectionEntry &entry=*it;
        if (entry.isDBO())
        {
            if ((unsigned int) entry.id_.first == type_)
            {
                //loginf << "BufferTableWidget: show: check at " << entry.id_.second << " is correct type"<< endl;
                it2 = selected_items.find(entry.id_.second);
                if (it2 != selected_items.end())
                {
                    it2->second->setCheckState ( Qt::Checked );
                }
            }
        }
    }

    table_->resizeColumnsToContents();

    logdbg  << " BufferTableWidget: show: end";
}

