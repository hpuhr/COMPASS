#include "jsonimportertask.h"
#include "jsonimportertaskwidget.h"
#include "taskmanager.h"
#include "atsdb.h"
#include "dbobject.h"
#include "dbobjectmanager.h"

#include <jsoncpp/json/json.h>

JSONImporterTask::JSONImporterTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager* task_manager)
: Configurable (class_id, instance_id, task_manager)
{
    registerParameter("db_object_str", &db_object_str_, "");
}

JSONImporterTask::~JSONImporterTask()
{

}

JSONImporterTaskWidget* JSONImporterTask::widget()
{
    if (!widget_)
    {
        widget_ = new JSONImporterTaskWidget (*this);
    }

    assert (widget_);
    return widget_;
}

std::string JSONImporterTask::dbObjectStr() const
{
    return db_object_str_;
}

void JSONImporterTask::dbObjectStr(const std::string& db_object_str)
{
    loginf << "JSONImporterTask: dbObjectStr: " << db_object_str;

    db_object_str_ = db_object_str;

    assert (ATSDB::instance().objectManager().existsObject(db_object_str_));
    db_object_ = &ATSDB::instance().objectManager().object(db_object_str_);
}
