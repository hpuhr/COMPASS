/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include <QApplication>

#include <memory>

class MainWindow;

//namespace ATSDB
//{

/**
 * @brief Main Class
 *
 */
class Client : public QApplication
{
public:
    ///@brief Constructor.
  Client(int& argc, char** argv);
  ///@brief Destructor.
  virtual ~Client();

  ///@brief Re-implementation from QApplication so exceptions can be thrown in slots.
  virtual bool notify(QObject* receiver, QEvent* event);

  bool quitRequested() const;
  MainWindow& mainWindow ();

private:
  //bool atsdb_initialized_ {false};

  std::string system_install_path_;
  bool quit_requested_ {false};

  bool config_and_data_reset_wanted_ {false};

  bool config_and_data_exists_ {false};
  bool config_and_data_copied_ {false};

  bool upgrade_needed_ {false};
  bool config_and_data_deletion_wanted_ {false};

  std::unique_ptr<MainWindow> main_window_;

  void checkAndSetupConfig ();

  void checkNeededActions ();
  void performNeededActions ();

  void deleteConfigurationAndData ();
  void copyConfigurationAndData ();
  void copyConfiguration ();
};
//}

#endif /* CLIENT_H_ */
