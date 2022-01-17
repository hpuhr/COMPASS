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

#ifndef DBOREADDBJOB_H_
#define DBOREADDBJOB_H_

#include "boost/date_time/posix_time/posix_time.hpp"
#include "dbcontent/variable/variableset.h"
#include "job.h"

class Buffer;
class DBContent;
class DBInterface;

/**
 * @brief DBO reading job
 *
 * Incrementally reads data record from DBO tables and writes the results into a DBDataSet.
 *
 */
class DBOReadDBJob : public Job
{
    Q_OBJECT

  signals:
    void intermediateSignal(std::shared_ptr<Buffer> buffer);

  public:
    DBOReadDBJob(DBInterface& db_interface, DBContent& dbobject, dbContent::VariableSet read_list,
                 std::string custom_filter_clause, std::vector<dbContent::Variable*> filtered_variables,
                 bool use_order, dbContent::Variable* order_variable, bool use_order_ascending,
                 const std::string& limit_str);
    virtual ~DBOReadDBJob();

    virtual void run();

    dbContent::VariableSet& readList() { return read_list_; }

    unsigned int rowCount() const;

  protected:
    DBInterface& db_interface_;
    DBContent& dbobject_;
    dbContent::VariableSet read_list_;
    std::string custom_filter_clause_;
    std::vector<dbContent::Variable*> filtered_variables_;
    bool use_order_;
    dbContent::Variable* order_variable_;
    bool use_order_ascending_;
    std::string limit_str_;

    unsigned int row_count_{0};

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;
};

#endif /* DBOREADDBJOB_H_ */
