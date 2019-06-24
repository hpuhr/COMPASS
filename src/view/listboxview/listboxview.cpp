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


#include "listboxview.h"
#include "listboxviewwidget.h"
#include "listboxviewdatasource.h"
#include "listboxviewdatawidget.h"
#include "listboxviewconfigwidget.h"
#include "logger.h"
#include "viewselection.h"

ListBoxView::ListBoxView(const std::string& class_id, const std::string& instance_id, ViewContainer *w,
                         ViewManager &view_manager)
    : View (class_id, instance_id, w, view_manager)
{
    registerParameter ("use_presentation", &use_presentation_, true);
    registerParameter ("overwrite_csv", &overwrite_csv_, true);
}

ListBoxView::~ListBoxView()
{
    if (data_source_)
    {
        delete data_source_;
        data_source_ = nullptr;
    }

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

void ListBoxView::update( bool atOnce )
{

}

void ListBoxView::clearData()
{

}

bool ListBoxView::init()
{
    View::init();

    createSubConfigurables ();

    assert (data_source_);

    connect( data_source_, SIGNAL(loadingStartedSignal ()), widget_->getDataWidget (), SLOT(loadingStartedSlot()));
    connect( data_source_, SIGNAL(updateData (DBObject&, std::shared_ptr<Buffer>)), widget_->getDataWidget (), SLOT(updateData (DBObject&, std::shared_ptr<Buffer>)) );

    connect (widget_->configWidget(), SIGNAL(exportSignal(bool)), widget_->getDataWidget(), SLOT(exportDataSlot(bool)));
    connect (widget_->getDataWidget(), SIGNAL(exportDoneSignal(bool)), widget_->configWidget(), SLOT(exportDoneSlot(bool)));

    connect (this, SIGNAL(usePresentationSignal(bool)), widget_->getDataWidget(), SLOT(usePresentationSlot(bool)));

    widget_->getDataWidget()->usePresentationSlot(use_presentation_);

    return true;
}

void ListBoxView::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "ListBoxView: generateSubConfigurable: class_id " << class_id << " instance_id " << instance_id;
    if ( class_id == "ListBoxViewDataSource" )
    {
        assert (!data_source_);
        data_source_ = new ListBoxViewDataSource( class_id, instance_id, this );
    }
    else if( class_id == "ListBoxViewWidget" )
    {
        widget_ = new ListBoxViewWidget( class_id, instance_id, this, this, central_widget_ );
        setWidget( widget_ );
    }
    else
        throw std::runtime_error ("ListBoxView: generateSubConfigurable: unknown class_id "+class_id );
}

void ListBoxView::checkSubConfigurables ()
{
    if (!data_source_)
    {
        generateSubConfigurable ("ListBoxViewDataSource", "ListBoxViewDataSource0");
    }

    if( !widget_ )
    {
        generateSubConfigurable ("ListBoxViewWidget", "ListBoxViewWidget0");
    }
}

DBOVariableSet ListBoxView::getSet (const std::string& dbo_name)
{
    assert (data_source_);

    return data_source_->getSet()->getExistingInDBFor(dbo_name);
}


void ListBoxView::selectionChanged()
{
    //  assert (data_source_);
    //  data_source_->updateSelection();
}
void ListBoxView::selectionToBeCleared()
{

}

bool ListBoxView::usePresentation() const
{
    return use_presentation_;
}

void ListBoxView::usePresentation(bool use_presentation)
{
    use_presentation_ = use_presentation;

    emit usePresentationSignal(use_presentation_);
}

bool ListBoxView::overwriteCSV() const
{
    return overwrite_csv_;
}

void ListBoxView::overwriteCSV(bool overwrite_csv)
{
    overwrite_csv_ = overwrite_csv;
}

void ListBoxView::updateSelection ()
{
    loginf << "ListBoxView: updateSelection";
}

