#include "createassociationstask.h"

#include "atsdb.h"
#include "createassociationstaskwidget.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbodatasource.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "jobmanager.h"
#include "metadbovariable.h"
#include "postprocesstask.h"
#include "stringconv.h"
#include "taskmanager.h"

#include <QApplication>
#include <QMessageBox>
#include <sstream>

using namespace std;
using namespace Utils;

const std::string CreateAssociationsTask::DONE_PROPERTY_NAME = "associations_created";

CreateAssociationsTask::CreateAssociationsTask(const std::string& class_id,
                                               const std::string& instance_id,
                                               TaskManager& task_manager)
    : Task("CreateAssociationsTask", "Associate Target Reports", true, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_calc_assoc.json")
{
    tooltip_ =
        "Allows creation of UTNs and target report association based on Mode S Addresses.";

    registerParameter("key_var_str", &key_var_str_, "rec_num");
    registerParameter("tod_var_str", &tod_var_str_, "tod");
    registerParameter("target_addr_var_str", &target_addr_var_str_, "target_addr");
}

CreateAssociationsTask::~CreateAssociationsTask() {}


TaskWidget* CreateAssociationsTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new CreateAssociationsTaskWidget(*this));

        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &CreateAssociationsTaskWidget::expertModeChangedSlot);
    }

    assert(widget_);
    return widget_.get();
}

void CreateAssociationsTask::deleteWidget() { widget_.reset(nullptr); }


