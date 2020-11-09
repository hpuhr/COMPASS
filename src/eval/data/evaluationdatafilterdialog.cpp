#include "evaluationdatafilterdialog.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QFormLayout>

EvaluationDataFilterDialog::EvaluationDataFilterDialog(EvaluationData& eval_data, EvaluationManager& eval_man,
                                                       QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), eval_data_(eval_data), eval_man_(eval_man)
{
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setWindowTitle("Filter UTNs");

    setModal(true);

    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    // config stuff
    QFormLayout* config_layout = new QFormLayout();

    {
        remove_short_check_ = new QCheckBox();
        remove_short_check_->setChecked(eval_man_.removeShortTargets());
        connect(remove_short_check_, &QCheckBox::clicked, this,
                &EvaluationDataFilterDialog::removeShortTargetsSlot);

        config_layout->addRow("Remove Short Targets", remove_short_check_);
    }


    main_layout->addLayout(config_layout);

    main_layout->addStretch();

    // buttons

    QHBoxLayout* button_layout = new QHBoxLayout();

    run_button_ = new QPushButton("Run");
    connect(run_button_, &QPushButton::clicked, this, &EvaluationDataFilterDialog::runSlot);
    button_layout->addWidget(run_button_);

    button_layout->addStretch();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &EvaluationDataFilterDialog::cancelSlot);
    button_layout->addWidget(cancel_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);
}

void EvaluationDataFilterDialog::removeShortTargetsSlot(bool checked)
{

}

void EvaluationDataFilterDialog::runSlot()
{
    eval_data_.setUseByFilter();

    close();
}

void EvaluationDataFilterDialog::cancelSlot()
{
    close();
}
