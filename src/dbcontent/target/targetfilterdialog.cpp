#include "dbcontent/target/targetfilterdialog.h"
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

namespace dbContent {

TargetFilterDialog::TargetFilterDialog(TargetModel& model, QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), model_(model)
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
    remove_short_check_->setChecked(model_.removeShortTargets());
    connect(remove_short_check_, &QCheckBox::clicked, this,
            &TargetFilterDialog::removeShortTargetsSlot);

    config_layout->addRow("Remove Short Targets", remove_short_check_);

    remove_st_min_updates_edit_ = new QLineEdit();
    remove_st_min_updates_edit_->setText(QString::number(model_.removeShortTargetsMinUpdates()));
    connect(remove_st_min_updates_edit_, &QLineEdit::editingFinished,
            this, &TargetFilterDialog::removeSTMinUpdatesEditedSlot);
    config_layout->addRow(tr("\tShort Targets Minimum Updates [1]"), remove_st_min_updates_edit_);

    remove_st_min_duration_edit_ = new QLineEdit();
    remove_st_min_duration_edit_->setText(QString::number(model_.removeShortTargetsMinDuration()));
    connect(remove_st_min_duration_edit_, &QLineEdit::editingFinished,
            this, &TargetFilterDialog::removeSTMinDurationEditedSlot);
    config_layout->addRow(tr("\tShort Targets Duration [s]"), remove_st_min_duration_edit_);

    // psr only
    remove_psr_only_targets_check_ = new QCheckBox();
    remove_psr_only_targets_check_->setChecked(model_.removePsrOnlyTargets());
    connect(remove_psr_only_targets_check_, &QCheckBox::clicked, this,
            &TargetFilterDialog::removePSROnlyTargetsSlot);

    config_layout->addRow("Remove Primary-Only Targets", remove_psr_only_targets_check_);

    // ma
    remove_mode_ac_only_check_ = new QCheckBox();
    remove_mode_ac_only_check_->setChecked(model_.removeModeACOnlys());
    connect(remove_mode_ac_only_check_, &QCheckBox::clicked, this,
            &TargetFilterDialog::removeModeACOnlyTargetsSlot);
    config_layout->addRow("Remove Mode A/C Code onlys", remove_mode_ac_only_check_);

    remove_mode_a_check_ = new QCheckBox();
    remove_mode_a_check_->setChecked(model_.filterModeACodes());
    connect(remove_mode_a_check_, &QCheckBox::clicked, this,
            &TargetFilterDialog::removeModeASlot);

    config_layout->addRow("Remove By Mode A Code", remove_mode_a_check_);

    remove_mode_a_blacklist_check_ = new QCheckBox();
    remove_mode_a_blacklist_check_->setChecked(model_.filterModeACodeBlacklist());
    remove_mode_a_blacklist_check_->setToolTip("If checked, values are a blacklist - if not checked, a whitelist");
    connect(remove_mode_a_blacklist_check_, &QCheckBox::clicked,
            this, &TargetFilterDialog::removeModeABlackListSlot);

    config_layout->addRow("Mode A Code Blacklist", remove_mode_a_blacklist_check_);


    remove_mode_a_edit_ = new QTextEdit();
    remove_mode_a_edit_->setText(model_.filterModeACodeValues().c_str());
    connect(remove_mode_a_edit_, &QTextEdit::textChanged, this,
            &TargetFilterDialog::removeModeAValuesSlot);

    config_layout->addRow("\tMode A Code (oct,oct1-oct2)", remove_mode_a_edit_);

    // mc

    remove_mode_c_check_ = new QCheckBox();
    remove_mode_c_check_->setChecked(model_.removeModeCValues());
    connect(remove_mode_c_check_, &QCheckBox::clicked, this,
            &TargetFilterDialog::removeModeCSlot);

    config_layout->addRow("Remove By Mode C Code", remove_mode_c_check_);

    remove_mode_c_min_edit_ = new QTextEdit();
    remove_mode_c_min_edit_->setText(QString::number(model_.removeModeCMinValue()));
    connect(remove_mode_c_min_edit_, &QTextEdit::textChanged, this,
            &TargetFilterDialog::removeModeCMinValueSlot);

    config_layout->addRow("\tMode Min Value [ft]", remove_mode_c_min_edit_);

    // ta
    remove_ta_check_ = new QCheckBox();
    remove_ta_check_->setChecked(model_.filterTargetAddresses());
    connect(remove_ta_check_, &QCheckBox::clicked, this,
            &TargetFilterDialog::removeTASlot);

    config_layout->addRow("Remove By Target Address", remove_ta_check_);

    remove_ta_blacklist_check_ = new QCheckBox();
    remove_ta_blacklist_check_->setChecked(model_.filterTargetAddressesBlacklist());
    remove_ta_blacklist_check_->setToolTip("If checked, values are a blacklist - if not checked, a whitelist");
    connect(remove_ta_blacklist_check_, &QCheckBox::clicked,
            this, &TargetFilterDialog::removeTABlackListSlot);

    config_layout->addRow("Target Addresses Blacklist", remove_ta_blacklist_check_);

