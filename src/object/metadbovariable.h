#ifndef METADBOVARIABLE_H
#define METADBOVARIABLE_H

#include "configurable.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "stringconv.h"

class MetaDBOVariableWidget;

class MetaDBOVariable: public Configurable
{
public:
    MetaDBOVariable(const std::string &class_id, const std::string &instance_id, DBObjectManager *object_manager);
    virtual ~MetaDBOVariable ();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    bool hasVariables () { return variables_.size() > 0; }
    PropertyDataType dataType ();
    const std::string &dataTypeString();
    Utils::String::Representation representation ();

    /// @brief Return if variable exist in DBO of type
    bool existsIn (const std::string &dbo_name);
    /// @brief Returns variable existing in DBO of type
    DBOVariable &getFor (const std::string &dbo_name);
    /// @brief Return variable identifier in DBO of type
    std::string getNameFor (const std::string &dbo_name);

    void removeVariable (const std::string &dbo_name);
    /// @brief Sets sub-variable name for DBO of type
    void addVariable (const std::string &dbo_name, const std::string &dbovariable_name);

    const std::map <std::string, DBOVariable&> &variables () { return variables_; }

    std::string name() const;
    void name(const std::string &name);

    std::string description() const;
    void description(const std::string &description);

    MetaDBOVariableWidget *widget ();

protected:
    std::string name_;
    std::string description_;

    DBObjectManager &object_manager_;

    MetaDBOVariableWidget *widget_;

    std::map <std::string, DBOVariableDefinition*> definitions_;
    std::map <std::string, DBOVariable&> variables_;

    virtual void checkSubConfigurables ();

    /// @brief Registers a parent variable
  //  void registerParentVariable (DBOVariable *parent);
  //  /// @brief Unregisters a parent variable
  //  void unregisterParentVariable (DBOVariable *parent);


};

#endif // METADBOVARIABLE_H
