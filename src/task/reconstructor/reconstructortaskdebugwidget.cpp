#include "reconstructortaskdebugwidget.h"
#include "reconstructortask.h"
#include "stringconv.h"
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
