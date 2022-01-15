#include "evaluationdatafilterdialog.h"
#include "evaluationdata.h"
#include "evaluationmanager.h"
#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QFormLayout>

using namespace std;

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

    // shorts
    remove_short_check_ = new QCheckBox();
    remove_short_check_->setChecked(eval_man_.removeShortTargets());
    connect(remove_short_check_, &QCheckBox::clicked, this,
            &EvaluationDataFilterDialog::removeShortTargetsSlot);

    config_layout->addRow("Remove Short Targets", remove_short_check_);

    remove_st_min_updates_edit_ = new QLineEdit();
    remove_st_min_updates_edit_->setText(QString::number(eval_man_.removeShortTargetsMinUpdates()));
    connect(remove_st_min_updates_edit_, &QLineEdit::editingFinished,
            this, &EvaluationDataFilterDialog::removeSTMinUpdatesEditedSlot);
    config_layout->addRow(tr("\tShort Targets Minimum Updates [1]"), remove_st_min_updates_edit_);

    remove_st_min_duration_edit_ = new QLineEdit();
    remove_st_min_duration_edit_->setText(QString::number(eval_man_.removeShortTargetsMinDuration()));
    connect(remove_st_min_duration_edit_, &QLineEdit::editingFinished,
            this, &EvaluationDataFilterDialog::removeSTMinDurationEditedSlot);
    config_layout->addRow(tr("\tShort Targets Duration [s]"), remove_st_min_duration_edit_);

    // psr only
    remove_psr_only_targets_check_ = new QCheckBox();
    remove_psr_only_targets_check_->setChecked(eval_man_.removePsrOnlyTargets());
    connect(remove_psr_only_targets_check_, &QCheckBox::clicked, this,
            &EvaluationDataFilterDialog::removePSROnlyTargetsSlot);

    config_layout->addRow("Remove Primary-Only Targets", remove_psr_only_targets_check_);

    // ma
    remove_mode_ac_only_check_ = new QCheckBox();
    remove_mode_ac_only_check_->setChecked(eval_man_.removeModeACOnlys());
    connect(remove_mode_ac_only_check_, &QCheckBox::clicked, this,
            &EvaluationDataFilterDialog::removeModeACOnlyTargetsSlot);
    config_layout->addRow("Remove Mode A/C Code onlys", remove_mode_ac_only_check_);

    remove_mode_a_check_ = new QCheckBox();
    remove_mode_a_check_->setChecked(eval_man_.removeModeACodes());
    connect(remove_mode_a_check_, &QCheckBox::clicked, this,
            &EvaluationDataFilterDialog::removeModeASlot);

    config_layout->addRow("Remove By Mode A Code", remove_mode_a_check_);

    remove_mode_a_edit_ = new QTextEdit();
    remove_mode_a_edit_->setText(eval_man_.removeModeACodeValues().c_str());
    connect(remove_mode_a_edit_, &QTextEdit::textChanged, this,
            &EvaluationDataFilterDialog::removeModeAValuesSlot);

    config_layout->addRow("\tMode A Code (oct,oct1-oct2)", remove_mode_a_edit_);

    // mc

    remove_mode_c_check_ = new QCheckBox();
    remove_mode_c_check_->setChecked(eval_man_.removeModeCValues());
    connect(remove_mode_c_check_, &QCheckBox::clicked, this,
            &EvaluationDataFilterDialog::removeModeCSlot);

    config_layout->addRow("Remove By Mode C Code", remove_mode_c_check_);

    remove_mode_c_min_edit_ = new QTextEdit();
    remove_mode_c_min_edit_->setText(QString::number(eval_man_.removeModeCMinValue()));
    connect(remove_mode_c_min_edit_, &QTextEdit::textChanged, this,
            &EvaluationDataFilterDialog::removeModeCMinValueSlot);

    config_layout->addRow("\tMode Min Value [ft]", remove_mode_c_min_edit_);

    // ta
    remove_ta_check_ = new QCheckBox();
    remove_ta_check_->setChecked(eval_man_.removeTargetAddresses());
    connect(remove_ta_check_, &QCheckBox::clicked, this,
            &EvaluationDataFilterDialog::removeTASlot);

    config_layout->addRow("Remove By Target Address", remove_ta_check_);

    remove_ta_edit_ = new QTextEdit();
    remove_ta_edit_->setText(eval_man_.removeTargetAddressValues().c_str());
    connect(remove_ta_edit_, &QTextEdit::textChanged, this,
            &EvaluationDataFilterDialog::removeTAValuesSlot);

    config_layout->addRow("\tTarget Adresses (hex)", remove_ta_edit_);

    // dbos
    remove_dbo_check_ = new QCheckBox();
    remove_dbo_check_->setChecked(eval_man_.removeNotDetectedDBOs());
    connect(remove_dbo_check_, &QCheckBox::clicked, this,
            &EvaluationDataFilterDialog::removeDBOsSlot);

    config_layout->addRow("Remove By Non-Detection of DBObject", remove_dbo_check_);

    for (auto& dbo_it : COMPASS::instance().objectManager())
    {
        QCheckBox* tmp = new QCheckBox();
        tmp->setChecked(eval_man_.removeNotDetectedDBO(dbo_it.first));
        tmp->setProperty("dbo_name", dbo_it.first.c_str());
        connect(tmp, &QCheckBox::clicked, this,
                &EvaluationDataFilterDialog::removeSpecificDBOsSlot);

        config_layout->addRow(("\tNon-Detection of "+dbo_it.first).c_str(), tmp);
    }

    main_layout->addLayout(config_layout);

    main_layout->addStretch();

    // buttons

    QHBoxLayout* button_layout = new QHBoxLayout();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &EvaluationDataFilterDialog::cancelSlot);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    run_button_ = new QPushButton("Run");
    connect(run_button_, &QPushButton::clicked, this, &EvaluationDataFilterDialog::runSlot);
    button_layout->addWidget(run_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);
}