    remove_ta_edit_ = new QTextEdit();
    remove_ta_edit_->setText(model_.filterTargetAddressValues().c_str());
    connect(remove_ta_edit_, &QTextEdit::textChanged, this,
            &TargetFilterDialog::removeTAValuesSlot);

    config_layout->addRow("\tTarget Addresses (hex)", remove_ta_edit_);

    // dbos
    remove_dbo_check_ = new QCheckBox();
    remove_dbo_check_->setChecked(model_.removeNotDetectedDBContents());
    connect(remove_dbo_check_, &QCheckBox::clicked, this,
            &TargetFilterDialog::removeDBContentsSlot);

    config_layout->addRow("Remove By Non-Detection in DBContent", remove_dbo_check_);

    for (auto& dbcont_it : COMPASS::instance().dbContentManager())
    {
        if (dbcont_it.second->isStatusContent())
            continue;

        QCheckBox* tmp = new QCheckBox();
        tmp->setChecked(model_.removeNotDetectedDBContent(dbcont_it.first));
        tmp->setProperty("dbcontent_name", dbcont_it.first.c_str());
        connect(tmp, &QCheckBox::clicked, this,
                &TargetFilterDialog::removeSpecificDBContentsSlot);

        config_layout->addRow(("\tNon-Detection of "+dbcont_it.first).c_str(), tmp);
    }

    main_layout->addLayout(config_layout);

    main_layout->addStretch();

    // buttons

    QHBoxLayout* button_layout = new QHBoxLayout();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &TargetFilterDialog::cancelSlot);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    run_button_ = new QPushButton("Run");
    connect(run_button_, &QPushButton::clicked, this, &TargetFilterDialog::runSlot);
    button_layout->addWidget(run_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);
}

void TargetFilterDialog::removeShortTargetsSlot(bool checked)
{
    model_.removeShortTargets(checked);
}

void TargetFilterDialog::removeSTMinUpdatesEditedSlot()
{
    assert (remove_st_min_updates_edit_);
    QString text = remove_st_min_updates_edit_->text();

    bool ok;
    unsigned int value = text.toUInt(&ok);

    if (ok)
        model_.removeShortTargetsMinUpdates(value);
    else
        logwrn << "TargetFilterDialog: removeSTMinUpdatesEditedSlot: conversion of text '"
               << text.toStdString() << "' failed";
}

void TargetFilterDialog::removeSTMinDurationEditedSlot()
{
    assert (remove_st_min_duration_edit_);
    QString text = remove_st_min_duration_edit_->text();

    bool ok;
    float value = text.toFloat(&ok);

    if (ok)
        model_.removeShortTargetsMinDuration(value);
    else
        logwrn << "TargetFilterDialog: removeSTMinDurationEditedSlot: conversion of text '"
               << text.toStdString() << "' failed";
}

void TargetFilterDialog::removePSROnlyTargetsSlot(bool checked)
{
    model_.removePsrOnlyTargets(checked);
}

void TargetFilterDialog::removeModeACOnlyTargetsSlot(bool checked)
{
    model_.removeModeACOnlys(checked);
}

void TargetFilterDialog::removeModeASlot(bool checked)
{
    model_.filterModeACodes(checked);
}

void TargetFilterDialog::removeModeABlackListSlot(bool checked)
{
    model_.filterModeACodeBlacklist(checked);
}

void TargetFilterDialog::removeModeAValuesSlot()
{
    assert (remove_mode_a_edit_);
    model_.filterModeACodeValues(remove_mode_a_edit_->document()->toPlainText().toStdString());
}

void TargetFilterDialog::removeModeCSlot(bool checked)
{
    model_.removeModeCValues(checked);
}

void TargetFilterDialog::removeModeCMinValueSlot()
{
    assert (remove_mode_c_min_edit_);
    model_.removeModeCMinValue(remove_mode_c_min_edit_->document()->toPlainText().toFloat());
}

void TargetFilterDialog::removeTASlot(bool checked)
{
    model_.filterTargetAddresses(checked);
}

void TargetFilterDialog::removeTABlackListSlot(bool checked)
{
    model_.filterTargetAddressesBlacklist(checked);
}

void TargetFilterDialog::removeTAValuesSlot()
{
    assert (remove_ta_edit_);
    model_.filterTargetAddressValues(remove_ta_edit_->document()->toPlainText().toStdString());
}

void TargetFilterDialog::removeDBContentsSlot(bool checked)
{
    model_.removeNotDetectedDBContents(checked);
}

void TargetFilterDialog::removeSpecificDBContentsSlot(bool checked)
{
    QCheckBox* tmp = dynamic_cast<QCheckBox*>(sender());
    assert (tmp);

    QVariant data = tmp->property("dbcontent_name");
    assert (data.isValid());

    string dbcontent_name = data.toString().toStdString();

    model_.removeNotDetectedDBContents(dbcontent_name,checked);
}

void TargetFilterDialog::runSlot()
{
    model_.setUseByFilter();

    close();
}

void TargetFilterDialog::cancelSlot()
{
    close();
}

}
