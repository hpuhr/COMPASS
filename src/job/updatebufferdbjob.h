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

#ifndef UpdateBufferDBJob_H_
#define UpdateBufferDBJob_H_

#include <list>

#include "job.h"

#include <cassert>

class Buffer;
class DBObject;
class DBInterface;
class DBOVariable;

/**
 * @brief Buffer write job
 *
 * Writes buffer's data contents to a database table.
 */
class UpdateBufferDBJob : public Job
{
    Q_OBJECT

  signals:
    void updateProgressSignal(float percent);

  public:
    UpdateBufferDBJob(DBInterface& db_interface, DBObject& dbobject, DBOVariable& key_var,
                      std::shared_ptr<Buffer> buffer);

    virtual ~UpdateBufferDBJob();

    virtual void run();

    std::shared_ptr<Buffer> buffer()
    {
        assert(buffer_);
        return buffer_;
    }

  protected:
    DBInterface& db_interface_;
    DBObject& dbobject_;
    DBOVariable& key_var_;
    std::shared_ptr<Buffer> buffer_;
};

#endif /* UpdateBufferDBJob_H_ */
