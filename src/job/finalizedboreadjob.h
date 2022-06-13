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

#ifndef FINALIZEDBOREADJOB_H_
#define FINALIZEDBOREADJOB_H_

#include "dbcontent/variable/variableset.h"
#include "global.h"
#include "job.h"

class DBContent;
class Buffer;

/**
 * @brief Finalizes read DBContent data from DBOReadDBJob
 *
 * Uses finalizeDBData() from Util.
 */
class FinalizeDBOReadJob : public Job
{
  public:
    FinalizeDBOReadJob(DBContent& dbobject, dbContent::VariableSet& read_list,
                       std::shared_ptr<Buffer> buffer);
    virtual ~FinalizeDBOReadJob();

    virtual void run();

    std::shared_ptr<Buffer> buffer()
    {
        assert(buffer_);
        return buffer_;
    }

  protected:
    DBContent& dbobject_;
    dbContent::VariableSet read_list_;
    std::shared_ptr<Buffer> buffer_;
};

#endif /* FINALIZEDBOREADJOB_H_ */
