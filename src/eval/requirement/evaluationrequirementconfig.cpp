#include "evaluationrequirementconfig.h"
#include "evaluationrequirementgroup.h"
#include "logger.h"

#include <QFormLayout>
#include <QLineEdit>

using namespace std;

EvaluationRequirementConfig::EvaluationRequirementConfig(
        const std::string& class_id, const std::string& instance_id,
        EvaluationRequirementGroup& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : Configurable(class_id, instance_id, &group), EvaluationStandardTreeItem(&group),
      group_(group), standard_(standard), eval_man_(eval_man)
{
    registerParameter("name", &name_, "");
    registerParameter("short_name", &short_name_, "");

    assert (name_.size());
    assert (short_name_.size());

    createSubConfigurables();
}

EvaluationRequirementConfig::~EvaluationRequirementConfig()
{

}


void EvaluationRequirementConfig::generateSubConfigurable(const std::string& class_id,
                                                          const std::string& instance_id)
{
    assert(false);
}

std::string EvaluationRequirementConfig::name() const
{
    return name_;
}

void EvaluationRequirementConfig::checkSubConfigurables()
{
}

void EvaluationRequirementConfig::addGUIElements(QFormLayout* layout)
{
    assert (layout);

    QLineEdit* name_edit = new QLineEdit (name_.c_str());
    connect(name_edit, &QLineEdit::textEdited, this, &EvaluationRequirementConfig::changedNameSlot);

    layout->addRow("Name", name_edit);

    QLineEdit* short_name_edit = new QLineEdit (short_name_.c_str());
    connect(short_name_edit, &QLineEdit::textEdited, this, &EvaluationRequirementConfig::changedShortNameSlot);

    layout->addRow("Short Name", short_name_edit);
}

EvaluationStandardTreeItem* EvaluationRequirementConfig::child(int row)
{
    return nullptr;
}

int EvaluationRequirementConfig::childCount() const
{
    return 0;
}

int EvaluationRequirementConfig::columnCount() const
{
    return 1;
}

QVariant EvaluationRequirementConfig::data(int column) const
{
    assert (column == 0);

    return name_.c_str();
}

int EvaluationRequirementConfig::row() const
{
    return 0;
}

void EvaluationRequirementConfig::changedNameSlot(const QString& value)
{
    string value_str = value.toStdString();

    loginf << "EvaluationRequirementConfig: changedNameSlot: name '" << value_str << "'";
}

void EvaluationRequirementConfig::changedShortNameSlot(const QString& value)
{
    string value_str = value.toStdString();

    loginf << "EvaluationRequirementConfig: changedShortNameSlot: name '" << value_str << "'";
}

void EvaluationRequirementConfig::name(const std::string& name)
{
    name_ = name;
}

void EvaluationRequirementConfig::shortName(const std::string& short_name)
{
    short_name_ = short_name;
}

std::string EvaluationRequirementConfig::shortName() const
{
    return short_name_;
}
