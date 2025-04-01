#include "logwidget.h"
#include "reportwidget.h"
#include "files.h"
#include "logger.h"
#include "taskmanager.h"

#include <QVBoxLayout>
#include <QDateTime>
#include <QScrollBar>
#include <QToolBar>

#include <algorithm>

using namespace Utils;

LogWidget::LogWidget()
    : ToolBoxWidget(nullptr),
    text_display_(new QTextEdit(this))
{
    text_display_->setReadOnly(true);
    text_display_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto* layout = new QVBoxLayout();
    layout->addWidget(text_display_);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    default_icon_ = QIcon(Utils::Files::getIconFilepath("log.png").c_str());
    error_icon_ = QIcon(Utils::Files::getIconFilepath("log_err.png").c_str());

    checkIcon();

    // addLogMessage("Quest started: Seeking the Holy Grail... again.", LogStreamType::Info);
    // addLogMessage("System encountered a shrubbery-related conflict.", LogStreamType::Warning);
    // addLogMessage("French taunter detected on network. Expect rude messages.", LogStreamType::Error);

    // addLogMessage("User 'moss' requested admin rights. Denied for safety.", LogStreamType::Info);
    // addLogMessage("Roy is asking, 'Have you tried turning it off and on again?'", LogStreamType::Warning);
    // addLogMessage("Error: Jen clicked something. Nobody knows what happened.", LogStreamType::Error);

    // addLogMessage("Black Knight: 'Tis but a scratch!' Ignoring minor system fault.", LogStreamType::Info);
    // addLogMessage("Cat on the keyboard. Input may be unpredictable.", LogStreamType::Warning);
    // addLogMessage("The server room is on fire! Again!", LogStreamType::Error);
}

void LogWidget::addLogMessage(const std::string& message, LogStreamType type)
{
    log_entries_.push_back({QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"), message, type, false});

    updateDisplay();
    checkIcon();
}

void LogWidget::acceptMessages()
{
    for (auto& entry : log_entries_)
        entry.accepted = true;

    updateDisplay();

    checkIcon();
}

void LogWidget::updateDisplay()
{
    text_display_->clear();

    for (const auto& entry : log_entries_)
    {
        QString prefix;

        QString color = "black";
        QString style;

        if (!entry.accepted) {
            switch (entry.type) {
            case LogStreamType::Warning:
                color = "#FFA500";  // orange
                style = "font-style:italic;";
                prefix = "WARN";
                break;
            case LogStreamType::Error:
                color = "red";
                style = "font-weight:bold;";
                prefix = "ERR";
                break;
            case LogStreamType::Info:
                prefix = "INFO";
                break;
            }
        } else {
            // Even if accepted, set prefix
            switch (entry.type) {
            case LogStreamType::Warning: prefix = "WARN"; break;
            case LogStreamType::Error:   prefix = "ERROR";  break;
            case LogStreamType::Info:    prefix = "INFO"; break;
            }
        }

        QString fullMessage = QString("%1 [%2]: %3")
                                  .arg(entry.timestamp, prefix, entry.message.c_str());

        QString formatted = QString("<span style='font-family:monospace; color:%1;%2'>%3</span>")
                                .arg(color, style, fullMessage);
        text_display_->append(formatted);
    }

    QScrollBar* sb = text_display_->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void LogWidget::checkIcon()
{
    auto old_val = has_unaccepted_errors_;

    has_unaccepted_errors_ = std::any_of(
        log_entries_.begin(), log_entries_.end(),
        [](const LogEntry& entry) {
            return entry.type == LogStreamType::Error && !entry.accepted;
        });

    if (old_val != has_unaccepted_errors_)
        emit iconChangedSignal();
}

/**
 */
QIcon LogWidget::toolIcon() const
{
    if (has_unaccepted_errors_)
        return error_icon_;
    else
        return default_icon_;
}

/**
 */
std::string LogWidget::toolName() const
{
    return "Log";
}

/**
 */
std::string LogWidget::toolInfo() const
{
    return "Log";
}

/**
 */
std::vector<std::string> LogWidget::toolLabels() const
{
    return { "Log" };
}

/**
 */
toolbox::ScreenRatio LogWidget::defaultScreenRatio() const
{
    return ToolBoxWidget::defaultScreenRatio();
}

/**
 */
void LogWidget::addToConfigMenu(QMenu* menu)
{
}

/**
 */
void LogWidget::addToToolBar(QToolBar* tool_bar)
{
    auto action_accept = tool_bar->addAction("Accept");
    action_accept->setIcon(QIcon(Utils::Files::getIconFilepath("done.png").c_str()));
    connect(action_accept, &QAction::triggered, this, &LogWidget::acceptMessages);
}

/**
 */
void LogWidget::loadingStarted()
{
}

/**
 */
void LogWidget::loadingDone()
{
}


