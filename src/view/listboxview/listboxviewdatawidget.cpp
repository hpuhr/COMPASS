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

#include "listboxviewdatawidget.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QTabWidget>

#include "allbuffertablewidget.h"
#include "atsdb.h"
#include "buffer.h"
#include "buffertablewidget.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "listboxviewdatasource.h"
#include "logger.h"

ListBoxViewDataWidget::ListBoxViewDataWidget(ListBoxView* view, ListBoxViewDataSource* data_source,
                                             QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), view_(view), data_source_(data_source)
{
    assert(data_source_);

    QHBoxLayout* layout = new QHBoxLayout();

    tab_widget_ = new QTabWidget();
    layout->addWidget(tab_widget_);

    for (auto& obj_it : ATSDB::instance().objectManager())
    {
        if (obj_it.second->hasData())
        {
            if (!all_buffer_table_widget_)
            {
                all_buffer_table_widget_ = new AllBufferTableWidget(*view_, *data_source_);
                tab_widget_->addTab(all_buffer_table_widget_, "All");
                connect(all_buffer_table_widget_, &AllBufferTableWidget::exportDoneSignal, this,
                        &ListBoxViewDataWidget::exportDoneSlot);
                connect(this, &ListBoxViewDataWidget::showOnlySelectedSignal,
                        all_buffer_table_widget_, &AllBufferTableWidget::showOnlySelectedSlot);
                connect(this, &ListBoxViewDataWidget::usePresentationSignal,
                        all_buffer_table_widget_, &AllBufferTableWidget::usePresentationSlot);
                connect(this, &ListBoxViewDataWidget::showAssociationsSignal,
                        all_buffer_table_widget_, &AllBufferTableWidget::showAssociationsSlot);
            }

            BufferTableWidget* buffer_table =
                new BufferTableWidget(*obj_it.second, *view_, *data_source_);
            tab_widget_->addTab(buffer_table, obj_it.first.c_str());
            buffer_tables_[obj_it.first] = buffer_table;
            connect(buffer_table, &BufferTableWidget::exportDoneSignal, this,
                    &ListBoxViewDataWidget::exportDoneSlot);
            connect(this, &ListBoxViewDataWidget::showOnlySelectedSignal, buffer_table,
                    &BufferTableWidget::showOnlySelectedSlot);
            connect(this, &ListBoxViewDataWidget::usePresentationSignal, buffer_table,
                    &BufferTableWidget::usePresentationSlot);
            connect(this, &ListBoxViewDataWidget::showAssociationsSignal, buffer_table,
                    &BufferTableWidget::showAssociationsSlot);
        }
    }

    setLayout(layout);
}

ListBoxViewDataWidget::~ListBoxViewDataWidget()
{
    // TODO
    // buffer_tables_.clear();
}

void ListBoxViewDataWidget::clearTables()
{
    logdbg << "ListBoxViewDataWidget: updateTables: start";
    // TODO
    //  std::map <DB_OBJECT_TYPE, BufferTableWidget*>::iterator it;

    //  for (it = buffer_tables_.begin(); it != buffer_tables_.end(); it++)
    //  {
    //    it->second->show (0, 0, false);
    //  }

    logdbg << "ListBoxViewDataWidget: updateTables: end";
}

void ListBoxViewDataWidget::loadingStartedSlot()
{
    if (all_buffer_table_widget_)
        all_buffer_table_widget_->clear();

    for (auto buffer_table : buffer_tables_)
        buffer_table.second->clear();
}

void ListBoxViewDataWidget::updateDataSlot(DBObject& object, std::shared_ptr<Buffer> buffer)
{
    logdbg << "ListBoxViewDataWidget: updateTables: start";

    assert(all_buffer_table_widget_);
    all_buffer_table_widget_->show(buffer);

    assert(buffer_tables_.count(object.name()) > 0);
    buffer_tables_.at(object.name())->show(buffer);

    logdbg << "ListBoxViewDataWidget: updateTables: end";
}

void ListBoxViewDataWidget::exportDataSlot(bool overwrite)
{
    logdbg << "ListBoxViewDataWidget: exportDataSlot";
    assert(tab_widget_);

    AllBufferTableWidget* all_buffer_widget =
        dynamic_cast<AllBufferTableWidget*>(tab_widget_->currentWidget());

    BufferTableWidget* buffer_widget =
        dynamic_cast<BufferTableWidget*>(tab_widget_->currentWidget());

    if (all_buffer_widget && !buffer_widget)
    {
        all_buffer_widget->exportSlot(overwrite);
        return;
    }

    if (!all_buffer_widget && !buffer_widget)
    {
        QMessageBox msgBox;
        msgBox.setText("Export can not be used without loaded data.");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();

        exportDoneSignal(true);
        return;
    }

    buffer_widget->exportSlot(overwrite);
}

void ListBoxViewDataWidget::exportDoneSlot(bool cancelled) { emit exportDoneSignal(cancelled); }

void ListBoxViewDataWidget::showOnlySelectedSlot(bool value)
{
    loginf << "ListBoxViewDataWidget: showOnlySelectedSlot: " << value;
    emit showOnlySelectedSignal(value);
}

void ListBoxViewDataWidget::usePresentationSlot(bool use_presentation)
{
    loginf << "ListBoxViewDataWidget: usePresentationSlot";

    emit usePresentationSignal(use_presentation);
}

void ListBoxViewDataWidget::showAssociationsSlot(bool value)
{
    loginf << "ListBoxViewDataWidget: showAssociationsSlot: " << value;
    emit showAssociationsSignal(value);
}

void ListBoxViewDataWidget::resetModels()
{
    if (all_buffer_table_widget_)
        all_buffer_table_widget_->resetModel();

    for (auto& table_widget_it : buffer_tables_)
        table_widget_it.second->resetModel();
}

void ListBoxViewDataWidget::updateToSelection()
{
    if (all_buffer_table_widget_)
        all_buffer_table_widget_->updateToSelection();

    for (auto& table_widget_it : buffer_tables_)
        table_widget_it.second->updateToSelection();
}