void EvaluationDataFilterDialog::removeShortTargetsSlot(bool checked)
{
    eval_man_.removeShortTargets(checked);
}

void EvaluationDataFilterDialog::removeSTMinUpdatesEditedSlot()
{
    assert (remove_st_min_updates_edit_);
    QString text = remove_st_min_updates_edit_->text();

    bool ok;
    unsigned int value = text.toUInt(&ok);

    if (ok)
        eval_man_.removeShortTargetsMinUpdates(value);
    else
        logwrn << "EvaluationDataFilterDialog: removeSTMinUpdatesEditedSlot: conversion of text '"
               << text.toStdString() << "' failed";
}

void EvaluationDataFilterDialog::removeSTMinDurationEditedSlot()
{
    assert (remove_st_min_duration_edit_);
    QString text = remove_st_min_duration_edit_->text();

    bool ok;
    float value = text.toFloat(&ok);

    if (ok)
        eval_man_.removeShortTargetsMinDuration(value);
    else
        logwrn << "EvaluationDataFilterDialog: removeSTMinDurationEditedSlot: conversion of text '"
               << text.toStdString() << "' failed";
}

void EvaluationDataFilterDialog::removePSROnlyTargetsSlot(bool checked)
{
    eval_man_.removePsrOnlyTargets(checked);
}

void EvaluationDataFilterDialog::removeModeACOnlyTargetsSlot(bool checked)
{
    eval_man_.removeModeACOnlys(checked);
}

void EvaluationDataFilterDialog::removeModeASlot(bool checked)
{
    eval_man_.removeModeACodes(checked);
}
void EvaluationDataFilterDialog::removeModeAValuesSlot()
{
    assert (remove_mode_a_edit_);
    eval_man_.removeModeACodeValues(remove_mode_a_edit_->document()->toPlainText().toStdString());
}

void EvaluationDataFilterDialog::removeModeCSlot(bool checked)
{
    eval_man_.removeModeCValues(checked);
}

void EvaluationDataFilterDialog::removeModeCMinValueSlot()
{
    assert (remove_mode_c_min_edit_);
    eval_man_.removeModeCMinValue(remove_mode_c_min_edit_->document()->toPlainText().toFloat());
}

void EvaluationDataFilterDialog::removeTASlot(bool checked)
{
    eval_man_.removeTargetAddresses(checked);
}

void EvaluationDataFilterDialog::removeTAValuesSlot()
{
    assert (remove_ta_edit_);
    eval_man_.removeTargetAddressValues(remove_ta_edit_->document()->toPlainText().toStdString());
}

void EvaluationDataFilterDialog::removeDBOsSlot(bool checked)
{
    eval_man_.removeNotDetectedDBOs(checked);
}

void EvaluationDataFilterDialog::removeSpecificDBOsSlot(bool checked)
{
    QCheckBox* tmp = dynamic_cast<QCheckBox*>(sender());
    assert (tmp);

    QVariant data = tmp->property("dbo_name");
    assert (data.isValid());

    string dbo_name = data.toString().toStdString();

    eval_man_.removeNotDetectedDBOs(dbo_name,checked);
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
