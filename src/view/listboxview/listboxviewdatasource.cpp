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
//#include "DBOInfoDBJob.h"

#include <QMessageBox>

ListBoxViewDataSource::ListBoxViewDataSource(const std::string &class_id, const std::string &instance_id, Configurable *parent)
: QObject(), Configurable (class_id, instance_id, parent), set_(0), selection_entries_ (ViewSelection::getInstance().getEntries())
{
    registerParameter ("use_filters", &use_filters_, false);
    registerParameter ("limit_min", &limit_min_, 0);
    registerParameter ("limit_max", &limit_max_, 100);
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
//        if (ATSDB::getInstance().contains(it->first) && it->second->isLoadable())
//            data_[it->first] = 0;
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
    if (set_ == 0)
    {
        //TODO
//        logdbg  << "ListBoxViewDataSource: checkSubConfigurables: generating variables";
//        DB_OBJECT_TYPE type = DBO_UNDEFINED;

//        Configuration &set_configuration = addNewSubConfiguration ("DBOVariableOrderedSet", "DBOVariableOrderedSet0");

//        std::string var_name = "id";
//        Configuration &id_configuration = set_configuration.addNewSubConfiguration ("DBOVariableOrderDefinition",
//                "DBOVariableOrderDefinition"+var_name+"0");
//        id_configuration.addParameterUnsignedInt ("dbo_type", type);
//        id_configuration.addParameterString ("id", var_name);
//        id_configuration.addParameterUnsignedInt ("index", 1);
//        //set_configuration.addSubConfigurable("DBOVariableOrderDefinition", "DBOVariableOrderDefinition"+var_name+"0", "DBOVariableOrderDefinition"+var_name+"0Config0");


//        var_name = "frame_time";
//        Configuration &ft_configuration = set_configuration.addNewSubConfiguration ("DBOVariableOrderDefinition",
//                "DBOVariableOrderDefinition"+var_name+"0");
//        ft_configuration.addParameterUnsignedInt ("dbo_type", type);
//        ft_configuration.addParameterString ("id", var_name);
//        ft_configuration.addParameterUnsignedInt ("index", 0);
//        //set_configuration.addSubConfigurable("DBOVariableOrderDefinition", "DBOVariableOrderDefinition"+var_name+"0", "DBOVariableOrderDefinition"+var_name+"0Config0");


//        var_name = "mode_3a_code";
//        Configuration &modea_configuration = set_configuration.addNewSubConfiguration ("DBOVariableOrderDefinition",
//                "DBOVariableOrderDefinition"+var_name+"0");
//        modea_configuration.addParameterUnsignedInt ("dbo_type", type);
//        modea_configuration.addParameterString ("id", var_name);
//        modea_configuration.addParameterUnsignedInt ("index", 2);
//        //set_configuration.addSubConfigurable("DBOVariableOrderDefinition", "DBOVariableOrderDefinition"+var_name+"0", "DBOVariableOrderDefinition"+var_name+"0Config0");

//        var_name = "mode_c_height";
//        Configuration &modec_configuration = set_configuration.addNewSubConfiguration ("DBOVariableOrderDefinition",
//                "DBOVariableOrderDefinition"+var_name+"0");
//        modec_configuration.addParameterUnsignedInt ("dbo_type", type);
//        modec_configuration.addParameterString ("id", var_name);
//        modec_configuration.addParameterUnsignedInt ("index", 3);
//        //set_configuration.addSubConfigurable("DBOVariableOrderDefinition", "DBOVariableOrderDefinition"+var_name+"0", "DBOVariableOrderDefinition"+var_name+"0Config0");

        generateSubConfigurable ("DBOVariableOrderedSet", "DBOVariableOrderedSet0");
    }
}

void ListBoxViewDataSource::updateData ()
{
    logdbg  << "ListBoxViewDataSource: updateData: start";

    //TODO
//    std::map <DB_OBJECT_TYPE, Buffer*>::iterator it;

//    for (it = data_.begin(); it != data_.end(); it++)
//    {
//        if (it->second)
//        {
//            delete it->second;
//            it->second=0;
//        }
//    }
//    //assert (limit_max_ > limit_min_);

//    std::string order_by_variable;

//    std::vector<unsigned int> ids;

//    for (it = data_.begin(); it != data_.end(); it++)
//    {
//        ids.clear();
//        if (use_selection_ && selection_entries_.size() > 0)
//        {
//            ViewSelectionEntries::iterator it2;

//            for (it2 = selection_entries_.begin(); it2 != selection_entries_.end(); it2++)
//            {
//                ViewSelectionEntry &entry=*it2;
//                if (entry.isDBO())
//                {
//                    if (entry.id_.first == it->first)
//                        ids.push_back(entry.id_.second);

//                    if (ids.size() > limit_max_-limit_min_)
//                        break;
//                }
//            }
//        }

//        //PropertyList list = set_->getPropertyList  (it->first);
//        DBOVariableSet read_list = set_->getUnorderedSet ();

//        assert (DBObjectManager::getInstance().existsDBOVariable (DBO_UNDEFINED, "id"));
//        DBOVariable *id = DBObjectManager::getInstance().getDBOVariable (DBO_UNDEFINED, "id");

//        if (!read_list.hasVariable(id))
//        {
//            read_list.add(id);
//        }

//        order_by_variable="";
//        if (use_order_)
//        {
//            assert (DBObjectManager::getInstance().existsDBOVariable((DB_OBJECT_TYPE)order_variable_type_int_, order_variable_name_));
//            DBOVariable *order_variable = DBObjectManager::getInstance().getDBOVariable((DB_OBJECT_TYPE)order_variable_type_int_, order_variable_name_);
//            if (order_variable->existsIn(it->first))
//                order_by_variable = order_variable->getFor(it->first)->getName( );
//        }

//        if (use_selection_ && ids.size() == 0)
//        {
//            data_ [it->first] = 0;
//            logdbg  << "ListBoxViewDataSource: updateData: data for type " << it->first << " is 0";
//        }
//        else
//        {
//            data_ [it->first] = 0;

//            ATSDB::getInstance().getInfo (this, boost::bind( &ListBoxViewDataSource::readInfoDone, this, _1),
//                    boost::bind( &ListBoxViewDataSource::readInfoObsolete, this, _1), it->first, ids, read_list, use_filters_,
//                    order_by_variable, order_ascending_, limit_min_, limit_max_, !database_view_);
//        }
//    }
    logdbg  << "ListBoxViewDataSource: updateData: end";
}

void ListBoxViewDataSource::loadingStartedSlot ()
{
    loginf << "ListBoxViewDataSource: loadingStartedSlot";
}

void ListBoxViewDataSource::newDataSlot (DBObject &object)
{
    logdbg << "ListBoxViewDataSource: newDataSlot: object " << object.name();
//    assert (job);

//    DBOInfoDBJob *infojob = (DBOInfoDBJob*) job;

//    DB_OBJECT_TYPE type = infojob->getType();
//    Buffer *buffer = infojob->getResultBuffer();

//    delete job;

//    assert (buffer);

//    emit updateData ((unsigned int) type, buffer);
}

void ListBoxViewDataSource::loadingDoneSlot(DBObject &object)
{
    loginf << "ListBoxViewDataSource: loadingDoneSlot: object " << object.name();
    //delete job;
}

