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

#include "taskmanagerlogwidget.h"

#include <QScrollBar>
#include <QString>

const std::map<QString, QColor> m_colours {{"red", QColor(Qt::red)},
                                           {"orange", QColor(255,165,0)},
                                           {"green", QColor(0,150,0)}
                                          };

TaskManagerLogWidget::TaskManagerLogWidget(QWidget* parent)
    : QPlainTextEdit(parent)
{
}

void TaskManagerLogWidget::appendSuccess(const std::string& text)
{
    std::string html_text = "<font color=\""+m_colours.at("green").name().toStdString()+"\">"+text+"</font>";
    this->appendHtml(html_text.c_str()); // Adds the message to the widget
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum()); // Scrolls to the bottom
}

void TaskManagerLogWidget::appendInfo(const std::string& text)
{
    this->appendPlainText(text.c_str()); // Adds the message to the widget
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum()); // Scrolls to the bottom
    //m_logFile.write(text); // Logs to file
}
void TaskManagerLogWidget::appendWarning(const std::string& text)
{
    std::string html_text = "<font color=\""+m_colours.at("orange").name().toStdString()+"\">"+text+"</font>";
    this->appendHtml(html_text.c_str()); // Adds the message to the widget
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum()); // Scrolls to the bottom
}
void TaskManagerLogWidget::appendError(const std::string& text)
{
    std::string html_text = "<font color=\""+m_colours.at("red").name().toStdString()+"\">"+text+"</font>";
    this->appendHtml(html_text.c_str()); // Adds the message to the widget
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum()); // Scrolls to the bottom
}
