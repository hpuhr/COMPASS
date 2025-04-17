
#include "taskresultswidget.h"
#include "taskresult.h"
#include "reportwidget.h"
#include "files.h"
#include "logger.h"
#include "taskmanager.h"
#include "asynctask.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QSettings>
#include <QMenu>
#include <QWidgetAction>
#include <QComboBox>
#include <QMessageBox>

using namespace Utils;

TaskResultsWidget::TaskResultsWidget(TaskManager& task_man)
    : ToolBoxWidget(nullptr), task_man_(task_man)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QHBoxLayout* top_layout = new QHBoxLayout;
    main_layout->addLayout(top_layout);

    report_combo_ = new QComboBox;
    report_combo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    connect(report_combo_, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            [ = ] (const QString& text){

                loginf << "TaskResultsWidget: report_combo_ currentText '" << text.toStdString() << "'";

                if (!text.size()) // happens on clear
                    return;

                assert (task_man_.hasResult(text.toStdString()));
                setReport(text.toStdString());
            });

    report_combo_->setDisabled(true);

    top_layout->addWidget(report_combo_);

    remove_result_button_ = new QPushButton("Remove Result");
    remove_result_button_->setIcon(QIcon(Files::getIconFilepath("delete.png").c_str()));
    remove_result_button_->setEnabled(false);

    connect(remove_result_button_, &QPushButton::pressed, this, &TaskResultsWidget::removeCurrentResult);

    top_layout->addWidget(remove_result_button_);

    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    // Add to a layout
    main_layout->addWidget(line);

    report_widget_ = new ResultReport::ReportWidget(*this);

    main_layout->addWidget(report_widget_);

    report_widget_->setDisabled(true);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);

    connect (&task_man, &TaskManager::taskResultsChangedSignal,
            this, &TaskResultsWidget::updateResultsSlot);
}

TaskResultsWidget::~TaskResultsWidget() {}

void TaskResultsWidget::setReport(const std::string name)
{
    loginf << "TaskResultsWidget: setReport: name '" << name << "'";

    current_report_name_ = name;

    int name_idx = report_combo_->findText(current_report_name_.c_str());

    loginf << "TaskResultsWidget: setReport: name_idx " << name_idx;

    if (name_idx < 0)
    {
        report_combo_->clear();
        report_combo_->setDisabled(true);

        current_report_name_ = "";

        report_widget_->clear();
        report_widget_->setDisabled(true);

        return;
    }

    if (report_combo_->currentIndex() != name_idx)
    {
        report_combo_->blockSignals(true);

        report_combo_->setCurrentIndex(name_idx);

        report_combo_->blockSignals(false);
    }

    report_combo_->setDisabled(false);

    assert (task_man_.hasResult(name));
    report_widget_->setReport(task_man_.getOrCreateResult(name)->report());

    report_widget_->setDisabled(false);
}

void TaskResultsWidget::updateResultsSlot()
{
    updateResults();
}

void TaskResultsWidget::updateResults(const std::string& selected_result)
{
    loginf << "TaskResultsWidget: updateResultsSlot";

    report_combo_->blockSignals(true);
    report_combo_->clear();

    //new selection specified?
    if (!selected_result.empty())
        current_report_name_ = selected_result;

    bool current_found = false;
    for (auto& res_it : task_man_.results())
    {
        loginf << "TaskResultsWidget: updateResultsSlot: adding '" << res_it.second->name() << "'";

        report_combo_->addItem(res_it.second->name().c_str());

        if (current_report_name_ == res_it.second->name())
            current_found = true;
    }

    //reset current report if not part of results
    if (!current_found)
        current_report_name_ = "";

    //current report not yet set => set to first
    if (current_report_name_.empty() && !task_man_.results().empty())
        current_report_name_ = task_man_.results().begin()->second->name();

    loginf << "TaskResultsWidget: updateResultsSlot: count " << report_combo_->count();

    if (!report_combo_->count())
        current_report_name_ = "";

    report_combo_->setEnabled(report_combo_->count() > 0);
    remove_result_button_->setEnabled(report_combo_->count() > 0);

    report_combo_->blockSignals(false);

    loginf << "TaskResultsWidget: updateResultsSlot: setReport";
    setReport(current_report_name_);
}

/**
 */
bool TaskResultsWidget::removeResult(const std::string& name)
{
    return task_man_.removeResult(name, false);
}

/**
 */
void TaskResultsWidget::removeCurrentResult()
{
    int idx = report_combo_->currentIndex();
    if (idx < 0)
        return;

    int n         = report_combo_->count();
    int new_index = idx < n - 1 ? idx + 1 : (idx > 0 ? idx - 1 : -1);

    auto name = report_combo_->currentText().toStdString();
    assert(task_man_.hasResult(name));

    QString msg = "Do you relly want to remove result '" + QString::fromStdString(name) + "'?";
    auto answer = QMessageBox::question(this, "Remove Result", msg, QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No);
    if (answer == QMessageBox::StandardButton::No)
        return;

    auto cb = [ this, name ] (const AsyncTaskState&, AsyncTaskProgressWrapper&) 
    { 
        return this->removeResult(name);
    };

    AsyncFuncTask task(cb, "Remove Result", "Removing result", false);
    task.runAsyncDialog(true, this);

    std::string selection_name = new_index >= 0 ? report_combo_->itemText(new_index).toStdString() : "";

    updateResults(selection_name);
}

/**
 */
QIcon TaskResultsWidget::toolIcon() const
{
    return QIcon(Utils::Files::getIconFilepath("reports.png").c_str());
}

/**
 */
std::string TaskResultsWidget::toolName() const
{
    return "Reports";
}

/**
 */
std::string TaskResultsWidget::toolInfo() const
{
    return "Reports";
}

/**
 */
std::vector<std::string> TaskResultsWidget::toolLabels() const
{
    return { "Reports" };
}

/**
 */
toolbox::ScreenRatio TaskResultsWidget::defaultScreenRatio() const
{
    return ToolBoxWidget::defaultScreenRatio();
}

/**
 */
void TaskResultsWidget::addToConfigMenu(QMenu* menu)
{
}

/**
 */
void TaskResultsWidget::addToToolBar(QToolBar* tool_bar)
{
}

/**
 */
void TaskResultsWidget::loadingStarted()
{
}

/**
 */
void TaskResultsWidget::loadingDone()
{
}

/**
 */
std::string TaskResultsWidget::currentReportName() const
{
    return current_report_name_;
}

/**
 */
void TaskResultsWidget::selectID(const std::string id)
{
    report_widget_->selectId(id);
}
