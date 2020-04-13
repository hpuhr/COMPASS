#include "projection.h"

#include "logger.h"
#include "projectionmanager.h"

Projection::Projection(const std::string& class_id, const std::string& instance_id,
                       ProjectionManager& proj_manager)
    : Configurable(class_id, instance_id, &proj_manager), proj_manager_(proj_manager)
{
    registerParameter("name", &name_, "");

    assert(name_.size());

    // createSubConfigurables called in subclasses
}

Projection::~Projection() {}

void Projection::generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id)
{
}

std::string Projection::name() const { return name_; }

void Projection::name(const std::string& name) { name_ = name; }

void Projection::checkSubConfigurables() {}
