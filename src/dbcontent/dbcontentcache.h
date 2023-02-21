#ifndef DBCONTENT_CACHE_H
#define DBCONTENT_CACHE_H

#include "buffer.h"

class DBContentManager;

namespace dbContent {

class Cache
{
public:
    Cache(DBContentManager& dbcont_man);

    bool add(std::map<std::string, std::shared_ptr<Buffer>> buffers); // something changed flag
    void clear();

    bool has(const std::string& dbcontent_name);
    std::shared_ptr<Buffer> get(const std::string& dbcontent_name);

    template <typename T>
    bool hasVar(const std::string& dbcontent_name, const Property& var_property);
    template <typename T>
    NullableVector<T>& getVar(const std::string& dbcontent_name, const Property& var_property);

    template <typename T>
    bool hasMetaVar(const std::string& dbcontent_name, const Property& metavar_property);

    template <typename T>
    NullableVector<T>& getMetaVar(const std::string& dbcontent_name, const Property& metavar_property);

    using BufferIterator = typename std::map<std::string, std::shared_ptr<Buffer>>::iterator;
    BufferIterator begin() { return buffers_.begin(); }
    BufferIterator end() { return buffers_.end(); }

protected:
    DBContentManager& dbcont_man_;

    std::map<std::string, std::shared_ptr<Buffer>> buffers_; // dbcont name -> buffer
    std::map<std::string, std::map<std::string, std::string>> meta_var_lookup_; // dbcont name -> metvarname -> varname

    void updateMetaVarLookup();
};

template <typename T>
inline bool Cache::hasVar(const std::string& dbcontent_name, const Property& var_property)
{
    if (!buffers_.count(dbcontent_name))
        return false;

    return buffers_.at(dbcontent_name)->has<T>(var_property.name());
}

template <typename T>
inline NullableVector<T>& Cache::getVar(const std::string& dbcontent_name, const Property& var_property)
{
    assert (hasVar<T>(dbcontent_name, var_property));
    return buffers_.at(dbcontent_name)->get<T>(var_property.name());
}

template <typename T>
inline bool Cache::hasMetaVar(const std::string& dbcontent_name, const Property& metavar_property)
{
    return meta_var_lookup_.count(dbcontent_name)
            && meta_var_lookup_.at(dbcontent_name).count(metavar_property.name());
}

template <typename T>
inline NullableVector<T>& Cache::getMetaVar(const std::string& dbcontent_name, const Property& metavar_property)
{
    assert (hasMetaVar<T>(dbcontent_name, metavar_property));
    assert (buffers_.at(dbcontent_name)->has<T>(meta_var_lookup_.at(dbcontent_name).at(metavar_property.name())));
    return buffers_.at(dbcontent_name)->get<T>(meta_var_lookup_.at(dbcontent_name).at(metavar_property.name()));
}

} // namespace dbContent

#endif // DBCONTENT_CACHE_H
