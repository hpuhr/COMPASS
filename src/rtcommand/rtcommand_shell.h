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

#pragma once 

#include <QWidget>

class QTextEdit;
class QLineEdit;

namespace rtcommand
{

/**
 * 
 */
class RTCommandShell : public QWidget 
{
public:
    RTCommandShell(QWidget* parent = nullptr);
    virtual ~RTCommandShell() = default;

private:
    enum class LogType 
    {
        Plain = 0,
        Warning,
        Error,
        Success
    };

    void processCommand();
    void log(const QString& txt, LogType log_type = LogType::Plain);

    QTextEdit*   cmd_backlog_;
    QLineEdit*   cmd_edit_;
};

} // namespace rtcommand
