#include "dboeditdatasourceaction.h"
#include "dbobject.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;

DBOEditDataSourceAction::DBOEditDataSourceAction(const std::string& action, const std::string& target_type,
                                                 const std::string& target_id)
    : action_(action), target_type_(target_type), target_id_(target_id)
{
//    std::string action_; // "None", "Add", "Overwrite"
    assert (action_ == "None" || action_ == "Add" || action_ == "Overwrite");
//    std::string target_type_; // "Config", "DB"
    assert (target_type_ == "" || target_type_ == "Config" || target_type_ == "DB");
//    std::string target_id_; // "New" ("Add"),id ("Overwrite")
    assert (target_id_ == "" || target_id_ == "New" || String::isNumber(target_id_));

    if (action_ == "None")
        assert (target_type_ == "" && target_id_ == "");
    if (action_ == "Add")
        assert (target_type_ != "" && target_id_ == "New");
    if (action == "Overwrite")
        assert (target_type_ != "" && String::isNumber(target_id_));


    refreshActionString();
}

void DBOEditDataSourceAction::perform (DBObject& object, const std::string& source_type, const std::string& source_id)
{
    loginf << "DBOEditDataSourceAction: perform: object src " << source_type << ", " << source_id
           << " action " << action_ << " tgt " << target_type_ << ", " << target_id_;

    assert (action_ != "None");

    if (action_ == "Add")
    {
        if (source_type == "DB" && target_type_ == "Config")
        {
            assert (String::isNumber(source_id));
            unsigned int id = std::stoi(source_id);
            assert (object.hasDataSource(id));
            object.addNewStoredDataSource() = object.getDataSource(id);
        }
    }
}

void DBOEditDataSourceAction::refreshActionString ()
{
    action_str_ = action_;

    if (action_ != "None")
    {
        if (action_ == "Add")
        {
            action_str_ += " to "+target_type_+" with new id";
        }
        else if ("Overwrite")
        {
            action_str_ += " in "+target_type_+" with id "+target_id_;
        }
    }
}
