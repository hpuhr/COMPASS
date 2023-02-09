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
#include "files.h"

#include <QTextEdit>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QShortcut>
#include <QListWidget>
#include <QPushButton>
#include <QToolButton>

namespace rtcommand
{

namespace
{
    QIcon icon(const std::string& fn) 
    {
        return QIcon(Utils::Files::getIconFilepath(fn).c_str());
    };
}

/**************************************************************************************
 * RTCommandBacklogWidget
 **************************************************************************************/

/** 
 */
RTCommandBacklogWidget::RTCommandBacklogWidget(QWidget* parent)
:   QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    command_list_ = new QListWidget(this);
    command_list_->setSelectionMode(QListWidget::SelectionMode::ExtendedSelection);
    layout->addWidget(command_list_);

    QHBoxLayout* layout_h = new QHBoxLayout;
    layout->addLayout(layout_h);

    QToolButton* clear_button = new QToolButton;
    clear_button->setIcon(icon("delete.png"));
    clear_button->setToolTip("Remove selected commands from backlog");
    layout_h->addWidget(clear_button);

    QPushButton* clear_all_button = new QPushButton("Clear Backlog");
    clear_all_button->setToolTip("Remove all commands from backlog");
    clear_all_button->setIcon(icon("delete.png"));
    layout_h->addWidget(clear_all_button);

    layout_h->addStretch(1);
    
    connect(command_list_, &QListWidget::itemDoubleClicked, this, &RTCommandBacklogWidget::acceptCommandItem);
    connect(clear_button, &QToolButton::pressed, this, &RTCommandBacklogWidget::clearSelected);
    connect(clear_all_button, &QPushButton::pressed, this, &RTCommandBacklogWidget::clearAll);

    updateList();
}

/**
 */
void RTCommandBacklogWidget::updateList()
{
    auto cmds = RTCommandManager::instance().commandBacklog();

    command_list_->clear();

    for (const auto& cmd : cmds)
        command_list_->addItem(QString::fromStdString(cmd));

    if (command_list_->count() > 0)
        command_list_->setCurrentRow(command_list_->count() - 1);
}

/** 
 */
void RTCommandBacklogWidget::clearSelected()
{
    //@TODO
    updateList();
}

/** 
 */
void RTCommandBacklogWidget::clearAll()
{
    RTCommandManager::instance().clearBacklog();
    updateList();
}

/** 
 */
void RTCommandBacklogWidget::acceptCommandItem(QListWidgetItem* item)
{
    emit commandSelected(item->text());
}

/**************************************************************************************
 * RTCommandBacklogDialog
 **************************************************************************************/

/** 
 */
RTCommandBacklogDialog::RTCommandBacklogDialog(QWidget* parent)
:   QDialog(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    RTCommandBacklogWidget* widget = new RTCommandBacklogWidget(this);
    layout->addWidget(widget);

    auto cb = [ = ] (const QString& cmd)
    {
        this->selected_command_ = cmd;
        this->accept();
    };

    connect(widget, &RTCommandBacklogWidget::commandSelected, cb);
}

/**************************************************************************************
 * RTCommandShell
 **************************************************************************************/

/** 
 */
RTCommandShell::RTCommandShell(QWidget* parent)
:   QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    cmd_shell_ = new QTextEdit;
    cmd_shell_->setReadOnly(true);
    cmd_shell_->setFrameStyle(QTextEdit::NoFrame);
    layout->addWidget(cmd_shell_);

    QHBoxLayout* layout_h = new QHBoxLayout;
    layout->addLayout(layout_h);

    cmd_edit_ = new QLineEdit;
    cmd_edit_->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    cmd_edit_->setCursorPosition(0);
    layout_h->addWidget(cmd_edit_);

    QToolButton* backlog_button = new QToolButton;
    backlog_button->setIcon(icon("edit_old.png"));
    backlog_button->setToolTip("Show command backlog");
    layout_h->addWidget(backlog_button);

    connect(cmd_edit_, &QLineEdit::returnPressed, this, &RTCommandShell::processCommand);
    connect(backlog_button, &QToolButton::pressed, this, &RTCommandShell::showBacklog);

    auto s_last = new QShortcut(cmd_edit_);
    s_last->setKey(Qt::Key_Up);
    connect(s_last, &QShortcut::activated, this, &RTCommandShell::lastCmd);

    auto s_next = new QShortcut(cmd_edit_);
    s_next->setKey(Qt::Key_Down);
    connect(s_next, &QShortcut::activated, this, &RTCommandShell::nextCmd);
}

/**
*/
void RTCommandShell::resetLocalBacklog()
{
    local_backlog_.clear();
    local_backlog_index_ = -1;
}

/**
*/
void RTCommandShell::updateCommandFromLocalBacklog()
{
    cmd_edit_->setText("");
    cmd_edit_->setCursorPosition(0);

    if (local_backlog_.empty() || local_backlog_index_ < 0)
        return;

    cmd_edit_->setText(QString::fromStdString(local_backlog_.at(local_backlog_index_)));
    cmd_edit_->setCursorPosition(cmd_edit_->text().count());
}

/**
*/
void RTCommandShell::showBacklog()
{
    RTCommandBacklogDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted)
    {
        const QString& cmd = dlg.selectedCommand();
        cmd_edit_->setText(cmd);
        cmd_edit_->setCursorPosition(cmd.count());
    }
}

/**
*/
void RTCommandShell::processCommand()
{
    QString line = cmd_edit_->text().trimmed();
    
    cmd_edit_->setText("");
    resetLocalBacklog();

    if (line.isEmpty())
        return;

    bool ok = RTCommandManager::instance().addCommand(line.toStdString());

    if (ok)
        log(line, LogType::Plain);
    else
        log("[Error] Creating command '" + line + "' failed", LogType::Error);
}

/**
*/
void RTCommandShell::log(const QString& txt, LogType log_type)
{
    QColor color = Qt::black; //LogType::Plain
    if (log_type == LogType::Error)
        color = Qt::red;
    else if (log_type == LogType::Warning)
        color = Qt::darkYellow;
    else if (log_type == LogType::Success)
        color = Qt::darkGreen;

    cmd_shell_->setTextColor(color);
    cmd_shell_->append(txt);
}

/**
*/
void RTCommandShell::lastCmd()
{
    if (local_backlog_.empty())
    {
        //init local backlog
        local_backlog_ = RTCommandManager::instance().commandBacklog();
        if (local_backlog_.empty())
            return;

        local_backlog_index_ = (int)local_backlog_.size() - 1;
    }
    else 
    {
        local_backlog_index_ = std::max(0, local_backlog_index_ - 1);
    }

    updateCommandFromLocalBacklog();
}

/**
*/
void RTCommandShell::nextCmd()
{
    if (local_backlog_.empty())
        return;

    int n = (int)local_backlog_.size();

    ++local_backlog_index_;

    if (local_backlog_index_ == n)
    {
        resetLocalBacklog();
    }

    updateCommandFromLocalBacklog();
}

} // namespace rtcommand
