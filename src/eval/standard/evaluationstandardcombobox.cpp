#include "evaluationstandardcombobox.h"
#include "evaluationmanager.h"

EvaluationStandardComboBox::EvaluationStandardComboBox(EvaluationManager& eval_man, QWidget* parent)
    : QComboBox(parent), eval_man_(eval_man)
{
    updateStandards();

    connect(this, SIGNAL(activated(const QString&)), this, SIGNAL(changedStandardSignal(const QString&)));
}

EvaluationStandardComboBox::~EvaluationStandardComboBox()
{

}

void EvaluationStandardComboBox::setStandardName(const std::string& value)
{
    int index = findText(QString(value.c_str()));
    assert(index >= 0);
    setCurrentIndex(index);
}

void EvaluationStandardComboBox::updateStandards()
{
    clear();

    addItem("");

    for (auto std_it = eval_man_.standardsBegin(); std_it != eval_man_.standardsEnd(); ++std_it)
    {
        addItem(std_it->first.c_str());
    }

    setCurrentIndex(0);
}
