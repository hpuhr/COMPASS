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
  Client(int& argc, char ** argv);
  ///@brief Destructor.
  virtual ~Client() { }

  ///@brief Re-implementation from QApplication so exceptions can be thrown in slots.
  virtual bool notify(QObject * receiver, QEvent * event);

  bool quitRequested() const;

private:
  bool quit_requested_ {false};

  void copyConfigurationAndData (const std::string& system_install_path);
  void copyConfiguration (const std::string& system_install_path);
};
//}

#endif /* CLIENT_H_ */
