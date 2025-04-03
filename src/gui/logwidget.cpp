#include "logwidget.h"
#include "reportwidget.h"
#include "files.h"
#include "logger.h"
#include "taskmanager.h"

#include <QVBoxLayout>
#include <QDateTime>
#include <QScrollBar>
#include <QToolBar>
#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>

#include <algorithm>

using namespace Utils;

LogWidget::LogWidget(LogStore& log_store)
    : ToolBoxWidget(nullptr), log_store_(log_store)
{

    QVBoxLayout* main_layout = new QVBoxLayout();

    // table
    proxy_model_ = new QSortFilterProxyModel();
    proxy_model_->setSourceModel(&log_store_);

    table_view_ = new QTableView();
    table_view_->setModel(proxy_model_);
    table_view_->setSortingEnabled(true);
    table_view_->sortByColumn(1, Qt::AscendingOrder);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    table_view_->horizontalHeader()->setMaximumSectionSize(300);
    //table_view_->setIconSize(QSize(24, 24));
    table_view_->setContextMenuPolicy(Qt::CustomContextMenu);
    table_view_->setWordWrap(true);
    table_view_->reset();
    // update done later

    // connect(table_view_, &QTableView::customContextMenuRequested,
    //         this, &TargetListWidget::customContextMenuSlot);

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
    main_layout->addWidget(table_view_);

    setLayout(main_layout);

    default_icon_ = QIcon(Utils::Files::getIconFilepath("log.png").c_str());
    error_icon_ = QIcon(Utils::Files::getIconFilepath("log_err.png").c_str());

    connect (&log_store_, &LogStore::messagesChangedSignal,
            this, &LogWidget::messagesChangedSlot, Qt::QueuedConnection);

    //updateDisplay();
    checkIcon();
}

void LogWidget::messagesChangedSlot()
{
    //updateDisplay();
    checkIcon();
}

void LogWidget::acceptMessagesSlot()
{
    log_store_.acceptMessages();
}

// void LogWidget::updateDisplay()
// {
//     text_display_->clear();

//     for (const auto& entry : log_store_.logEntries())
//     {
//         QString prefix;

//         QString color = "black";
//         QString style;

//         if (!entry.accepted) {
//             switch (entry.type) {
//             case LogStreamType::Warning:
//                 color = "#FFA500";  // orange
//                 style = "font-style:italic;";
//                 prefix = "WARN";
//                 break;
//             case LogStreamType::Error:
//                 color = "red";
//                 style = "font-weight:bold;";
//                 prefix = "ERR";
//                 break;
//             case LogStreamType::Info:
//                 prefix = "INFO";
//                 break;
//             }
//         } else {
//             // Even if accepted, set prefix
//             switch (entry.type) {
//             case LogStreamType::Warning: prefix = "WARN"; break;
//             case LogStreamType::Error:   prefix = "ERROR";  break;
//             case LogStreamType::Info:    prefix = "INFO"; break;
//             }
//         }

//         QString fullMessage = QString("%1 [%2]: %3")
//                                   .arg(entry.timestamp, prefix, entry.message.c_str());

//         QString formatted = QString("<span style='font-family:monospace; color:%1;%2'>%3</span>")
//                                 .arg(color, style, fullMessage);
//         text_display_->append(formatted);
//     }

//     QScrollBar* sb = text_display_->verticalScrollBar();
//     sb->setValue(sb->maximum());
// }

void LogWidget::checkIcon()
{
    auto old_val = has_unaccepted_errors_;

    has_unaccepted_errors_ = std::any_of(
        log_store_.logEntries().begin(), log_store_.logEntries().end(),
        [](const LogStore::LogEntry& entry) {
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
    connect(action_accept, &QAction::triggered, this, &LogWidget::acceptMessagesSlot);
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


