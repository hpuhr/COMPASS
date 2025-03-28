#include "logwidget.h"
#include "reportwidget.h"
#include "files.h"
#include "logger.h"
#include "taskmanager.h"

#include <QVBoxLayout>
#include <QDateTime>
#include <QScrollBar>

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

    default_icon_ = QIcon();  // No icon by default
    error_icon_ = QIcon(":/icons/red_dot.png");  // Replace with your red alert icon

    updateIcon();
}

void LogWidget::addLogMessage(const QString& message, LogType type)
{
    bool accepted = (type != LogType::Error);
    log_entries_.push_back({message, type, accepted});

    updateDisplay();
    updateIcon();
}

// void LogWidget::onWidgetActivated()
// {
//     for (auto& entry : log_entries_)
//     {
//         if (entry.type == LogType::Error)
//             entry.accepted = true;
//     }

//     updateIcon();
// }

void LogWidget::updateDisplay()
{
    text_display_->clear();

    for (const auto& entry : log_entries_)
    {
        QString timestamp = QDateTime::currentDateTime().toString("[yyyy-MM-dd HH:mm:ss] ");
        QString color;

        switch (entry.type)
        {
        case LogType::Info:    color = "black";   break;
        case LogType::Warning: color = "orange";  break;
        case LogType::Error:   color = "red";     break;
        }

        QString formatted = QString("<span style='color:%1;'>%2%3</span>")
                                .arg(color, timestamp, entry.message);
        text_display_->append(formatted);
    }

    QScrollBar* sb = text_display_->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void LogWidget::updateIcon()
{
    bool has_unaccepted_errors = std::any_of(
        log_entries_.begin(), log_entries_.end(),
        [](const LogEntry& entry) {
            return entry.type == LogType::Error && !entry.accepted;
        });

   // setIcon(has_unaccepted_errors ? error_icon_ : default_icon_);
}

/**
 */
QIcon LogWidget::toolIcon() const
{
    return QIcon(Utils::Files::getIconFilepath("log.png").c_str());
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


