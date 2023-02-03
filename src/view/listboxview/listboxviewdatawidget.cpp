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

#include "listboxviewdatawidget.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QTabWidget>

#include "allbuffertablewidget.h"
#include "compass.h"
#include "buffer.h"
#include "buffertablewidget.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "listboxviewdatasource.h"
#include "logger.h"

ListBoxViewDataWidget::ListBoxViewDataWidget(ListBoxView* view, 
                                             ListBoxViewDataSource* data_source,
                                             QWidget* parent, 
                                             Qt::WindowFlags f)
:   ViewDataWidget(parent, f), view_(view), data_source_(data_source)
{
    assert(data_source_);

    QHBoxLayout* layout = new QHBoxLayout();

    tab_widget_ = new QTabWidget();
    layout->addWidget(tab_widget_);

    for (auto& obj_it : COMPASS::instance().dbContentManager())
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
    }

    setLayout(layout);
}

ListBoxViewDataWidget::~ListBoxViewDataWidget()
{
    // TODO
    // buffer_tables_.clear();
}

void ListBoxViewDataWidget::clearData()
{
    logdbg << "ListBoxViewDataWidget: clearData";

    buffers_.clear();

    if (all_buffer_table_widget_)
        all_buffer_table_widget_->clear();

    for (auto buffer_table : buffer_tables_)
        buffer_table.second->clear();

    logdbg << "ListBoxViewDataWidget: clearData: end";
}

void ListBoxViewDataWidget::loadingStartedSlot()
{
    clearData();
}

void ListBoxViewDataWidget::updateDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data,
                                           bool requires_reset)
{
    loginf << "ListBoxViewDataWidget: updateTables";

//    assert(all_buffer_table_widget_);
//    all_buffer_table_widget_->show(buffer);

//    assert(buffer_tables_.count(object.name()) > 0);
//    buffer_tables_.at(object.name())->show(buffer);

    buffers_ = data;

    logdbg << "ListBoxViewDataWidget: updateTables: end";
}

void ListBoxViewDataWidget::loadingDoneSlot()
{
    loginf << "ListBoxViewDataWidget: loadingDoneSlot";

    assert(all_buffer_table_widget_);
    all_buffer_table_widget_->show(buffers_);

    for (auto& buf_it : buffers_)
    {
        assert(buffer_tables_.count(buf_it.first) > 0);
        buffer_tables_.at(buf_it.first)->show(buf_it.second);
    }

    selectFirstSelectedRow();

    emit dataLoaded();
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

void ListBoxViewDataWidget::selectFirstSelectedRow()
{
    if (all_buffer_table_widget_)
        all_buffer_table_widget_->selectSelectedRows();
}

AllBufferTableWidget* ListBoxViewDataWidget::getAllBufferTableWidget ()
{
    assert (all_buffer_table_widget_);
    return all_buffer_table_widget_;
}

void ListBoxViewDataWidget::toolChanged_impl(int mode)
{
    //nothing to do here
}
