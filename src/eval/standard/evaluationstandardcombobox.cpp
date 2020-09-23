#include "evaluationstandardcombobox.h"
#include "evaluationmanager.h"
#include "logger.h"

using namespace std;

EvaluationStandardComboBox::EvaluationStandardComboBox(EvaluationManager& eval_man, QWidget* parent)
    : QComboBox(parent), eval_man_(eval_man)
{
    updateStandards();

    connect(this, &QComboBox::currentTextChanged,
            this, &EvaluationStandardComboBox::changedStandardSlot);
}

EvaluationStandardComboBox::~EvaluationStandardComboBox()
{

}

void EvaluationStandardComboBox::changedStandardSlot(const QString& standard_name)
{
    string std_name = standard_name.toStdString();

    loginf << "EvaluationStandardComboBox: changedStandardSlot: standard " << std_name;

    if (!eval_man_.hasCurrentStandard() || eval_man_.currentStandard() != std_name)
        eval_man_.currentStandard(std_name);
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
