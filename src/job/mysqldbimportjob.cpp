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

#include "mysqldbimportjob.h"

#include <archive.h>
#include <archive_entry.h>

#include <fstream>
#include <iostream>

#include "atsdb.h"
#include "dbinterface.h"
#include "files.h"
#include "logger.h"
#include "mysqlppconnection.h"
#include "stringconv.h"

using namespace Utils;

MySQLDBImportJob::MySQLDBImportJob(const std::string& filename, bool archive,
                                   MySQLppConnection& connection)
    : Job("MySQLDBImportJob"), filename_(filename), archive_(archive), connection_(connection)
{
}

MySQLDBImportJob::~MySQLDBImportJob() {}

void MySQLDBImportJob::run()
{
    logdbg << "MySQLDBImportJob: run";

    started_ = true;

    if (archive_)
        importSQLArchiveFile();
    else
        importSQLFile();

    done_ = true;

    logdbg << "MySQLDBImportJob: run: done";
}

size_t MySQLDBImportJob::numLines() const { return num_lines_; }

size_t MySQLDBImportJob::numErrors() const { return num_errors_; }

bool MySQLDBImportJob::quitBecauseOfErrors() const { return quit_because_of_errors_; }

void MySQLDBImportJob::importSQLFile()
{
    loginf << "MySQLDBImportJob: importSQLFile: importing " << filename_;
    assert(Files::fileExists(filename_));

    std::ifstream is;
    is.open(filename_.c_str(), std::ios::binary);
    is.seekg(0, std::ios::end);
    size_t file_byte_size = is.tellg();
    is.close();
    assert(file_byte_size);
    loginf << "MySQLDBImportJob: importSQLFile: file_byte_size: " << file_byte_size;

    //    for (unsigned int cnt=0; cnt < 10; cnt++)
    //        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    std::ifstream sql_file(filename_);
    assert(sql_file.is_open());

    std::string line;
    std::stringstream ss;
    size_t byte_cnt = 0;

    while (getline(sql_file, line))
    {
        try
        {
            byte_cnt += line.size();

            if (line.find("delimiter") != std::string::npos ||
                line.find("DELIMITER") != std::string::npos ||
                line.find("VIEW") != std::string::npos)
            {
                loginf << "MySQLDBImportJob: importSQLFile: breaking at delimiter, bytes "
                       << byte_cnt;
                break;
            }

            ss << line << '\n';

            if (line.back() == ';')
            {
                //  loginf << "MySQLppConnection: importSQLFile: line cnt " << line_cnt << " of " <<
                //  line_count
                //            << " strlen " << ss.str().size() << "'";

                if (ss.str().size())
                    connection_.executeSQL(ss.str());

                ss.str("");
            }

            ++num_lines_;

            if (num_lines_ % 10 == 0)
            {
                emit statusSignal("Read " + std::to_string(num_lines_) + " lines");
            }
        }
        catch (std::exception& e)
        {
            logwrn << "MySQLDBImportJob: importSQLFile: sql error '" << e.what() << "'";
            ss.str("");
            ++num_errors_;

            if (num_errors_ > 3)
            {
                logwrn << "MySQLDBImportJob: importSQLFile: quit after too many errors";

                quit_because_of_errors_ = true;
                break;
            }
        }
    }

    sql_file.close();
    ATSDB::instance().interface().databaseContentChanged();
}

