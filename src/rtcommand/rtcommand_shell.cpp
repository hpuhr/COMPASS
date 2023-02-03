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

#include "rtcommand_shell.h"
#include "rtcommand_registry.h"

#include <QTextEdit>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace rtcommand
{

/** 
 */
RTCommandShell::RTCommandShell(QWidget* parent)
:   QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    cmd_backlog_ = new QTextEdit;
    cmd_backlog_->setReadOnly(true);
    cmd_backlog_->setFrameStyle(QTextEdit::NoFrame);
    layout->addWidget(cmd_backlog_);

    cmd_edit_ = new QLineEdit;
    layout->addWidget(cmd_edit_);

    cmd_edit_->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    connect(cmd_edit_, &QLineEdit::returnPressed, this, &RTCommandShell::processCommand);
}

/**
*/
void RTCommandShell::processCommand()
{
    QString line = cmd_edit_->text().trimmed();

    cmd_edit_->setText("");

    if (line.isEmpty())
        return;

    QStringList parts = line.split(" ");
    if (parts.count() < 1)
        return;

    QString cmd = parts.front();
    if (cmd.isEmpty())
        return;

    const auto& cmds = RTCommandRegistry::instance().availableCommands();

    auto it = cmds.find(cmd);
    if (it == cmds.end())
    {
        log("Command '" + cmd + "' not recognized", LogType::Error);
        return;
    }

    const auto& descr = it->second;
    
    log(descr.name + "\n    " + descr.description, LogType::Success);
}

/**
*/
void RTCommandShell::log(const QString& txt, LogType log_type)
{
    QString backlog = cmd_backlog_->toHtml();

    QString color = "black"; //LogType::Plain
    if (log_type == LogType::Error)
        color = "red";
    else if (log_type == LogType::Warning)
        color = "orange";
    else if (log_type == LogType::Success)
        color = "green";
    
    backlog += "<text color=\"" + color + "\">" + txt + "</text><br>";

    cmd_backlog_->setHtml(backlog);
}

} // namespace rtcommand
