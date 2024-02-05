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
#include "listboxviewwidget.h"
#include "listboxview.h"
#include "allbuffertablewidget.h"
#include "compass.h"
#include "buffer.h"
#include "buffertablewidget.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "listboxviewdatasource.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QTabWidget>
#include <QTableView>
#include <QHeaderView>

ListBoxViewDataWidget::ListBoxViewDataWidget(ListBoxViewWidget* view_widget, 
                                             QWidget* parent, 
                                             Qt::WindowFlags f)
:   ViewDataWidget(view_widget, parent, f)
{
    view_ = view_widget->getView();
    assert(view_);

    data_source_ = view_->getDataSource();
    assert(data_source_);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setMargin(0);

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

void ListBoxViewDataWidget::clearData_impl()
{
    logdbg << "ListBoxViewDataWidget: clearData_impl: begin";

    if (all_buffer_table_widget_)
        all_buffer_table_widget_->clear();

    for (auto buffer_table : buffer_tables_)
        buffer_table.second->clear();

    logdbg << "ListBoxViewDataWidget: clearData_impl: end";
}

void ListBoxViewDataWidget::loadingStarted_impl()
{
    loginf << "ListBoxViewDataWidget: loadingStarted_impl";
    //nothing to do yet
}

void ListBoxViewDataWidget::updateData_impl(bool requires_reset)
{
    logdbg << "ListBoxViewDataWidget: updateData_impl: begin";

    //nothing to do yet

    //    assert(all_buffer_table_widget_);
    //    all_buffer_table_widget_->show(buffer);

    //    assert(buffer_tables_.count(object.name()) > 0);
    //    buffer_tables_.at(object.name())->show(buffer);

    logdbg << "ListBoxViewDataWidget: updateData_impl: end";
}

void ListBoxViewDataWidget::loadingDone_impl()
{
    logdbg << "ListBoxViewDataWidget: loadingDone_impl: begin";

    //default behavior
    ViewDataWidget::loadingDone_impl();

    logdbg << "ListBoxViewDataWidget: loadingDone_impl: end";
}

bool ListBoxViewDataWidget::redrawData_impl(bool recompute)
{
    logdbg << "ListBoxViewDataWidget: redrawData_impl: start - recompute = " << recompute;

    assert(all_buffer_table_widget_);
    all_buffer_table_widget_->show(viewData());

    for (auto& buf_it : viewData())
    {
        assert(buffer_tables_.count(buf_it.first) > 0);
        buffer_tables_.at(buf_it.first)->show(buf_it.second);
    }

    selectFirstSelectedRow();

    logdbg << "ListBoxViewDataWidget: redrawData_impl: end";

    return (all_buffer_table_widget_->rowCount() > 0);
}

void ListBoxViewDataWidget::liveReload_impl()
{
    //implement live reload behavior here
}

void ListBoxViewDataWidget::exportDataSlot()
{
    logdbg << "ListBoxViewDataWidget: exportDataSlot";
    assert(tab_widget_);

    AllBufferTableWidget* all_buffer_widget =
        dynamic_cast<AllBufferTableWidget*>(tab_widget_->currentWidget());

    BufferTableWidget* buffer_widget =
        dynamic_cast<BufferTableWidget*>(tab_widget_->currentWidget());

    if (all_buffer_widget && !buffer_widget)
    {
        all_buffer_widget->exportSlot();
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

    buffer_widget->exportSlot();
}

void ListBoxViewDataWidget::exportDoneSlot(bool cancelled) 
{ 
    emit exportDoneSignal(cancelled); 
}

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

void ListBoxViewDataWidget::viewInfoJSON_impl(nlohmann::json& info) const
{
    nlohmann::json table_infos = nlohmann::json::array();

    auto addTable = [ & ] (const std::string& db_content, 
                           const QTableView* table,
                           bool show_only_selected,
                           bool use_presentation)
    {
        nlohmann::json table_info;

        table_info[ "content"            ] = db_content;
        table_info[ "show_only_selected" ] = show_only_selected;
        table_info[ "use_presentation"   ] = use_presentation;
        table_info[ "count"              ] = table->model()->rowCount();

        //get properties
        std::vector<std::string> properties;
        for(int i = 0; i < table->model()->columnCount(); i++)
            properties.push_back(table->model()->headerData(i, Qt::Horizontal).toString().toStdString());
        
        table_info[ "properties" ] = properties;

        //get line zero
        std::vector<std::string> line0; 
        if (table->model()->rowCount() > 0)
        {
            for(int i = 0; i < table->model()->columnCount(); i++)
            {
                auto index = table->model()->index(0, i);
                line0.push_back(table->model()->data(index, Qt::DisplayRole).toString().toStdString());
            }
        }

        table_info[ "line0" ] = line0;

        table_infos.push_back(table_info);
    };

    addTable("All", 
             all_buffer_table_widget_->table(), 
             all_buffer_table_widget_->showOnlySelected(), 
             all_buffer_table_widget_->usePresentation());

    for (const auto& it : buffer_tables_)
    {
        addTable(it.first, 
                 it.second->table(), 
                 it.second->showOnlySelected(), 
                 it.second->usePresentation());
    }

    info[ "tables" ] = table_infos;
}
