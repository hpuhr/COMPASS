#include "jsonparsingschema.h"
#include "jsonimportertask.h"

JSONParsingSchema::JSONParsingSchema(const std::string& class_id, const std::string& instance_id,
                                     JSONImporterTask& task)
    : Configurable(class_id, instance_id, &task), task_(&task)
{
    registerParameter("name", &name_, "");

    assert (name_.size());

    createSubConfigurables();
}

JSONParsingSchema& JSONParsingSchema::operator=(JSONParsingSchema&& other)
{

    name_ = other.name_;
    task_ = other.task_;

    mappings_ = std::move(other.mappings_);

    other.configuration().updateParameterPointer ("name", &name_);

//    widget_ = std::move(other.widget_);
//    if (widget_)
//        widget_->setParser(*this);
//    other.widget_ = nullptr;

    return static_cast<JSONParsingSchema&>(Configurable::operator=(std::move(other)));
}

void JSONParsingSchema::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id == "JSONObjectParser")
    {
        mappings_.emplace_back (class_id, instance_id, this);
    }
    else
        throw std::runtime_error ("JSONImporterTask: generateSubConfigurable: unknown class_id "+class_id );
}

std::string JSONParsingSchema::name() const
{
    return name_;
}

void JSONParsingSchema::name(const std::string &name)
{
    name_ = name;
}
