#ifndef METADBOVARIABLE_H
#define METADBOVARIABLE_H

#include "configurable.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"

class MetaDBOVariable: public Configurable
{
public:
    MetaDBOVariable(const std::string &class_id, const std::string &instance_id, DBObjectManager *object_manager);
    virtual ~MetaDBOVariable ();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    //  /// @brief Return if variable exist in DBO of type
    //  bool existsIn (const std::string &dbo_type);
    //  /// @brief Returns variable existing in DBO of type
    //  DBOVariable *getFor (const std::string &dbo_type);
    //  /// @brief Returns first available variable
    //  DBOVariable *getFirst ();
    //  /// @brief Return variable identifier in DBO of type
    //  std::string getNameFor (const std::string &dbo_type);

    //  /// @brief Sets sub-variable name for DBO of type
    //  void setSubVariable (const std::string &type, std::string name);


protected:
    DBObjectManager &object_manager_;
    std::map <std::string, DBOVariableDefinition*> definitions_;

    /// Flag indicating if this meta-variable has registered itself to its sub-variables
    //bool registered_as_parent_;

    virtual void checkSubConfigurables ();

    /// @brief Registers a parent variable
  //  void registerParentVariable (DBOVariable *parent);
  //  /// @brief Unregisters a parent variable
  //  void unregisterParentVariable (DBOVariable *parent);


};

#endif // METADBOVARIABLE_H
