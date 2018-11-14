#ifndef DBOEDITDATASOURCEACTIONOPTIONS_H
#define DBOEDITDATASOURCEACTIONOPTIONS_H

#include <string>
#include <map>
#include <memory>

#include "dboeditdatasourceaction.h"

class DBObject;
class DBODataSource;
class StoredDBODataSource;
class DBOEditDataSourceActionOptionsWidget;

class DBOEditDataSourceActionOptions
{
public:
    DBOEditDataSourceActionOptions (DBObject& object, const std::string& sourceType, const std::string& sourceId);
    // none action is added by default with action_id 0
    DBOEditDataSourceActionOptions () = default;

    void addPossibleAction (const std::string& action, const std::string& target_type,
                            const std::string& target_id);

    using PossibleActionIterator = typename std::map<unsigned int, DBOEditDataSourceAction>::iterator;
    PossibleActionIterator begin() { return possible_actions_.begin(); }
    PossibleActionIterator end() { return possible_actions_.end(); }
    size_t numOptions () { return possible_actions_.size(); }

    bool performFlag() const;
    void performFlag(bool perform_flag);

    std::string sourceType() const;
    void sourceType(const std::string &source_type);

    std::string sourceId() const;
    void sourceId(const std::string &source_id);

    unsigned int currentActionId() const;
    void currentActionId(unsigned int current_action_id);

    DBOEditDataSourceActionOptionsWidget* widget ();

    void perform ();

private:
    DBObject* object_ {nullptr};

    bool perform_ {false};

    std::string source_type_; // "cfg", "db"
    std::string source_id_; // id

    unsigned int current_action_id_;

    std::map<unsigned int, DBOEditDataSourceAction> possible_actions_; // action_id -> action

    std::unique_ptr<DBOEditDataSourceActionOptionsWidget> widget_;
};

namespace DBOEditDataSourceActionOptionsCreator
{
    DBOEditDataSourceActionOptions getSyncOptionsFromDB (DBObject& object, DBODataSource& source);
    DBOEditDataSourceActionOptions getSyncOptionsFromCfg (DBObject& object, StoredDBODataSource& source);
}


#endif // DBOEDITDATASOURCEACTIONOPTIONS_H
