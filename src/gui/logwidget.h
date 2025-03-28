#pragma once

#include "toolboxwidget.h"
#include <QTextEdit>
#include <QIcon>

#include <string>
#include <vector>

class LogWidget : public ToolBoxWidget
{
    Q_OBJECT

public:
    enum class LogType { Info, Warning, Error };

    explicit LogWidget();

    void addLogMessage(const QString& message, LogType type);

    //ToolBoxWidget
    QIcon toolIcon() const override final;
    std::string toolName() const override final;
    std::string toolInfo() const override final;
    std::vector<std::string> toolLabels() const override final;
    toolbox::ScreenRatio defaultScreenRatio() const override final;
    void addToConfigMenu(QMenu* menu) override final;
    void addToToolBar(QToolBar* tool_bar) override final;
    void loadingStarted() override final;
    void loadingDone() override final;

protected:
    //void onWidgetActivated() override;

private:
    struct LogEntry {
        QString message;
        LogType type;
        bool accepted;
    };

    void updateDisplay();
    void updateIcon();

    std::vector<LogEntry> log_entries_;
    QTextEdit* text_display_;
    QIcon default_icon_;
    QIcon error_icon_;
};
