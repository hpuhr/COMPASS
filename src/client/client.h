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

#ifndef CLIENT_H_
#define CLIENT_H_

#include "global.h"

#include <QApplication>

class Client : public QApplication
{
public:
    Client(int& argc, char** argv);
    virtual ~Client();

    virtual bool notify(QObject* receiver, QEvent* event);

    bool quitRequested() const;

    void run ();

private:

    std::string system_install_path_;
    bool quit_requested_{false};

    bool home_subdir_deletion_wanted_{false};
    bool config_and_data_copy_wanted_{false};

    std::string create_new_sqlite3_db_filename_;
    std::string open_sqlite3_db_filename_;
    //    std::string import_view_points_filename;
    std::string import_asterix_filename_;
    bool import_asterix_network_ {false};
    std::string import_asterix_network_time_offset_;
    //    std::string asterix_framing;
    //    std::string asterix_decoder_cfg;

    void checkAndSetupConfig();

    void checkNeededActions();
    void performNeededActions();

    void deleteCompleteHomeSubDir();
    void copyConfigurationAndData();
};

#endif /* CLIENT_H_ */
