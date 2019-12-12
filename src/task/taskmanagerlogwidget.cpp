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
//    appendSuccess ("Sucess");
//    appendInfo ("Info");
//    appendWarning ("Warning");
//    appendError ("Error");
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
