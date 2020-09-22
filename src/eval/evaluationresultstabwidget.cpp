#include "evaluationresultstabwidget.h"

#include "evaluationmanagerwidget.h"
#include "evaluationmanager.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

EvaluationResultsTabWidget::EvaluationResultsTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget)
    : QWidget(nullptr), eval_man_(eval_man), man_widget_(man_widget)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);
}
