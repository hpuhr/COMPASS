#pragma once

#include "toolboxwidget.h"

#include <boost/optional.hpp>

#include <QWidget>

class TaskManager;

namespace ResultReport
{
    class ReportWidget;
}

class QComboBox;
class QPushButton;

class TaskResultsWidget : public ToolBoxWidget
{
    Q_OBJECT

public slots:
    void updateResultsSlot();

public:
    TaskResultsWidget(TaskManager& task_man);
    virtual ~TaskResultsWidget();

    void setReport(const std::string name);
    void selectID(const std::string id,
                  bool show_figure = false);
    void restoreBackupSection();

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

    std::string currentReportName() const;

protected:
    void updateResults(const std::string& selected_result = "");
    void removeCurrentResult();
    bool removeResult(const std::string& name);

    TaskManager& task_man_;

    QComboBox* report_combo_ {nullptr};
    QPushButton* remove_result_button_ {nullptr};
    std::string current_report_name_;
    std::string current_report_name_backup_;
    std::string current_section_name_backup_;

    ResultReport::ReportWidget* report_widget_;
};


