#ifndef ASTERIXCATEGORYCONFIG_H
#define ASTERIXCATEGORYCONFIG_H

#include <string>

#include "configurable.h"

class ASTERIXCategoryConfig : public Configurable
{
public:
    ASTERIXCategoryConfig (const std::string& category, const std::string& class_id, const std::string& instance_id,
                           Configurable* parent)
        : Configurable (class_id, instance_id,parent)
    {
        registerParameter("category", &category_, "");
        registerParameter("decode", &decode_, false);
        registerParameter("edition", &edition_, "");
    }

    virtual void generateSubConfigurable (const std::string& class_id, const std::string& instance_id) {}

    std::string category() const
    {
        return category_;
    }

    bool decode() const
    {
        return decode_;
    }
    void decode(bool decode)
    {
        decode_ = decode;
    }


    std::string edition() const
    {
        return edition_;
    }

    void edition(const std::string &edition)
    {
        edition_ = edition;
    }

private:
    std::string category_;
    bool decode_ {false};
    std::string edition_;

protected:
    virtual void checkSubConfigurables () {}
};

#endif // ASTERIXCATEGORYCONFIG_H
