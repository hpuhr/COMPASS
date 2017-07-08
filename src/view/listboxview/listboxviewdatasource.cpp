/*
 * ListBoxViewDataSource.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#include "configuration.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "atsdb.h"
#include "listboxviewdatasource.h"
#include "logger.h"
#include "job.h"


#include <QMessageBox>

ListBoxViewDataSource::ListBoxViewDataSource(const std::string &class_id, const std::string &instance_id, Configurable *parent)
: QObject(), Configurable (class_id, instance_id, parent), set_(nullptr), selection_entries_ (ViewSelection::getInstance().getEntries())
{
    registerParameter ("use_filters", &use_filters_, false);
    registerParameter ("use_selection", &use_selection_, true);
    registerParameter ("use_order", &use_order_, false);
    registerParameter ("order_variable_type_int", &order_variable_type_int_, 0);
    registerParameter ("order_variable_name", &order_variable_name_, "frame_time");
    registerParameter ("order_ascending", &order_ascending_, true);
    registerParameter ("database_view", &database_view_, false);

    connect (&ATSDB::instance().objectManager(), SIGNAL(loadingStartedSignal()), this, SLOT(loadingStartedSlot()));

    for (auto object : ATSDB::instance().objectManager().objects())
    {
        connect (object.second, SIGNAL (newDataSignal(DBObject &)), this, SLOT(newDataSlot(DBObject&)));
        connect (object.second, SIGNAL (loadingDoneSignal(DBObject &)), this, SLOT(loadingDoneSlot(DBObject&)));
    }

    use_filters_=false;

    createSubConfigurables ();
}

ListBoxViewDataSource::~ListBoxViewDataSource()
{
    if (set_)
    {
        delete set_;
        set_=0;
    }
}

void ListBoxViewDataSource::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "ListBoxViewDataSource: generateSubConfigurable: class_id " << class_id << " instance_id " << instance_id;
    if (class_id.compare("DBOVariableOrderedSet") == 0)
    {
        assert (set_ == 0);
        set_ = new DBOVariableOrderedSet (class_id, instance_id, this);
    }
    else
        throw std::runtime_error ("ListBoxViewDataSource: generateSubConfigurable: unknown class_id "+class_id );
}

void ListBoxViewDataSource::checkSubConfigurables ()
{
    if (set_ == nullptr)
        generateSubConfigurable ("DBOVariableOrderedSet", "DBOVariableOrderedSet0");

    assert (set_);
}

void ListBoxViewDataSource::loadingStartedSlot ()
{
    logdbg << "ListBoxViewDataSource: loadingStartedSlot";
    emit loadingStartedSignal ();
}

void ListBoxViewDataSource::newDataSlot (DBObject &object)
{
    logdbg << "ListBoxViewDataSource: newDataSlot: object " << object.name();

    std::shared_ptr <Buffer> buffer = object.data();
    assert (buffer);

    emit updateData (object, buffer);
}

void ListBoxViewDataSource::loadingDoneSlot(DBObject &object)
{
    logdbg << "ListBoxViewDataSource: loadingDoneSlot: object " << object.name();
}

