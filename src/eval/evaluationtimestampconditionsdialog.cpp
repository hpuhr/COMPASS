#include "evaluationtimestampconditionsdialog.h"
#include "util/timeconv.h"
#include "timewindowcollectionwidget.h"
#include "evaluationmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QScrollArea>

using namespace Utils;

EvaluationTimestampConditionsDialog::EvaluationTimestampConditionsDialog(EvaluationManager& eval_man, QWidget* parent)
    : QDialog(parent), eval_man_(eval_man)
{
    QFormLayout* form_layout = new QFormLayout;
    form_layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    // time
    use_time_check_ = new QCheckBox ();
    connect(use_time_check_, &QCheckBox::clicked, this, &EvaluationTimestampConditionsDialog::toggleUseTimeSlot);
    form_layout->addRow("Use Timestamp Filter", use_time_check_);

    time_begin_edit_ = new QDateTimeEdit(QDateTime::currentDateTime());
    time_begin_edit_->setDisplayFormat(Time::QT_DATETIME_FORMAT.c_str());
    connect(time_begin_edit_, &QDateTimeEdit::dateTimeChanged, this, &EvaluationTimestampConditionsDialog::timeBeginEditedSlot);
    form_layout->addRow("Timestamp Begin", time_begin_edit_);

    time_end_edit_ = new QDateTimeEdit(QDateTime::currentDateTime());
    time_end_edit_->setDisplayFormat(Time::QT_DATETIME_FORMAT.c_str());
    connect(time_end_edit_, &QDateTimeEdit::dateTimeChanged, this, &EvaluationTimestampConditionsDialog::timeEndEditedSlot);
    form_layout->addRow("Timestamp End", time_end_edit_);

    tw_widget_ = new TimeWindowCollectionWidget(eval_man.excludedTimeWindows());
    form_layout->addRow("Exlcuded Time Windows", tw_widget_);

    setLayout(form_layout);
}

EvaluationTimestampConditionsDialog::~EvaluationTimestampConditionsDialog()
{
    tw_widget_ = nullptr;
}

void EvaluationTimestampConditionsDialog::updateValues()
{
    update_active_ = true;

    // time filter
    assert (use_time_check_);
    use_time_check_->setChecked(eval_man_.useTimestampFilter());

    assert (time_begin_edit_);
    //time_begin_edit_->setText(String::timeStringFromDouble(eval_man_.loadTimestampBegin()).c_str());
    time_begin_edit_->setDateTime(QDateTime::fromString(Time::toString(eval_man_.loadTimestampBegin()).c_str(),
                                                        Time::QT_DATETIME_FORMAT.c_str()));

    assert (time_end_edit_);
    //time_end_edit_->setText(String::timeStringFromDouble(eval_man_.loadTimestampEnd()).c_str());
    time_end_edit_->setDateTime(QDateTime::fromString(Time::toString(eval_man_.loadTimestampEnd()).c_str(),
                                                      Time::QT_DATETIME_FORMAT.c_str()));

    update_active_ = false;

}

/**
 */
void EvaluationTimestampConditionsDialog::toggleUseTimeSlot()
{
    assert (use_time_check_);
    eval_man_.useTimestampFilter(use_time_check_->checkState() == Qt::Checked);
}

/**
 */
void EvaluationTimestampConditionsDialog::timeBeginEditedSlot (const QDateTime& datetime)
{
    if (update_active_)
        return;

    loginf << "EvaluationTimestampConditionsDialog: timeBeginEditedSlot: value "
           << datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString();

    eval_man_.loadTimestampBegin(Time::fromString(datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString()));
}

/**
 */
void EvaluationTimestampConditionsDialog::timeEndEditedSlot (const QDateTime& datetime)
{
    if (update_active_)
        return;

    loginf << "EvaluationTimestampConditionsDialog: timeEndEditedSlot: value "
           << datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString();

    eval_man_.loadTimestampEnd(Time::fromString(datetime.toString(Time::QT_DATETIME_FORMAT.c_str()).toStdString()));
}
