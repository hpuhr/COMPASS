#include "asterixjsonparsingschema.h"

ASTERIXJSONParsingSchema::ASTERIXJSONParsingSchema(const std::string& class_id, const std::string& instance_id,
                                                   Configurable* parent)
    : Configurable(class_id, instance_id, parent)
{
    registerParameter("name", &name_, "");

    assert(name_.size());

    createSubConfigurables();
}

//ASTERIXJSONParsingSchema& ASTERIXJSONParsingSchema::operator=(ASTERIXJSONParsingSchema&& other)
//{
//    name_ = other.name_;

//    parsers_ = std::move(other.parsers_);

//    other.configuration().updateParameterPointer("name", &name_);

//    //    widget_ = std::move(other.widget_);
//    //    if (widget_)
//    //        widget_->setParser(*this);
//    //    other.widget_ = nullptr;

//    return static_cast<ASTERIXJSONParsingSchema&>(Configurable::operator=(std::move(other)));
//}

void ASTERIXJSONParsingSchema::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "ASTERIXJSONParser")
    {
        Configuration& sub_config = configuration().getSubConfiguration(class_id, instance_id);

        unsigned int category;

        if (sub_config.hasParameterConfigValueUint("category"))
            category = sub_config.getParameterConfigValueUint("category");

        assert(parsers_.find(category) == parsers_.end());

        logdbg << "ASTERIXJSONParsingSchema: generateSubConfigurable: generating schema " << instance_id
               << " for cat  " << category;

        parsers_[category].reset(new ASTERIXJSONParser(class_id, instance_id, this));
    }
    else
        throw std::runtime_error("ASTERIXJSONParsingSchema: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

std::string ASTERIXJSONParsingSchema::name() const { return name_; }

void ASTERIXJSONParsingSchema::name(const std::string& name) { name_ = name; }

ASTERIXJSONParser& ASTERIXJSONParsingSchema::parser(unsigned int category)
{
    assert(hasObjectParser(category));
    return *parsers_.at(category).get();
}

void ASTERIXJSONParsingSchema::removeParser(unsigned int category)
{
    assert(hasObjectParser(category));
    parsers_.erase(category);
}

void ASTERIXJSONParsingSchema::updateMappings()
{
    for (auto& p_it : parsers_)
        p_it.second->updateMappings();
}
