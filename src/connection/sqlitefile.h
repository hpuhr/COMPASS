#ifndef SQLITE_FILE_H
#define SQLITE_FILE_H

#include "configurable.h"

class SQLiteFile : public Configurable
{
public:
    SQLiteFile(const std::string class_id, const std::string &instance_id, Configurable *parent)
        : Configurable (class_id, instance_id, parent)
    {
        registerParameter ("name", &name_, "");
    }
    virtual ~SQLiteFile () {}

    /// Returns the database server name or IP address
    const std::string &name () const { return name_; }
    void name (const std::string &name) { name_ = name; }

protected:
    std::string name_;
};

#endif // SQLITE_FILE_H
