#ifndef DBOLABELDEFINITION_H
#define DBOLABELDEFINITION_H

#include <QObject>
#include <list>
#include <memory>

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
    DBOLabelDefinition* def_parent_ {nullptr};
    std::string variable_name_;

    bool show_; // show in label
    std::string prefix_;
    std::string suffix_;

    virtual void checkSubConfigurables () {}
};

class DBOLabelDefinitionWidget;
class Buffer;

class DBOLabelDefinition : public QObject, public Configurable
{
    Q_OBJECT

public slots:
    void labelDefinitionChangedSlot ();

public:
    DBOLabelDefinition(const std::string& class_id, const std::string& instance_id, DBObject* parent);
    virtual ~DBOLabelDefinition();

    DBOVariableSet &readList ();
    const std::map<std::string, DBOLabelEntry*>& entries () { return entries_; }
    DBOLabelEntry& entry (const std::string& variable_name);

    void updateReadList ();
    void checkLabelDefintions();

    virtual void generateSubConfigurable (const std::string& class_id, const std::string& instance_id);

    DBOLabelDefinitionWidget* widget ();

    std::map<int, std::string> generateLabels (std::vector<int> rec_nums, std::shared_ptr<Buffer> buffer,
                                               int break_item_cnt);

protected:
    DBObject* db_object_ {nullptr};
    std::map<std::string, DBOLabelEntry*> entries_; //varname -> labelentry

    DBOVariableSet read_list_;

    DBOLabelDefinitionWidget* widget_ {nullptr};

    virtual void checkSubConfigurables ();
};


#endif // DBOLABELDEFINITION_H
