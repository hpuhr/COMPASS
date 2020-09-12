#include "evaluationmanager.h"
#include "atsdb.h"

EvaluationManager::EvaluationManager(const std::string& class_id, const std::string& instance_id, ATSDB* atsdb)
    : Configurable(class_id, instance_id, atsdb, "eval.json"), atsdb_(*atsdb)
{
    createSubConfigurables();
}

void EvaluationManager::close()
{

}

EvaluationManager::~EvaluationManager()
{

}

void EvaluationManager::generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id)
{
    throw std::runtime_error("EvaluationManager: generateSubConfigurable: unknown class_id " +
                             class_id);
}

void EvaluationManager::checkSubConfigurables()
{

}
