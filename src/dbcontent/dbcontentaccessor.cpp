/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbcontentaccessor.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "bufferaccessor.h"
#include "targetreportaccessor.h"

namespace dbContent 
{

/**
*/
DBContentAccessor::DBContentAccessor()
{
}

/**
*/
bool DBContentAccessor::add(std::map<std::string, std::shared_ptr<Buffer>> buffers)
{
    bool something_changed = false;

    for (auto& buf_it : buffers)
    {
        if (!buf_it.second->size()) // empty buffer
            continue;

        if (buffers_.count(buf_it.first))
        {
            loginf << "DBContentAccessor: add: adding buffer dbo " << buf_it.first
                   << " adding size " << buf_it.second->size() << " current size " << buffers_.at(buf_it.first)->size();

            buffers_.at(buf_it.first)->seizeBuffer(*buf_it.second.get());

            loginf << "DBContentAccessor: add: new buffer dbo " << buf_it.first
                   << " size " << buffers_.at(buf_it.first)->size();
        }
        else
        {
            buffers_[buf_it.first] = std::move(buf_it.second);

            loginf << "DBContentAccessor: add: created buffer dbo " << buf_it.first
                   << " size " << buffers_.at(buf_it.first)->size();
        }
        something_changed = true;
    }

    if (something_changed)
        updateDBContentLookup();

    return something_changed;
}

void DBContentAccessor::removeEmptyBuffers()
{
    bool something_changed = false;

    // remove empty buffers
    std::map<std::string, std::shared_ptr<Buffer>> tmp_data = buffers_;

    for (auto& buf_it : tmp_data)
        if (!buf_it.second->size())
        {
            buffers_.erase(buf_it.first);
            something_changed = true;
        }

    if (something_changed)
        updateDBContentLookup();
}

/**
*/
void DBContentAccessor::clear()
{
    buffers_.clear();
    dbcontent_lookup_.clear();
}

/**
*/
bool DBContentAccessor::has(const std::string& dbcontent_name) const
{
    return buffers_.count(dbcontent_name);
}

/**
*/
std::shared_ptr<Buffer> DBContentAccessor::get(const std::string& dbcontent_name)
{
    assert (has(dbcontent_name));
    return buffers_.at(dbcontent_name);
}

/**
*/
void DBContentAccessor::updateDBContentLookup()
{
    dbcontent_lookup_.clear();

    for (auto& buf_it : buffers_)
    {
        //generate lookup for buffer's dbcontent
        auto& lookup = dbcontent_lookup_[buf_it.first];
        lookup = std::shared_ptr<DBContentVariableLookup>(new DBContentVariableLookup(buf_it.first, buf_it.second));
        lookup->update(COMPASS::instance().dbContentManager());
    }
}

/**
*/
BufferAccessor DBContentAccessor::bufferAccessor(const std::string& dbcontent_name) const
{
    assert (has(dbcontent_name));
    return BufferAccessor(dbcontent_lookup_.at(dbcontent_name));
}

/**
*/
TargetReportAccessor DBContentAccessor::targetReportAccessor(const std::string& dbcontent_name) const
{
    assert (has(dbcontent_name));
    return TargetReportAccessor(dbcontent_lookup_.at(dbcontent_name));
}

/**
*/
void DBContentAccessor::print() const 
{
    for (const auto& elem : dbcontent_lookup_)
    {
        elem.second->print();
        std::cout << std::endl;
    }
}

} // namespace dbContent
