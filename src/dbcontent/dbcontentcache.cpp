#include "dbcontentcache.h"
#include "dbcontentmanager.h"
#include "dbcontent/variable/metavariable.h"

namespace dbContent {

Cache::Cache(DBContentManager& dbcont_man)
    : dbcont_man_(dbcont_man)
{

}

bool Cache::add(std::map<std::string, std::shared_ptr<Buffer>> buffers)
{
    bool something_changed = false;

    for (auto& buf_it : buffers)
    {
        if (!buf_it.second->size()) // empty buffer
            continue;

        if (buffers_.count(buf_it.first))
        {
            loginf << "Cache: add: adding buffer dbo " << buf_it.first
                   << " adding size " << buf_it.second->size() << " current size " << buffers_.at(buf_it.first)->size();

            buffers_.at(buf_it.first)->seizeBuffer(*buf_it.second.get());

            loginf << "Cache: add: new buffer dbo " << buf_it.first
                   << " size " << buffers_.at(buf_it.first)->size();
        }
        else
        {
            buffers_[buf_it.first] = move(buf_it.second);

            loginf << "Cache: add: created buffer dbo " << buf_it.first
                   << " size " << buffers_.at(buf_it.first)->size();
        }
        something_changed = true;
    }

    if (something_changed)
        updateMetaVarLookup();

    return something_changed;
}

void Cache::clear()
{
    buffers_.clear();
}

bool Cache::has(const std::string& dbcontent_name)
{
    return buffers_.count(dbcontent_name);
}

std::shared_ptr<Buffer> Cache::get(const std::string& dbcontent_name)
{
    assert (has(dbcontent_name));
    return buffers_.at(dbcontent_name);
}

void Cache::updateMetaVarLookup()
{
    meta_var_lookup_.clear();

    for (const auto& meta_var_it : dbcont_man_.metaVariables())
    {
        for (auto& buf_it : buffers_)
        {
            if (meta_var_it.second->existsIn(buf_it.first) // // meta exists in buf dbcont name
                    && buf_it.second->hasAnyPropertyNamed(meta_var_it.second->getNameFor(buf_it.first)))
            {
                // meta var -> var exists in buffer
                meta_var_lookup_[buf_it.first][meta_var_it.first] = meta_var_it.second->getNameFor(buf_it.first);
            }
        }
    }
}

} // namespace dbContent