void MySQLDBImportJob::importSQLArchiveFile()
{
    loginf << "MySQLDBImportJob: importSQLArchiveFile: importing " << filename_;
    assert(Files::fileExists(filename_));

    // if gz but not tar.gz or tgz
    bool raw = String::hasEnding(filename_, ".gz") && !String::hasEnding(filename_, ".tar.gz");

    loginf << "MySQLDBImportJob: importSQLArchiveFile: importing " << filename_ << " raw " << raw;

    struct archive* a;
    struct archive_entry* entry;
    int r;

    a = archive_read_new();

    if (raw)
    {
        archive_read_support_filter_gzip(a);
        archive_read_support_filter_bzip2(a);
        archive_read_support_format_raw(a);
    }
    else
    {
        archive_read_support_filter_all(a);
        archive_read_support_format_all(a);
    }
    r = archive_read_open_filename(a, filename_.c_str(), 10240);  // Note 1

    if (r != ARCHIVE_OK)
        throw std::runtime_error("MySQLDBImportJob: importSQLArchiveFile: archive error: " +
                                 std::string(archive_error_string(a)));

    const void* buff;
    size_t size;
    int64_t offset;

    size_t byte_cnt = 0;

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
    {
        loginf << "MySQLDBImportJob: importSQLArchiveFile: archive file found: "
               << archive_entry_pathname(entry) << " size " << archive_entry_size(entry);

        // msg_box.setInformativeText(archive_entry_pathname(entry));

        bool done = false;

        std::stringstream ss;

        for (;;)
        {
            r = archive_read_data_block(a, &buff, &size, &offset);

            if (r == ARCHIVE_EOF)
                break;
            if (r != ARCHIVE_OK)
                throw std::runtime_error("MySQLDBImportJob: importSQLArchiveFile: archive error: " +
                                         std::string(archive_error_string(a)));

            std::string str(reinterpret_cast<char const*>(buff), size);

            std::vector<std::string> lines = String::split(str, '\n');
            std::string line;

            // loginf << "UGA read str has " << lines.size() << " lines";

            for (std::vector<std::string>::iterator line_it = lines.begin(); line_it != lines.end();
                 line_it++)
            {
                if (line_it + 1 == lines.end())
                {
                    // loginf << "last one";
                    ss << *line_it;
                    break;
                }

                try
                {
                    ss << *line_it << '\n';

                    line = ss.str();

                    byte_cnt += line.size();

                    if (line.find("delimiter") != std::string::npos ||
                        line.find("DELIMITER") != std::string::npos ||
                        line.find("VIEW") != std::string::npos)
                    {
                        loginf << "MySQLDBImportJob: importSQLArchiveFile: breaking at delimiter, "
                                  "bytes "
                               << byte_cnt;
                        done = true;
                        break;
                    }

                    if (line_it->back() == ';')
                    {
                        //                        loginf << "MySQLppConnection:
                        //                        importSQLArchiveFile: line cnt " << line_cnt
                        //                               <<  " strlen " << ss.str().size() << "'";

                        if (line.size())
                            connection_.executeSQL(line + "\n");

                        ss.str("");
                    }

                    if (num_lines_ % 10 == 0)
                    {
                        logdbg << "MySQLppConnection: importSQLArchiveFile: line cnt "
                               << num_lines_;

                        emit statusSignal("Read " + std::to_string(num_lines_) +
                                          " lines from archive entry '" +
                                          std::string(archive_entry_pathname(entry)) + "'");
                    }

                    ++num_lines_;
                }
                catch (std::exception& e)
                {
                    logwrn << "MySQLppConnection: importSQLArchiveFile: sql error '" << e.what()
                           << "'";
                    ss.str("");
                    ++num_errors_;

                    if (num_errors_ > 3)
                    {
                        logwrn << "MySQLppConnection: importSQLArchiveFile: quit after too many "
                                  "errors";

                        quit_because_of_errors_ = true;
                        done = true;
                        break;
                    }
                }
            }

            if (done)
                break;
        }

        if (done)
            break;

        loginf << "MySQLppConnection: importSQLArchiveFile: archive file "
               << archive_entry_pathname(entry) << " imported";
    }

    r = archive_read_close(a);
    if (r != ARCHIVE_OK)
        throw std::runtime_error(
            "MySQLppConnection: importSQLArchiveFile: archive read close error: " +
            std::string(archive_error_string(a)));

    r = archive_read_free(a);

    if (r != ARCHIVE_OK)
        throw std::runtime_error(
            "MySQLppConnection: importSQLArchiveFile: archive read free error: " +
            std::string(archive_error_string(a)));

    ATSDB::instance().interface().databaseContentChanged();
}
