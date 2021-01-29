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

#ifndef MYSQLDBIMPORTJOB_H
#define MYSQLDBIMPORTJOB_H

#include "job.h"

class MySQLppConnection;

class MySQLDBImportJob : public Job
{
    Q_OBJECT
  signals:
    void statusSignal(const QString message);

  public:
    MySQLDBImportJob(const std::string& filename, bool archive, MySQLppConnection& connection);
    virtual ~MySQLDBImportJob();

    virtual void run();

    size_t numLines() const;
    size_t numErrors() const;
    bool quitBecauseOfErrors() const;

  private:
    std::string filename_;
    bool archive_{false};

    MySQLppConnection& connection_;

    size_t num_lines_{0};
    size_t num_errors_{0};
    bool quit_because_of_errors_{false};

    void importSQLFile();
    void importSQLArchiveFile();
};

#endif  // MYSQLDBIMPORTJOB_H
