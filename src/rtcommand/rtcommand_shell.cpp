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
#include "rtcommand_manager.h"

#include <QTextEdit>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QShortcut>
#include <QProcess>

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

    connect(cmd_edit_, &QLineEdit::returnPressed, this, &RTCommandShell::processCommand);

    auto s_last = new QShortcut(cmd_edit_);
    s_last->setKey(Qt::Key_Up);
    connect(s_last, &QShortcut::activated, this, &RTCommandShell::lastCmd);

    auto s_next = new QShortcut(cmd_edit_);
    s_next->setKey(Qt::Key_Down);
    connect(s_next, &QShortcut::activated, this, &RTCommandShell::nextCmd);

    cmd_edit_->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    cmd_edit_->setCursorPosition(0);

    //commands_.push_back("uiset --object=window5.osgview1.reload");
    //commands_.push_back("uiset --object=window5.osgview1.toolbar --value=\"Toggle Time Filter\"");
    //commands_.push_back("uiset --object=window5.osgview1.timefilter.slider --value=0.5");
}

/**
*/
void RTCommandShell::processCommand()
{
    QString line = cmd_edit_->text().trimmed();
    
    cmd_edit_->setText("");

    if (line.isEmpty())
        return;

    current_command_ = -1;
    commands_.push_back(line);

    bool ok = RTCommandManager::instance().addCommand(line.toStdString());

    if (ok)
        log(line, LogType::Success);
    else
        log("[Error] Creating command '" + line + "' failed", LogType::Error);
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

/**
*/
void RTCommandShell::lastCmd()
{
    int n = (int)commands_.size();
    if (n == 0)
        return;

    current_command_ = current_command_ < 0 ? n - 1 : std::max(0, current_command_ - 1);
    cmd_edit_->setText(commands_[ current_command_ ]);
    cmd_edit_->setCursorPosition(cmd_edit_->text().count());
}

/**
*/
void RTCommandShell::nextCmd()
{
    int n = (int)commands_.size();
    if (n == 0)
        return;

    if (current_command_ < 0)
        return;
    
    ++current_command_;
    if (current_command_ == n)
    {
        cmd_edit_->setText("");
        current_command_ = -1;
        return;
    }

    cmd_edit_->setText(commands_[ current_command_ ]);
    cmd_edit_->setCursorPosition(cmd_edit_->text().count());
}

} // namespace rtcommand
