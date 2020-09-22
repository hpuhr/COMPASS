#include "evaluationstandardwidget.h"
#include "evaluationstandard.h"

#include <QVBoxLayout>
#include <QLabel>

EvaluationStandardWidget::EvaluationStandardWidget(EvaluationStandard& standard)
    : QWidget(), standard_(standard)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    main_layout->addWidget(new QLabel(standard_.name().c_str()));

    setLayout(main_layout);
}
