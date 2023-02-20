#ifndef DBCONTENT_TARGET_H
#define DBCONTENT_TARGET_H

#include "json.hpp"

#include <set>

namespace dbContent {

class Target
{
public:
    const unsigned int utn_ {0};

    Target(unsigned int utn, nlohmann::json info);

    unsigned int utn() { return utn_; }

    bool use() const;
    void use(bool value);

    std::string comment() const;
    void comment (const std::string& value);

    nlohmann::json info() const { return info_; }

    std::set<unsigned int> tas();
    void tas(const std::set<unsigned int>& tas);

    std::set<unsigned int> mas();
    void mas(const std::set<unsigned int>& mas);

    unsigned int dbContentCount(const std::string& dbcontent_name);
    void dbContentCount(const std::string& dbcontent_name, unsigned int value);

    bool hasAdsbMOPSVersion();
    unsigned int adsbMOPSVersion();
    void adsbMOPSVersion(unsigned int value);

protected:
    nlohmann::json info_;
};

}
#endif // DBCONTENT_TARGET_H
