/*
 * OSGViewDataSource.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#include "configuration.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "atsdb.h"
#include "osgviewdatasource.h"
#include "logger.h"
#include "job.h"


#include <QMessageBox>

OSGViewDataSource::OSGViewDataSource(const std::string &class_id, const std::string &instance_id, Configurable *parent)
: QObject(), Configurable (class_id, instance_id, parent), set_(nullptr), selection_entries_ (ViewSelection::getInstance().getEntries())
{
    connect (&ATSDB::instance().objectManager(), SIGNAL(loadingStartedSignal()), this, SLOT(loadingStartedSlot()));

    for (auto object : ATSDB::instance().objectManager().objects())
    {
        connect (object.second, SIGNAL (newDataSignal(DBObject &)), this, SLOT(newDataSlot(DBObject&)));
        connect (object.second, SIGNAL (loadingDoneSignal(DBObject &)), this, SLOT(loadingDoneSlot(DBObject&)));
    }

    createSubConfigurables ();
}

OSGViewDataSource::~OSGViewDataSource()
{
    if (set_)
    {
        delete set_;
        set_=0;
    }
}

void OSGViewDataSource::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "OSGViewDataSource: generateSubConfigurable: class_id " << class_id << " instance_id " << instance_id;
    if (class_id.compare("DBOVariableOrderedSet") == 0)
    {
        assert (set_ == 0);
        set_ = new DBOVariableOrderedSet (class_id, instance_id, this);
    }
    else
        throw std::runtime_error ("OSGViewDataSource: generateSubConfigurable: unknown class_id "+class_id );
}

void OSGViewDataSource::checkSubConfigurables ()
{
    if (set_ == nullptr)
        generateSubConfigurable ("DBOVariableOrderedSet", "DBOVariableOrderedSet0");

    assert (set_);
}

void OSGViewDataSource::loadingStartedSlot ()
{
    logdbg << "OSGViewDataSource: loadingStartedSlot";
    emit loadingStartedSignal ();
}

void OSGViewDataSource::newDataSlot (DBObject &object)
{
    std::shared_ptr <Buffer> buffer = object.data();
    assert (buffer);

    logdbg << "OSGViewDataSource: newDataSlot: object " << object.name() << " buffer size " << buffer->size();

    emit updateData (object, buffer);
}

void OSGViewDataSource::loadingDoneSlot(DBObject &object)
{
    logdbg << "OSGViewDataSource: loadingDoneSlot: object " << object.name();
}

