#ifndef DBOEDITDATASOURCEACTION_H
#define DBOEDITDATASOURCEACTION_H

#include <string>

class DBObject;

class DBOEditDataSourceAction
{
public:
    DBOEditDataSourceAction (const std::string& action="None", const std::string& target_type="",
                             const std::string& target_id="");

    const std::string& getActionString ()
    {
        return action_str_;
    }

    void perform (DBObject& object, const std::string& source_type, const std::string& source_id);

private:
    std::string action_; // "None", "Add", "Overwrite"
    std::string target_type_; // "Config", "DB"
    std::string target_id_; // "New" ("Add"),id ("Overwrite")

    std::string action_str_;

    void refreshActionString ();

};

#endif // DBOEDITDATASOURCEACTION_H
