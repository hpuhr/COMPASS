#include "evaluationresultstabwidget.h"

#include "evaluationmanagerwidget.h"
#include "evaluationmanager.h"
#include "evaluationresultsgenerator.h"
#include "eval/results/report/treemodel.h"
#include "eval/results/report/treeitem.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "files.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QSplitter>
#include <QSettings>

using namespace EvaluationResultsReport;
using namespace Utils;

EvaluationResultsTabWidget::EvaluationResultsTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget)
    : QWidget(nullptr), eval_man_(eval_man), man_widget_(man_widget)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    { // button layout
        QHBoxLayout* button_layout = new QHBoxLayout();

        QIcon left_icon(Files::getIconFilepath("arrow_to_left.png").c_str());

        back_button_ = new QPushButton ();
        back_button_->setIcon(left_icon);
        connect (back_button_, &QPushButton::clicked, this, &EvaluationResultsTabWidget::stepBackSlot);
        button_layout->addWidget(back_button_);

        button_layout->addStretch();

        main_layout->addLayout(button_layout);

    }
    //QHBoxLayout* res_layout = new QHBoxLayout();

    splitter_ = new QSplitter();
    splitter_->setOrientation(Qt::Horizontal);

    tree_view_.reset(new QTreeView());
    tree_view_->setModel(&eval_man_.resultsGenerator().resultsModel());
    tree_view_->setRootIsDecorated(false);
    tree_view_->expandToDepth(3);

    connect (tree_view_.get(), &QTreeView::clicked, this, &EvaluationResultsTabWidget::itemClickedSlot);

    splitter_->addWidget(tree_view_.get());

    // results stack

    QScrollArea* scroll_area = new QScrollArea();
    scroll_area->setWidgetResizable(true);

    results_widget_ = new QStackedWidget();

    scroll_area->setWidget(results_widget_);
    splitter_->addWidget(scroll_area);

    splitter_->setStretchFactor(1, 1);

    QSettings settings("ATSDB", "EvalManagerResultsWidget");
    splitter_->restoreState(settings.value("splitterSizes").toByteArray());

    main_layout->addWidget(splitter_);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);

    updateBackButton();
}

EvaluationResultsTabWidget::~EvaluationResultsTabWidget()
{
    assert (splitter_);

    QSettings settings("ATSDB", "EvalManagerResultsWidget");
    settings.setValue("splitterSizes", splitter_->saveState());
}

void EvaluationResultsTabWidget::expand()
{
    loginf << "EvaluationResultsTabWidget: expand";

    tree_view_->expandToDepth(3);
}

void EvaluationResultsTabWidget::selectId (const std::string& id)
{
    loginf << "EvaluationResultsTabWidget: selectId: id '" << id << "'";

    QModelIndex index = eval_man_.resultsGenerator().resultsModel().findItem(id);

    if (!index.isValid())
    {
        logerr << "EvaluationResultsTabWidget: selectId: id '" << id << "' not found";
        return;
    }

    assert (tree_view_);
    tree_view_->selectionModel()->clear();

    expandAllParents(index);

    tree_view_->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    tree_view_->scrollTo(index);
    itemClickedSlot(index);
}

void EvaluationResultsTabWidget::reshowLastId ()
{
    if (id_history_.size() >= 1)
    {
        selectId(*id_history_.rbegin()); // select last one

        id_history_.pop_back(); // remove re-added id. slightly hacky
    }
}


void EvaluationResultsTabWidget::itemClickedSlot(const QModelIndex& index)
{
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    assert (item);

    id_history_.push_back(item->id());

    loginf << "EvaluationResultsTabWidget: itemClickedSlot: name " << item->name();

    if (dynamic_cast<EvaluationResultsReport::RootItem*>(item))
    {
        loginf << "EvaluationResultsTabWidget: itemClickedSlot: root";
        showResultWidget(nullptr);
    }
    else if (dynamic_cast<EvaluationResultsReport::Section*>(item))
    {
        EvaluationResultsReport::Section* section = dynamic_cast<EvaluationResultsReport::Section*>(item);
        assert (section);

        loginf << "EvaluationResultsTabWidget: itemClickedSlot: section " << section->name();
        showResultWidget(section->getContentWidget());
    }

    updateBackButton();
}

void EvaluationResultsTabWidget::stepBackSlot()
{
    loginf << "EvaluationResultsTabWidget: stepBackSlot";

    assert (id_history_.size() > 1);

    id_history_.pop_back(); // remove last entry
    reshowLastId(); // show last id

    updateBackButton();
}

void EvaluationResultsTabWidget::showResultWidget(QWidget* widget)
{
    assert(results_widget_);

    if (!widget)
    {
        while (results_widget_->count() > 0)  // remove all widgets
            results_widget_->removeWidget(results_widget_->widget(0));
        return;
    }

    if (results_widget_->indexOf(widget) < 0)
        results_widget_->addWidget(widget);

    results_widget_->setCurrentWidget(widget);
}

void EvaluationResultsTabWidget::expandAllParents (QModelIndex index)
{
    if (!index.isValid()) {
        return;
    }

    QModelIndex parent_index;

    for (parent_index = index.parent(); parent_index.isValid(); parent_index = parent_index.parent())
    {
        if (!tree_view_->isExpanded(parent_index))
            tree_view_->expand(parent_index);
    }
}

void EvaluationResultsTabWidget::updateBackButton ()
{
    assert (back_button_);

    back_button_->setEnabled(id_history_.size() > 1);
}
