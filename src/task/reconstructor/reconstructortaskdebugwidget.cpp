#include "reconstructortaskdebugwidget.h"
#include "reconstructortask.h"
#include "stringconv.h"
#include "timeconv.h"
#include "logger.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>

using namespace std;
using namespace Utils;


ReconstructorTaskDebugWidget::ReconstructorTaskDebugWidget(ReconstructorTask& task, QWidget *parent)
    : QWidget{parent}, task_(task)
{
    QFormLayout* combo_layout = new QFormLayout;
    //combo_layout->setMargin(0);
    combo_layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);

    utns_edit_ = new QLineEdit();
    connect(utns_edit_, &QLineEdit::textEdited, this, &ReconstructorTaskDebugWidget::utnsChangedSlot);
    combo_layout->addRow("UTNs", utns_edit_);

    rec_nums_edit_ = new QLineEdit();
    connect(rec_nums_edit_, &QLineEdit::textEdited, this, &ReconstructorTaskDebugWidget::recNumsChangedSlot);
    combo_layout->addRow("Record Numbers", rec_nums_edit_);

    timestamp_min_edit_ = new QLineEdit();
    connect(timestamp_min_edit_, &QLineEdit::textEdited, this, &ReconstructorTaskDebugWidget::timestampsChanged);
    combo_layout->addRow("Timestamp Min.", timestamp_min_edit_);

    timestamp_max_edit_ = new QLineEdit();
    connect(timestamp_max_edit_, &QLineEdit::textEdited, this, &ReconstructorTaskDebugWidget::timestampsChanged);
    combo_layout->addRow("Timestamp Max.", timestamp_max_edit_);

    setLayout(combo_layout);

    updateValues();
}

ReconstructorTaskDebugWidget::~ReconstructorTaskDebugWidget()
{
}

void ReconstructorTaskDebugWidget::updateValues()
{
    loginf << "ReconstructorTaskDebugWidget: updateValues";

    assert (utns_edit_);
    utns_edit_->setText(String::compress(task_.debugUTNs(), ',').c_str());

    assert (rec_nums_edit_);
    rec_nums_edit_->setText(String::compress(task_.debugRecNums(), ',').c_str());

    assert (timestamp_min_edit_);
    if (!task_.debugTimestampMin().is_not_a_date_time())
        timestamp_min_edit_->setText(QString::fromStdString(Utils::Time::toString(task_.debugTimestampMin())));
    else
        timestamp_min_edit_->setText("");

    assert (timestamp_max_edit_);
    if (!task_.debugTimestampMax().is_not_a_date_time())
        timestamp_max_edit_->setText(QString::fromStdString(Utils::Time::toString(task_.debugTimestampMax())));
    else
        timestamp_max_edit_->setText("");
}

void ReconstructorTaskDebugWidget::utnsChangedSlot(const QString& value)
{
    loginf << "ReconstructorTaskDebugWidget: utnsChangedSlot: value '" << value.toStdString() << "'";

    set<unsigned int> values_tmp;
    vector<string> split_str = String::split(value.toStdString(), ',');

    bool ok;

    for (auto& tmp_str : split_str)
    {
        unsigned int utn_tmp = QString(tmp_str.c_str()).toInt(&ok, 10);

        if (!ok)
        {
            logerr << "ReconstructorTaskDebugWidget: utnsChangedSlot: utn '" << tmp_str << "' not valid";
            break;
        }

        values_tmp.insert(utn_tmp);
    }

    task_.debugUTNs(values_tmp);
}

void ReconstructorTaskDebugWidget::recNumsChangedSlot(const QString& value)
{
    loginf << "ReconstructorTaskDebugWidget: recNumsChangedSlot: value '" << value.toStdString() << "'";

    set<unsigned long> values_tmp;
    vector<string> split_str = String::split(value.toStdString(), ',');

    bool ok;

    for (auto& tmp_str : split_str)
    {
        unsigned long utn_tmp = QString(tmp_str.c_str()).toInt(&ok, 10);

        if (!ok)
        {
            logerr << "ReconstructorTaskDebugWidget: utnsChangedSlot: utn '" << tmp_str << "' not valid";
            break;
        }

        values_tmp.insert(utn_tmp);
    }

    task_.debugRecNums(values_tmp);
}

void ReconstructorTaskDebugWidget::timestampsChanged()
{
    auto checkTimestamp = [ & ] (QLineEdit* line_edit)
    {
        auto txt = line_edit->text().toStdString();
        auto ts  = Utils::Time::fromString(txt);

        bool ts_ok = !ts.is_not_a_date_time();

        boost::optional<boost::posix_time::ptime> ret;

        if (ts_ok)
            ret = ts;

        line_edit->setStyleSheet(ts_ok ? "" : "color: red");

        return ret;
    };

    auto ts_min = checkTimestamp(timestamp_min_edit_);
    task_.debugTimestampMin(ts_min.has_value() ? ts_min.value() : boost::posix_time::ptime());

    if (ts_min.has_value())
        loginf << "ReconstructorTaskDebugWidget: timestampsChanged: set ts min to " << Utils::Time::toString(ts_min.value());

    auto ts_max = checkTimestamp(timestamp_max_edit_);
    task_.debugTimestampMax(ts_max.has_value() ? ts_max.value() : boost::posix_time::ptime());

    if (ts_max.has_value())
        loginf << "ReconstructorTaskDebugWidget: timestampsChanged: set ts max to " << Utils::Time::toString(ts_max.value());
}
