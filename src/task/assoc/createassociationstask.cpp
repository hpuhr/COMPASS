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

#include "createassociationstask.h"

#include "compass.h"
#include "createassociationstaskwidget.h"
#include "createassociationsjob.h"
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
#include "buffer.h"

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
           << COMPASS::instance().interface().ready();

    if (!COMPASS::instance().interface().ready())
        return false;

    logdbg << "CreateAssociationsTask: checkPrerequisites: done "
           << COMPASS::instance().interface().hasProperty(DONE_PROPERTY_NAME);

    if (COMPASS::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = COMPASS::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    if (!canRun())
        return false;

    // check if was post-processed
    logdbg << "CreateAssociationsTask: checkPrerequisites: post "
           << COMPASS::instance().interface().hasProperty(PostProcessTask::DONE_PROPERTY_NAME);

    if (!COMPASS::instance().interface().hasProperty(PostProcessTask::DONE_PROPERTY_NAME))
        return false;

    logdbg << "CreateAssociationsTask: checkPrerequisites: post2 "
           << COMPASS::instance().interface().hasProperty(PostProcessTask::DONE_PROPERTY_NAME);

    if (COMPASS::instance().interface().getProperty(PostProcessTask::DONE_PROPERTY_NAME) != "1")
        return false;

    // check if hash var exists in all data
    DBObjectManager& object_man = COMPASS::instance().objectManager();

    logdbg << "CreateAssociationsTask: checkPrerequisites: tracker hashes";
    assert (object_man.existsObject("Tracker"));

    if (!object_man.hasData())
        return false;

    logdbg << "CreateAssociationsTask: checkPrerequisites: ok";
    return true;
}

bool CreateAssociationsTask::isRecommended()
{
//    if (!checkPrerequisites())
//        return false;

//    return !done_;

    return false;
}

bool CreateAssociationsTask::canRun()
{
    DBObjectManager& object_man = COMPASS::instance().objectManager();

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

    loginf << "CreateAssociationsTask: run: started";

    task_manager_.appendInfo("CreateAssociationsTask: started");

    start_time_ = boost::posix_time::microsec_clock::local_time();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    assert(!status_dialog_);
    status_dialog_.reset(new CreateAssociationsStatusDialog(*this));
    connect(status_dialog_.get(), &CreateAssociationsStatusDialog::closeSignal, this,
            &CreateAssociationsTask::closeStatusDialogSlot);
    status_dialog_->markStartTime();

    checkAndSetMetaVariable(key_var_str_, &key_var_);
    checkAndSetMetaVariable(tod_var_str_, &tod_var_);
    checkAndSetMetaVariable(target_addr_var_str_, &target_addr_var_);

    DBObjectManager& object_man = COMPASS::instance().objectManager();

    for (auto& dbo_it : object_man)
    {
        if (!dbo_it.second->hasData())
            continue;

        DBOVariableSet read_set = getReadSetFor(dbo_it.first);
        connect(dbo_it.second, &DBObject::newDataSignal, this,
                &CreateAssociationsTask::newDataSlot);
        connect(dbo_it.second, &DBObject::loadingDoneSignal, this,
                &CreateAssociationsTask::loadingDoneSlot);

        dbo_it.second->load(read_set, false, false, nullptr, false);

        dbo_loading_done_flags_[dbo_it.first] = false;
    }

    status_dialog_->setDBODoneFlags(dbo_loading_done_flags_);
    status_dialog_->show();
}

void CreateAssociationsTask::newDataSlot(DBObject& object)
{
}

void CreateAssociationsTask::loadingDoneSlot(DBObject& object)
{
    loginf << "CreateAssociationsTask: loadingDoneSlot: object " << object.name();

    disconnect(&object, &DBObject::newDataSignal, this, &CreateAssociationsTask::newDataSlot);
    disconnect(&object, &DBObject::loadingDoneSignal, this,
               &CreateAssociationsTask::loadingDoneSlot);

    dbo_loading_done_flags_.at(object.name()) = true;

    assert(status_dialog_);
    status_dialog_->setDBODoneFlags(dbo_loading_done_flags_);

    dbo_loading_done_ = true;

    for (auto& done_it : dbo_loading_done_flags_)
        if (!done_it.second)
            dbo_loading_done_ = false;

    if (dbo_loading_done_)
    {
        task_manager_.appendInfo("CreateAssociationsTask: data loading done");
        loginf << "CreateAssociationsTask: loadingDoneSlot: data loading done";

        //assert(!create_job_);

        std::map<std::string, std::shared_ptr<Buffer>> buffers;

        DBObjectManager& object_man = COMPASS::instance().objectManager();

        for (auto& dbo_it : object_man)
        {
            if (!dbo_it.second->hasData())
                continue;

            buffers[dbo_it.first] = dbo_it.second->data();

            loginf << "CreateAssociationsTask: loadingDoneSlot: object " << object.name()
                   << " data " << buffers[dbo_it.first]->size();

            dbo_it.second->clearData();
        }

        create_job_ = std::make_shared<CreateAssociationsJob>(
            *this, COMPASS::instance().interface(), buffers);

        connect(create_job_.get(), &CreateAssociationsJob::doneSignal, this,
                &CreateAssociationsTask::createDoneSlot, Qt::QueuedConnection);
        connect(create_job_.get(), &CreateAssociationsJob::obsoleteSignal, this,
                &CreateAssociationsTask::createObsoleteSlot, Qt::QueuedConnection);
        connect(create_job_.get(), &CreateAssociationsJob::statusSignal, this,
                &CreateAssociationsTask::associationStatusSlot, Qt::QueuedConnection);
//        connect(create_job_.get(), &CreateAssociationsJob::saveAssociationsQuestionSignal,
//                this, &CreateAssociationsTask::saveAssociationsQuestionSlot,
//                Qt::QueuedConnection);

        JobManager::instance().addDBJob(create_job_);

        status_dialog_->setAssociationStatus("In Progress");
    }
}

void CreateAssociationsTask::createDoneSlot()
{
    loginf << "CreateAssociationsTask: createDoneSlot";

    create_job_done_ = true;

    status_dialog_->setAssociationStatus("Done");
//    status_dialog_->setFoundHashes(create_job_->foundHashes());
//    status_dialog_->setMissingHashesAtBeginning(create_job_->missingHashesAtBeginning());
//    status_dialog_->setMissingHashes(create_job_->missingHashes());
//    status_dialog_->setDubiousAssociations(create_job_->dubiousAssociations());
//    status_dialog_->setFoundDuplicates(create_job_->foundHashDuplicates());

    status_dialog_->setDone();

    if (!show_done_summary_)
        status_dialog_->close();

    create_job_ = nullptr;

    stop_time_ = boost::posix_time::microsec_clock::local_time();

    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    std::string time_str = String::timeStringFromDouble(diff.total_milliseconds() / 1000.0, false);

    COMPASS::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");

    task_manager_.appendSuccess("CreateAssociationsTask: done after " + time_str);
    done_ = true;

    QApplication::restoreOverrideCursor();

    emit doneSignal(name_);
}

void CreateAssociationsTask::createObsoleteSlot()
{
    create_job_ = nullptr;
}

void CreateAssociationsTask::associationStatusSlot(QString status)
{
    assert(status_dialog_);
    status_dialog_->setAssociationStatus(status.toStdString());
}

void CreateAssociationsTask::closeStatusDialogSlot()
{
    assert(status_dialog_);
    status_dialog_->close();
    status_dialog_ = nullptr;
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
    DBObjectManager& object_man = COMPASS::instance().objectManager();

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