bool CreateAssociationsTask::checkPrerequisites()
{
    logdbg << "CreateAssociationsTask: checkPrerequisites: ready "
           << ATSDB::instance().interface().ready();

    if (!ATSDB::instance().interface().ready())
        return false;

    logdbg << "CreateAssociationsTask: checkPrerequisites: done "
           << ATSDB::instance().interface().hasProperty(DONE_PROPERTY_NAME);

    if (ATSDB::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = ATSDB::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    if (!canRun())
        return false;

    // check if was post-processed
    logdbg << "CreateAssociationsTask: checkPrerequisites: post "
           << ATSDB::instance().interface().hasProperty(PostProcessTask::DONE_PROPERTY_NAME);

    if (!ATSDB::instance().interface().hasProperty(PostProcessTask::DONE_PROPERTY_NAME))
        return false;

    logdbg << "CreateAssociationsTask: checkPrerequisites: post2 "
           << ATSDB::instance().interface().hasProperty(PostProcessTask::DONE_PROPERTY_NAME);

    if (ATSDB::instance().interface().getProperty(PostProcessTask::DONE_PROPERTY_NAME) != "1")
        return false;

    // check if hash var exists in all data
    DBObjectManager& object_man = ATSDB::instance().objectManager();

    logdbg << "CreateAssociationsTask: checkPrerequisites: tracker hashes";
    assert (object_man.existsObject("Tracker"));

    if (!object_man.hasData())
        return false;

    logdbg << "CreateAssociationsTask: checkPrerequisites: ok";
    return true;
}

bool CreateAssociationsTask::isRecommended()
{
    if (!checkPrerequisites())
        return false;

    return !done_;
}

bool CreateAssociationsTask::canRun()
{
    DBObjectManager& object_man = ATSDB::instance().objectManager();

    // ATSDB::instance().interface().hasProperty(DONE_PROPERTY_NAME)

    logdbg << "CreateAssociationsTask: canRun: tracker " << object_man.existsObject("Tracker");

    // meta var stuff
    logdbg << "CreateAssociationsTask: canRun: meta vars";
    if (!key_var_str_.size() || !target_addr_var_str_.size() || !tod_var_str_.size())
        return false;

    logdbg << "CreateAssociationsTask: canRun: metas ";
    if (!object_man.existsMetaVariable(key_var_str_) ||
        !object_man.existsMetaVariable(target_addr_var_str_) ||
        !object_man.existsMetaVariable(tod_var_str_))
        return false;

    logdbg << "CreateAssociationsTask: canRun: metas in objects";
    for (auto& dbo_it : object_man)
    {
        if (!object_man.metaVariable(key_var_str_).existsIn(dbo_it.first) ||
            !object_man.metaVariable(target_addr_var_str_).existsIn(dbo_it.first) ||
            !object_man.metaVariable(tod_var_str_).existsIn(dbo_it.first))
            return false;
    }

    logdbg << "CreateAssociationsTask: canRun: ok";
    return true;
}

void CreateAssociationsTask::run()
{
    assert(canRun());
}

void CreateAssociationsTask::createDoneSlot()
{

}

void CreateAssociationsTask::createObsoleteSlot()
{
    //create_job_ = nullptr;
}

void CreateAssociationsTask::newDataSlot(DBObject& object)
{
    // updateProgressSlot();
}

void CreateAssociationsTask::loadingDoneSlot(DBObject& object)
{
    loginf << "CreateAssociationsTask: loadingDoneSlot: object " << object.name();

    disconnect(&object, &DBObject::newDataSignal, this, &CreateAssociationsTask::newDataSlot);
    disconnect(&object, &DBObject::loadingDoneSignal, this,
               &CreateAssociationsTask::loadingDoneSlot);

    dbo_loading_done_flags_.at(object.name()) = true;

//    assert(status_dialog_);
//    status_dialog_->setDBODoneFlags(dbo_loading_done_flags_);

    dbo_loading_done_ = true;

    for (auto& done_it : dbo_loading_done_flags_)
        if (!done_it.second)
            dbo_loading_done_ = false;
}

std::string CreateAssociationsTask::keyVarStr() const { return key_var_str_; }

void CreateAssociationsTask::keyVarStr(const std::string& var_str)
{
    loginf << "CreateAssociationsTask: keyVarStr: '" << var_str << "'";

    key_var_str_ = var_str;
}

std::string CreateAssociationsTask::targetAddrVarStr() const { return target_addr_var_str_; }

void CreateAssociationsTask::targetAddrVarStr(const std::string& var_str)
{
    loginf << "CreateAssociationsTask: targetAddrVarStr: '" << var_str << "'";

    target_addr_var_str_ = var_str;
}

std::string CreateAssociationsTask::todVarStr() const { return tod_var_str_; }

void CreateAssociationsTask::todVarStr(const std::string& var_str)
{
    loginf << "CreateAssociationsTask: todVarStr: '" << var_str << "'";

    tod_var_str_ = var_str;
}

MetaDBOVariable* CreateAssociationsTask::keyVar() const { return key_var_; }

MetaDBOVariable* CreateAssociationsTask::targetAddrVar() const { return target_addr_var_; }

MetaDBOVariable* CreateAssociationsTask::todVar() const { return tod_var_; }


void CreateAssociationsTask::checkAndSetMetaVariable(std::string& name_str,
                                                          MetaDBOVariable** var)
{
    DBObjectManager& object_man = ATSDB::instance().objectManager();

    if (!object_man.existsMetaVariable(name_str))
    {
        loginf << "CreateAssociationsTask: checkAndSetMetaVariable: var " << name_str
               << " does not exist";
        name_str = "";
        var = nullptr;
    }
    else
    {
        *var = &object_man.metaVariable(name_str);
        loginf << "CreateAssociationsTask: checkAndSetMetaVariable: var " << name_str
               << " set";
        assert(var);
    }
}

DBOVariableSet CreateAssociationsTask::getReadSetFor(const std::string& dbo_name)
{
    DBOVariableSet read_set;

    assert(key_var_);
    assert(key_var_->existsIn(dbo_name));
    read_set.add(key_var_->getFor(dbo_name));

    assert(tod_var_);
    assert(tod_var_->existsIn(dbo_name));
    read_set.add(tod_var_->getFor(dbo_name));

    assert(target_addr_var_);
    assert(target_addr_var_->existsIn(dbo_name));
    read_set.add(target_addr_var_->getFor(dbo_name));

    return read_set;
}
