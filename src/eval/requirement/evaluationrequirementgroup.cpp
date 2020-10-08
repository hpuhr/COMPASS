#include "evaluationrequirementgroup.h"
#include "evaluationstandard.h"
#include "evaluationrequirementdetectionconfig.h"
#include "logger.h"

#include <QInputDialog>
#include <QMessageBox>

#include <algorithm>

using namespace std;

EvaluationRequirementGroup::EvaluationRequirementGroup(const std::string& class_id, const std::string& instance_id,
                                                       EvaluationStandard& standard, EvaluationManager& eval_man)
    : Configurable(class_id, instance_id, &standard), EvaluationStandardTreeItem(&standard), standard_(standard),
      eval_man_(eval_man)
{
    registerParameter("name", &name_, "");

    assert (name_.size());

    createSubConfigurables();
}

EvaluationRequirementGroup::~EvaluationRequirementGroup()
{

}


void EvaluationRequirementGroup::generateSubConfigurable(const std::string& class_id,
                                                         const std::string& instance_id)
{
    if (class_id.compare("EvaluationRequirementDetectionConfig") == 0)
    {
        EvaluationRequirement::EvaluationRequirementDetectionConfig* config =
                new EvaluationRequirement::EvaluationRequirementDetectionConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::EvaluationRequirementConfig>(config));
    }
    else
        throw std::runtime_error("EvaluationStandard: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

std::string EvaluationRequirementGroup::name() const
{
    return name_;
}


bool EvaluationRequirementGroup::hasRequirementConfig (const std::string& name)
{
    auto iter = std::find_if(configs_.begin(), configs_.end(),
       [&name](const unique_ptr<EvaluationRequirement::EvaluationRequirementConfig>& x) { return x->name() == name;});

    return iter != configs_.end();
}

void EvaluationRequirementGroup::addRequirementConfig (const std::string& class_id, const std::string& name,
                                                       const std::string& short_name)
{
    loginf << "EvaluationRequirementGroup: addRequirementConfig: class_id " << class_id << " name " << name
           << " short_name " << short_name;

    assert (!hasRequirementConfig(name));

    standard_.beginModelReset();

    std::string instance = class_id + name + "0";

    Configuration& config = addNewSubConfiguration(class_id, instance);
    config.addParameterString("name", name);
    config.addParameterString("short_name", short_name);

    generateSubConfigurable(class_id, instance);

    sortConfigs();

    standard_.endModelReset();

    assert (hasRequirementConfig(name));

    emit configsChangedSignal();
}

EvaluationRequirement::EvaluationRequirementConfig& EvaluationRequirementGroup::requirementConfig (const std::string& name)
{
    assert (hasRequirementConfig(name));

    auto iter = std::find_if(configs_.begin(), configs_.end(),
                             [&name](const unique_ptr<EvaluationRequirement::EvaluationRequirementConfig>& x) { return x->name() == name;});

    return **iter;
}

void EvaluationRequirementGroup::removeRequirementConfig (const std::string& name)
{
    assert (hasRequirementConfig(name));

    auto iter = std::find_if(configs_.begin(), configs_.end(),
                             [&name](const unique_ptr<EvaluationRequirement::EvaluationRequirementConfig>& x) { return x->name() == name;});

    configs_.erase(iter);

    emit configsChangedSignal();
}

void EvaluationRequirementGroup::checkSubConfigurables()
{

}

EvaluationStandardTreeItem* EvaluationRequirementGroup::child(int row)
{
    if (row < 0 || row >= configs_.size())
        return nullptr;

    return configs_.at(row).get();
}

int EvaluationRequirementGroup::childCount() const
{
    return configs_.size();
}

int EvaluationRequirementGroup::columnCount() const
{
    return 1;
}

QVariant EvaluationRequirementGroup::data(int column) const
{
    assert (column == 0);

    return name_.c_str();
}

int EvaluationRequirementGroup::row() const
{
    return 0;
}

void EvaluationRequirementGroup::showMenu ()
{
    QMenu menu;

    // menu creation
    {
        QAction* del_action = menu.addAction("Delete Group");
        connect(del_action, &QAction::triggered, this, &EvaluationRequirementGroup::deleteGroupSlot);

        // requirements
        QMenu* req_menu = menu.addMenu("Add Requirement");;

        QAction* add_det_action = req_menu->addAction("Detection");
        add_det_action->setData("EvaluationRequirementDetectionConfig");
        connect(add_det_action, &QAction::triggered, this, &EvaluationRequirementGroup::addRequirementSlot);
    }

    menu.exec(QCursor::pos());
}

void EvaluationRequirementGroup::deleteGroupSlot()
{
    loginf << "EvaluationRequirementGroup " << name_ << ": deleteGroupSlot";

    standard_.removeGroup (name_);
}

void EvaluationRequirementGroup::addRequirementSlot()
{
    loginf << "EvaluationRequirementGroup " << name_ << ": addRequirementSlot";

    QAction* action = dynamic_cast<QAction*>(QObject::sender());
    assert (action);

    QVariant data = action->data();
    assert (data.isValid());

    string class_id = data.toString().toStdString();

    loginf << "EvaluationRequirementGroup " << name_ << ": addRequirementSlot: class_id " << class_id;

    bool ok;
    QString text =
            QInputDialog::getText(nullptr, tr("Requirement Name"),
                                  tr("Specify a (unique) requirement name:"), QLineEdit::Normal,
                                  "", &ok);

    std::string req_name;

    if (ok && !text.isEmpty())
    {
        req_name = text.toStdString();
        if (!req_name.size())
        {
            QMessageBox m_warning(QMessageBox::Warning, "Adding Requirement Failed",
                                  "Requirement has to have a non-empty name.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        if (hasRequirementConfig(req_name))
        {
            QMessageBox m_warning(QMessageBox::Warning, "Adding Requirement Failed",
                                  "Requirement with this name already exists.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }
    }

    std::string req_short_name;

    text =  QInputDialog::getText(nullptr, tr("Requirement Short Name"),
                                  tr("Specify a requirement short name:"), QLineEdit::Normal,
                                  "", &ok);

    if (ok && !text.isEmpty())
    {
        req_short_name = text.toStdString();
    }

    loginf << "EvaluationRequirementGroup " << name_ << ": addRequirementSlot: class_id " << class_id
           << " req_name '" << req_name << "' req_short_name '" << req_short_name << "'";

    addRequirementConfig(class_id, req_name, req_short_name);
}

void EvaluationRequirementGroup::sortConfigs()
{
    sort(configs_.begin(), configs_.end(),
         [](const unique_ptr<EvaluationRequirement::EvaluationRequirementConfig>&a, const unique_ptr<EvaluationRequirement::EvaluationRequirementConfig>& b) -> bool
    {
        return a->name() > b->name();
    });
}
