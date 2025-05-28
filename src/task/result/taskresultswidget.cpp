
#include "taskresultswidget.h"
#include "taskresult.h"
#include "reportwidget.h"
#include "files.h"
#include "logger.h"
#include "taskmanager.h"
#include "asynctask.h"
#include "compass.h"
#include "reportdefs.h"

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
#include <QFileDialog>

using namespace Utils;

/**
 */
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

    refresh_result_button_ = new QPushButton();
    refresh_result_button_->setIcon(Files::IconProvider::getIcon("refresh.png"));
    refresh_result_button_->setEnabled(false);
    refresh_result_button_->setFlat(true);
    refresh_result_button_->setToolTip("Refresh Result");

    connect(refresh_result_button_, &QPushButton::pressed, this, &TaskResultsWidget::refreshCurrentResult);

    top_layout->addWidget(refresh_result_button_);

    remove_result_button_ = new QPushButton();
    remove_result_button_->setIcon(Files::IconProvider::getIcon("delete.png"));
    remove_result_button_->setEnabled(false);
    remove_result_button_->setFlat(true);
    remove_result_button_->setToolTip("Remove Result");

    connect(remove_result_button_, &QPushButton::pressed, this, &TaskResultsWidget::removeCurrentResult);

    top_layout->addWidget(remove_result_button_);

    export_result_button_ = new QPushButton();
    export_result_button_->setIcon(Files::IconProvider::getIcon("right.png"));
    export_result_button_->setEnabled(false);
    export_result_button_->setFlat(true);
    export_result_button_->setToolTip("Export Result");

    connect(export_result_button_, &QPushButton::pressed, this, &TaskResultsWidget::exportCurrentResult);

    top_layout->addWidget(export_result_button_);

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
    connect (&task_man, &TaskManager::taskResultHeaderChangedSignal,
            this, &TaskResultsWidget::resultHeaderChanged);
}

/**
 */
TaskResultsWidget::~TaskResultsWidget() {}

/**
 */
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
    report_widget_->setReport(task_man_.result(name)->report());

    if (!task_man_.result(name)->startSection().empty())
        report_widget_->selectId(task_man_.result(name)->startSection());

    report_widget_->setDisabled(false);

    updateResultUI(name);
}

/**
 */
void TaskResultsWidget::updateResultsSlot()
{
    updateResults();
}

/**
 */
void TaskResultsWidget::updateResults(const std::string& selected_result)
{
    loginf << "TaskResultsWidget: updateResultsSlot";

    current_report_name_backup_  = current_report_name_;
    current_section_name_backup_ = report_widget_->currentSection();

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
    export_result_button_->setEnabled(report_combo_->count() > 0);

    report_combo_->blockSignals(false);

    loginf << "TaskResultsWidget: updateResultsSlot: setReport";
    setReport(current_report_name_);
}

/**
 */
void TaskResultsWidget::resultHeaderChanged(const QString& name)
{
    updateResultUI(name.toStdString());
}

/**
 */
void TaskResultsWidget::updateResultUI(const std::string& name)
{
    if (current_report_name_ != name)
        return;

    auto result = task_man_.result(name);
    assert(result);

    bool update_needed = result->updateNeeded();
    bool locked        = result->isLocked();

    std::string icon_file;
    QColor      icon_color;
    QString     icon_tooltip;
    
    if (locked)
    {
        icon_file    = "refresh.png";
        icon_color   = ResultReport::Colors::TextRed;
        icon_tooltip = "Result in read-only mode.\nRefresh to unlock.";
    }
    else if(update_needed)
    {
        icon_file    = "refresh.png";
        icon_color   = ResultReport::Colors::TextOrange;
        icon_tooltip = "Refresh needed";
    }
    else
    {
        icon_file    = "refresh.png";
        icon_tooltip = "Up-to-date";
    }

    refresh_result_button_->setEnabled(update_needed);
    refresh_result_button_->setIcon(Utils::Files::getIcon(icon_file, icon_color));
    refresh_result_button_->setToolTip(icon_tooltip);
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
void TaskResultsWidget::exportCurrentResult()
{
    auto name = report_combo_->currentText().toStdString();
    assert(task_man_.hasResult(name));

    auto dir = QString::fromStdString(COMPASS::instance().lastUsedPath());

    auto fn = QFileDialog::getSaveFileName(this, "Select Filename", dir, "*.tex");
    if  (fn.isEmpty())
        return;

    auto res = task_man_.result(name)->exportResult(fn.toStdString(), ResultReport::ReportExportMode::LatexPDF);
    if (!res.ok())
    {
        logerr << "TaskResultsWidget: exportCurrentResult: Exporting result failed: " << res.error();
        QMessageBox::critical(this, "Error", "Exporting report failed.");
    }
}

/**
 */
void TaskResultsWidget::refreshCurrentResult()
{
    auto name = report_combo_->currentText().toStdString();
    assert(task_man_.hasResult(name));

    auto res = task_man_.result(name)->update(true);
    if (!res.ok())
    {
        logerr << "TaskResultsWidget: refreshCurrentResult: failed: " << res.error();
        QMessageBox::critical(this, "Error", "Refreshing result failed.");
    }
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
void TaskResultsWidget::selectID(const std::string id, bool show_figure)
{
    report_widget_->selectId(id, show_figure);
}

/**
 */
void TaskResultsWidget::restoreBackupSection()
{
    loginf << "TaskResultsWidget: restoreBackupSection: trying to restore section";

    if (current_report_name_backup_.empty())
        return;

    if (current_report_name_backup_ != current_report_name_)
        setReport(current_report_name_backup_);

    if (current_report_name_backup_ != current_report_name_)
        return;

    loginf << "TaskResultsWidget: restoreBackupSection: restored report '" << current_report_name_backup_ << "'";

    if (current_section_name_backup_.empty())
        return;

    report_widget_->selectId(current_section_name_backup_);

    loginf << "TaskResultsWidget: restoreBackupSection: restored section '" << current_section_name_backup_ << "'";
}
