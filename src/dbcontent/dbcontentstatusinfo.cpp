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

#include "dbcontentstatusinfo.h"

#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/target/targetreportaccessor.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableset.h"

namespace dbContent
{
DBContentStatusInfo::DBContentStatusInfo() {}

dbContent::VariableSet DBContentStatusInfo::getReadSetFor(const std::string& dbcontent_name) const
{
    dbContent::VariableSet read_set;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    auto& dbcont = dbcont_man.dbContent(dbcontent_name);
    assert (dbcont.containsStatusContent());

    // ds id
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ds_id_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ds_id_));

    // line id
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_line_id_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_));

    // timestamp
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));

    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_message_type_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_message_type_));

    return read_set;
}
void DBContentStatusInfo::process(std::map<std::string, std::shared_ptr<Buffer>> buffers)
{
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    for (auto& buf_it : buffers)
    {
        const std::string dbcontent_name = buf_it.first;
        auto& dbcontent = dbcont_man.dbContent(dbcontent_name);

        if (!dbcontent.containsStatusContent())
            continue;

        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ds_id_));
        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_line_id_));
        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));
        assert (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_message_type_));

        Variable& ds_id_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ds_id_);
        Variable& line_id_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_);
        Variable& timestamp_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_);
        Variable& message_type_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_message_type_);

        NullableVector<unsigned int>& ds_id_vec = buf_it.second->get<unsigned int>(ds_id_var.name());
        NullableVector<unsigned int>& line_id_vec = buf_it.second->get<unsigned int>(line_id_var.name());
        NullableVector<boost::posix_time::ptime>& timestamp_vec = buf_it.second->get<boost::posix_time::ptime>(timestamp_var.name());
        NullableVector<unsigned char>& message_type_vec = buf_it.second->get<unsigned char>(line_id_var.name());

        assert (ds_id_vec.isNeverNull());
        assert (line_id_vec.isNeverNull());

        unsigned int buffer_size = buf_it.second->size();

        unsigned char message_type;

        for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
            {
                

                if (!message_type_vec.isNull(cnt))
                {
                    message_type = message_type_vec.get(cnt);

                    // cat002: 001 North marker message;002 Sector crossing message;
                    // cat010: 002 Start of Update Cycle
                    // cat019: 001 Start of Update Cycle
                    // cat023: ?
                    // cat034: 001 North marker message;002 Sector crossing message;
                    // cat065: 002 End of Batch

                    
                }
            }
    }
}

bool DBContentStatusInfo::hasInfo(unsigned int ds_id, unsigned int line_id) const {}

std::vector<boost::posix_time::ptime> DBContentStatusInfo::getInfo(unsigned int ds_id,
                                                                   unsigned int line_id) const
{
}

}  // namespace dbContent