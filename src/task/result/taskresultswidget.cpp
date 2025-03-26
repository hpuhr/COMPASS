#include "taskresultswidget.h"
#include "taskresult.h"
#include "reportwidget.h"
#include "files.h"
#include "logger.h"
#include "taskmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QSettings>
#include <QMenu>
#include <QWidgetAction>
#include <QComboBox>

using namespace Utils;

TaskResultsWidget::TaskResultsWidget(TaskManager& task_man)
    : ToolBoxWidget(nullptr), task_man_(task_man)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    report_combo_ = new QComboBox;

    connect(report_combo_, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            [ = ] (const QString& text){
                assert (task_man_.hasResult(text.toStdString()));
                setReport(text.toStdString());
            });

    report_combo_->setDisabled(true);

    main_layout->addWidget(report_combo_);

    QFrame *line = new QFrame;
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    // Add to a layout
    main_layout->addWidget(line);

    // connect(report_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
    //         [ = ] (int idx){
    //             setReport(report_combo_->itemText(idx).toStdString());
    // });

    report_widget_ = new ResultReport::ReportWidget(*this);

    main_layout->addWidget(report_widget_);

    report_widget_->setDisabled(true);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);

    connect (&task_man, &TaskManager::resultsChangedSignal,
            this, &TaskResultsWidget::updateResultsSlot);
}

TaskResultsWidget::~TaskResultsWidget() = default;

void TaskResultsWidget::setReport(const std::string name)
{
    loginf << "TaskResultsWidget: setReport: name '" << name << "'";

    current_report_name_ = name;

    int name_idx = report_combo_->findData(QVariant(current_report_name_.c_str()));

    if (name_idx < 0)
    {
        report_combo_->clear();
        current_report_name_ = "";

        report_widget_->clear();
        report_widget_->setDisabled(true);

        return;
    }

    if (report_combo_->currentIndex() == name_idx)
        return;

    report_combo_->setCurrentIndex(name_idx);

    assert (task_man_.hasResult(name));
    report_widget_->setReport(task_man_.getOrCreateResult(name)->report());

    report_widget_->setDisabled(false);
}

void TaskResultsWidget::updateResultsSlot()
{
    loginf << "TaskResultsWidget::updateResultsSlot";

    report_combo_->clear();

    for (auto& res_it : task_man_.results())
    {
        report_combo_->addItem(res_it.second->name().c_str());

        if (!current_report_name_.size()) // set to first if not set
            current_report_name_ = res_it.second->name().c_str();
    }

    if (!report_combo_->count())
    {
        current_report_name_ = "";
        report_combo_->setDisabled(true);
    }

    setReport(current_report_name_);
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

std::string TaskResultsWidget::currentReportName() const
{
    return current_report_name_;
}
