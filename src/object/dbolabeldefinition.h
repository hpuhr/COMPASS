#ifndef DBOLABELDEFINITION_H
#define DBOLABELDEFINITION_H

#include <QObject>
#include <list>

#include "configurable.h"
#include "dbovariableset.h"
//#include "DBObjectManager.h"

class DBObject;
class DBOLabelDefinition;

class DBOLabelEntry : public QObject, public Configurable
{
    Q_OBJECT

public:
    DBOLabelEntry(const std::string& class_id, const std::string& instance_id, DBOLabelDefinition* parent);

    virtual ~DBOLabelEntry();

    std::string variableName() const;
    void variableName(const std::string& variable_name);

    bool show() const;
    void show(bool show);

    std::string prefix() const;
    void prefix(const std::string& prefix);

    std::string suffix() const;
    void suffix(const std::string& suffix);

protected:
    DBOLabelDefinition* parent_;
    //  unsigned int index;
    std::string variable_name_;
    //unsigned int dbo_type_int_;
    bool show_; // show in label
    std::string prefix_;
    std::string suffix_;

    //DBOVariable *variable_{nullptr};

    virtual void checkSubConfigurables () {}
};

class DBOLabelDefinitionWidget;

class DBOLabelDefinition : public QObject, public Configurable
{
    Q_OBJECT

signals:
  void changedLabelDefinitionSignal ();

public:
    DBOLabelDefinition(const std::string& class_id, const std::string& instance_id, DBObject* parent);
    virtual ~DBOLabelDefinition();

    DBOVariableSet &readList () { return read_list_; }
    const std::map<std::string, DBOLabelEntry*>& entries () { return entries_; }
    DBOLabelEntry& entry (const std::string& variable_name);

    void updateReadList ();
    void checkLabelDefintions();

    //void print ();

    virtual void generateSubConfigurable (const std::string& class_id, const std::string& instance_id);

    DBOLabelDefinitionWidget* widget ();

protected:
    DBObject* db_object_{nullptr};
    std::map<std::string, DBOLabelEntry*> entries_; //varname -> labelentry

    DBOVariableSet read_list_;

    DBOLabelDefinitionWidget* widget_ {nullptr};

    virtual void checkSubConfigurables ();
};


#endif // DBOLABELDEFINITION_H
