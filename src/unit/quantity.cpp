#include "unit.h"
#include "quantity.h"


Quantity::Quantity(const std::string &class_id, const std::string &instance_id, Configurable *parent)
    : Configurable (class_id, instance_id, parent)
{
    createSubConfigurables();
}

Quantity::~Quantity()
{
    for (auto it : units_)
        delete it.second;
    units_.clear();
}

void Quantity::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id == "Unit")
    {
        Unit *unit = new Unit (class_id, instance_id, *this);
        assert (units_.find(unit->getInstanceId()) == units_.end());
        units_.insert (std::pair <std::string, Unit*> (unit->getInstanceId(), unit));

    }
    else
        throw std::runtime_error ("UnitManager: generateSubConfigurable: unknown class_id "+class_id );
}

void Quantity::addUnit (const std::string &name, double factor, const std::string &definition)
{
    Configuration &config = addNewSubConfiguration ("Unit", name);
    config.addParameterDouble ("factor", factor);
    config.addParameterString ("definition", definition);
    generateSubConfigurable("Unit", name);
}

double Quantity::getFactor (const std::string &unit_source, const std::string &unit_destination)
{
  assert (units_.find(unit_source) != units_.end());
  assert (units_.find(unit_destination) != units_.end());
  double factor = 1.0;
  factor /= units_.at(unit_source)->factor();
  factor *= units_.at(unit_destination)->factor();
  return factor;
}
