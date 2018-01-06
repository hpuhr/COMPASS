#include "dbolabeldefinition.h"
#include "dbolabeldefinitionwidget.h"
//#include "ConfigurationManager.h"
#include "dbobject.h"
#include "dbovariable.h"

#include <iostream>
#include <string>


DBOLabelEntry::DBOLabelEntry(const std::string& class_id, const std::string& instance_id, DBOLabelDefinition* parent)
    : Configurable (class_id, instance_id, parent)
{
    //    registerParameter("index", &index, 0);
    registerParameter("variable_name", &variable_name_, "");
    //registerParameter("dbo_type_int", &dbo_type_int_, 0);
    registerParameter("show", &show_, false);
    registerParameter("prefix", &prefix_, "");
    registerParameter("suffix", &suffix_, "");

    createSubConfigurables ();

    //  if (!DBObjectManager::getInstance().existsDBOVariable((DB_OBJECT_TYPE) dbo_type_int, variable_id))
    //      throw std::runtime_error ("LabelEntry: constructor: dbo variable '"+variable_id+"' does not exist");

    //  variable = DBObjectManager::getInstance().getDBOVariable((DB_OBJECT_TYPE) dbo_type_int, variable_id);
}

DBOLabelEntry::~DBOLabelEntry()
{

}

std::string DBOLabelEntry::variableName() const
{
    return variable_name_;
}

void DBOLabelEntry::variableName(const std::string& variable_name)
{
    variable_name_ = variable_name;
}

bool DBOLabelEntry::show() const
{
    return show_;
}

void DBOLabelEntry::show(bool show)
{
    show_ = show;
}

std::string DBOLabelEntry::prefix() const
{
    return prefix_;
}

void DBOLabelEntry::prefix(const std::string& prefix)
{
    prefix_ = prefix;
}

std::string DBOLabelEntry::suffix() const
{
    return suffix_;
}

void DBOLabelEntry::suffix(const std::string& suffix)
{
    suffix_ = suffix;
}

DBOLabelDefinition::DBOLabelDefinition(const std::string& class_id, const std::string& instance_id, DBObject* parent)
    : Configurable (class_id, instance_id, parent), db_object_(parent)
{
    createSubConfigurables ();
}

DBOLabelDefinition::~DBOLabelDefinition()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    for (auto it : entries_)
    {
        delete it.second;
    }
    entries_.clear();

    read_list_.clear();
}

void DBOLabelDefinition::updateReadList ()
{
    read_list_.clear();

    //  for (auto it : entries_)
    //  {
    //    if (it.second->show)
    //    {
    //      read_list_.add(it.second->variable);
    //    }
    //  }
}

void DBOLabelDefinition::checkLabelDefintions()
{
    //TODO check if up to date
}

//void DBOLabelDefinition::print ()
//{
//  std::map <std::string, LabelEntry*>::iterator it;

//  std::vector <std::string> present_ids;

//  for (it=entries_.begin(); it != entries_.end(); it++)
//  {
//    std::cout << it->second->variable->id_ << std::endl;
//  }
//}

void DBOLabelDefinition::generateSubConfigurable (const std::string& class_id, const std::string& instance_id)
{
    if (class_id == "LabelEntry")
    {
        logdbg  << "DBOLabelDefinition: generateSubConfigurable: instance_id " << instance_id;

        DBOLabelEntry* entry = new DBOLabelEntry (class_id, instance_id, this);

//        if (!db_object_->hasVariable(entry->variableName()))
//        {
//            logwrn << "DBOLabelDefinition: generateSubConfigurable: outdated label definition for variable " << entry->variableName();
//            delete entry;
//            return;
//        }

        assert (entries_.find(entry->variableName()) == entries_.end());
        entries_ [entry->variableName()] = entry;

        updateReadList();
    }
    else
        throw std::runtime_error ("DBOLabelDefinition: generateSubConfigurable: unknown class_id "+class_id );
}

void DBOLabelDefinition::checkSubConfigurables ()
{
    logdbg  << "DBOLabelDefinition: checkSubConfigurables: object " << db_object_->name();

    const std::map<std::string, DBOVariable*>& variables = db_object_->variables();

    std::string variable_name;
    bool show=false;
    std::string prefix;
    std::string suffix;

    for (auto it : variables)
    {
        if (entries_.find (it.second->name()) == entries_.end())
        {
            variable_name = it.second->name();
            show=false;
            prefix="";
            suffix="";

            logdbg  << "DBOLabelDefinition: checkSubConfigurables: new var " << variable_name;

            if (variable_name == "tod")
            {
                show=true;
                prefix="T ";
            }
            else if (variable_name == "mode3a_code")
            {
                show=true;
                prefix="A";
            }
            else if (variable_name == "modec_code_ft")
            {
                show=true;
                prefix="A";
            }

            std::string instance_id = "LabelEntry"+variable_name+"0";

            Configuration &configuration = addNewSubConfiguration ("LabelEntry", instance_id);
            configuration.addParameterString("variable_name", variable_name);
            //configuration.addParameterUnsignedInt("dbo_type_int", dbo_type_int_);
            configuration.addParameterBool("show", show);
            configuration.addParameterString("prefix", prefix);
            configuration.addParameterString("suffix", suffix);
            generateSubConfigurable ("LabelEntry", instance_id);
        }
    }
}
DBOLabelEntry& DBOLabelDefinition::entry (const std::string& variable_name)
{
    return *entries_.at(variable_name);
}

DBOLabelDefinitionWidget* DBOLabelDefinition::widget ()
{
    if (!widget_)
    {
        widget_ = new DBOLabelDefinitionWidget (this);
        assert (widget_);
    }
    return widget_;
}
