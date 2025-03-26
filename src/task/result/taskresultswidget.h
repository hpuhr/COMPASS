#pragma once

#include "toolboxwidget.h"

#include <boost/optional.hpp>

#include <QWidget>

class TaskManager;

namespace ResultReport
{
    class ReportWidget;
}

class TaskResultsWidget : public ToolBoxWidget
{
    Q_OBJECT

public slots:

public:
    TaskResultsWidget(TaskManager& task_man);

    virtual ~TaskResultsWidget();

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
    TaskManager& task_man_;

    ResultReport::ReportWidget* report_widget_;
};


