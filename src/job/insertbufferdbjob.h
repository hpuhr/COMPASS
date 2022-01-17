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

#ifndef INSERTBUFFERDBJOB_H_
#define INSERTBUFFERDBJOB_H_

#include <list>

#include "job.h"

#include <cassert>

class Buffer;
class DBContent;
class DBInterface;
class DBTable;

/**
 * @brief Buffer write job
 *
 * Writes buffer's data contents to a database table.
 */
class InsertBufferDBJob : public Job
{
    Q_OBJECT

  signals:
    void insertProgressSignal(float percent);

  public:
    InsertBufferDBJob(DBInterface& db_interface, DBContent& dbobject, std::shared_ptr<Buffer> buffer,
                      bool emit_change = true);

    virtual ~InsertBufferDBJob();

    virtual void run();

    std::shared_ptr<Buffer> buffer()
    {
        assert(buffer_);
        return buffer_;
    }

    bool emitChange() const;

  protected:
    DBInterface& db_interface_;
    DBContent& dbobject_;
    std::shared_ptr<Buffer> buffer_;
    bool emit_change_{true};

    void partialInsertBuffer(DBTable& table);
};

#endif /* INSERTBUFFERDBJOB_H_ */
