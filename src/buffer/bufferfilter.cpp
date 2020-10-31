/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "BufferFilter.h"

#include "Buffer.h"
#include "DBOVariable.h"

/************************************************************************************
BufferFilterEntry
*************************************************************************************/

/**
Constructor
@param dbo_type DBO type the entry is assigned to.
  */
// BufferFilterEntry::BufferFilterEntry( const std::string &dbo_type )
//:   dbo_type_( dbo_type )
//{
//}

/**
Configurable constructor.
Don't forget to set initial values for the registered parameters through the Configuration ( use
getConfig() for convenience ).
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
  */
BufferFilterEntry::BufferFilterEntry(const std::string& class_id, const std::string& instance_id,
                                     Configurable* parent)
    : Configurable(class_id, instance_id, parent)
{
    registerParameter("dbo_type", &dbo_type_, "");
    registerParameter("id", &id_, "");
    registerParameter("data_type_str", &data_type_str_, "");

    data_type_ = Property::asDataType(data_type_str_);
}

/**
Destructor.
  */
BufferFilterEntry::~BufferFilterEntry() {}

/**
Sets the entries Property.
@param prop The new Property.
  */
void BufferFilterEntry::setProperty(const Property& prop)
{
    id_ = prop.getId();
    data_type_ = prop.getDataType();
    data_type_str_ = prop.asDataTypeString();
}

/**
Sets the entries Property.
@param id The new Property id.
@param data_type The new Property data type given as integer.
  */
void BufferFilterEntry::setProperty(const std::string& id, PropertyDataType data_type)
{
    id_ = id;
    data_type_ = data_type;
    data_type_str_ = Property::asString(data_type_);
}

/**
Sets the entries Property using the given DBOVariable.
Will throw an exception if the variable is meta.
@param var DBOVariable the Property for the entry will be extracted from.
  */
void BufferFilterEntry::setProperty(DBOVariable* var)
{
    assert(var);

    // no meta variable allowed (which subvariable to choose?)
    if (var->isMetaVariable())
        throw std::runtime_error(
            "BufferFilterEntry: setProperty: Cannot set entry from meta variable.");

    id_ = var->getId();
    data_type_ = var->getDataType();
}

/**
Smaller operator (used for ordering).
@param other Object to compare to.
@return True if smaller otherwise false.
  */
bool BufferFilterEntry::operator<(const BufferFilterEntry& other) const
{
    return (id_.compare(other.getProperty().getId()) < 0);
}

/**
Checks if the entries Property exists in the given buffer. Note that the data type is directly
checked for equality.
@param buffer The buffer to be searched.
@return True if the entries property could be found in the given buffer, otherwise false.
  */
bool BufferFilterEntry::exists(Buffer* buffer) const
{
    assert(buffer);

    const PropertyList& props = buffer->properties();
    if (!props.hasProperty(id_))
        return false;

    if (props.get(id_).getDataType() != data_type_)
        return false;

    return true;
}

/**
Retrieves the Property from the entry.
  */
Property BufferFilterEntry::getProperty() const { return Property(id_, data_type_); }

/**
Retrieves a Configuration with the given data filled in. Use this method for your convenience when
generating a Configurable.
@param config The config the data is filled in.
@param dbo_type DBO type.
@param id Property id.
@param data_type Property data type as integer number.
  */
void BufferFilterEntry::getConfig(Configuration& config, const std::string& dbo_type,
                                  const std::string& id, PropertyDataType data_type)
{
    config.addParameterString("dbo_type", dbo_type);
    config.addParameterString("id", id);
    config.addParameterString("data_type_str", Property::asString(data_type));
}

/**
Returns the Property id of the entry as string.
@return Property id as string.
  */
const std::string& BufferFilterEntry::getID() const { return id_; }

/**
Returns the Property data type of the entry as integer number.
@return Property data type as integer number.
  */
PropertyDataType BufferFilterEntry::dataType() const { return data_type_; }

/**
Returns the DBO type the entry is assigned to as integer number.
@return DBO type as integer number.
  */
const std::string& BufferFilterEntry::dboType() const { return dbo_type_; }

/************************************************************************************
BufferFilterRuleEntry
*************************************************************************************/

/**
Constructor.
@param dbo_type DBO type as integer number.
@param filter_rule Filter rule as integer number (see class BufferFilter).
  */
// BufferFilterRuleEntry::BufferFilterRuleEntry( const std::string &dbo_type, int filter_rule )
//:   dbo_type_( dbo_type ),
//    filter_rule_( filter_rule )
//{
//}

/**
Configurable constructor.
Don't forget to set initial values for the registered parameters through the Configuration ( use
getConfig() for convenience ).
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
  */
BufferFilterRuleEntry::BufferFilterRuleEntry(const std::string& class_id,
                                             const std::string& instance_id, Configurable* parent)
    : Configurable(class_id, instance_id, parent)
{
    registerParameter("dbo_type", &dbo_type_, "");
    registerParameter("filter_rule", &filter_rule_, -1);
}

/**
Destructor.
  */
BufferFilterRuleEntry::~BufferFilterRuleEntry() {}

/**
Retrieves a Configuration with the given data filled in. Use this method for your convenience when
generating a Configurable.
@param config The config the data is filled in.
@param dbo_type DBO type.
@param filter_rule Filter rule as integer number (see class BufferFilter).
  */
void BufferFilterRuleEntry::getConfig(Configuration& config, const std::string& dbo_type,
                                      int filter_rule)
{
    config.addParameterString("dbo_type", dbo_type);
    config.addParameterInt("filter_rule", filter_rule);
}

/**
Sets the entries filter rule as integer number.
@param filter_rule Filter rule as integer number (see class BufferFilter).
  */
void BufferFilterRuleEntry::setRule(int filter_rule) { filter_rule_ = filter_rule; }

/**
Returns the DBO type the entry is assigned to.
@return DBO type as integer number.
  */
const std::string& BufferFilterRuleEntry::dboType() const { return dbo_type_; }

/**
Returns the entries filter rule as integer number.
@return filter_rule Filter rule as integer number (see class BufferFilter).
  */
int BufferFilterRuleEntry::filterRule() const { return filter_rule_; }

/************************************************************************************
BufferFilter
*************************************************************************************/

/**
Constructor.
  */
// BufferFilter::BufferFilter()
//:   base_rule_( NULL )
//{
//    // TODO FIXXMEE
//    assert (false);
//    //base_rule_ = new BufferFilterRuleEntry( DBO_UNDEFINED, BLOCK );
//}

/**
Configurable constructor.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
  */
BufferFilter::BufferFilter(const std::string& class_id, const std::string& instance_id,
                           Configurable* parent)
    : Configurable(class_id, instance_id, parent), base_rule_(NULL)
{
    createSubConfigurables();
}

/**
Destructor.
  */
BufferFilter::~BufferFilter() { clear(); }

/**
Adds a new filter Property to the filter.
@param dbo_type DBO type the Property is assigned to.
@param prop New filter Property.
  */
void BufferFilter::addPropertyToFilter(const std::string& dbo_type, const Property& prop)
{
    FilterEntries& entries = property_filter_[dbo_type];
    if (entries.find(prop.getId()) != entries.end())
        throw std::runtime_error("BufferFilter: addEntry: Duplicate entry id.");

    //    if( unusable_ )
    //    {
    //        BufferFilterEntry* entry = new BufferFilterEntry( dbo_type );
    //        entry->setProperty( prop );
    //        property_filter_[ dbo_type ][ prop.getId() ] = entry;
    //    }
    //    else
    //    {
    Configuration& config = addNewSubConfiguration("BufferFilterEntry");
    BufferFilterEntry::getConfig(config, dbo_type, prop.getId(), prop.getDataType());
    generateSubConfigurable(config.getClassId(), config.getInstanceId());
    //    }
}

/**
Adds a new filter Property to the filter. If the given id already exists for the given DBO type
an exception will be thrown.
@param dbo_type DBO type the Property is assigned to.
@param id Property id string.
@param data_type Property data type as integer number.
  */
void BufferFilter::addPropertyToFilter(const std::string& dbo_type, const std::string& id,
                                       PropertyDataType data_type)
{
    FilterEntries& entries = property_filter_[dbo_type];
    if (entries.find(id) != entries.end())
        throw std::runtime_error("BufferFilter: addEntry: Duplicate entry id.");

    //    if( unusable_ )
    //    {
    //        BufferFilterEntry* entry = new BufferFilterEntry( dbo_type );
    //        entry->setProperty( id, data_type );
    //        property_filter_[ dbo_type ][ id ] = entry;
    //    }
    //    else
    //    {
    Configuration& config = addNewSubConfiguration("BufferFilterEntry", "");
    BufferFilterEntry::getConfig(config, dbo_type, id, data_type);
    generateSubConfigurable(config.getClassId(), config.getInstanceId());
    //    }
}

/**
Adds one or more new filter properties to the filter using the given DBOVariable. If the variable is
of meta type all of the subvariables properties will be added to the filter. If the given Property
id already exists for the a specific DBO type an exception will be thrown.
@param var DBOVariable the properties should be added from.
  */
void BufferFilter::addPropertyToFilter(DBOVariable* var)
{
    assert(var);

    if (!var->isMetaVariable())
    {
        addPropertyToFilter(var->getDBOType(), *var);
        return;
    }

    // add subvariables
    const std::map<std::string, std::string>& subs = var->getSubVariables();
    std::map<std::string, std::string>::const_iterator it, itend = subs.end();
    for (it = subs.begin(); it != itend; ++it)
        addPropertyToFilter(it->first, it->second, var->getDataType());
}

/**
Checks if the given Property id already exists for the given DBO type.
@param dbo_type DBO type.
@param id String id.
@return True if a Property with the given id already exists for the given DBO type, otherwise false.
  */
bool BufferFilter::hasProperty(const std::string& dbo_type, const std::string& id)
{
    if (property_filter_.find(dbo_type) == property_filter_.end())
        return false;
    FilterEntries& entries = property_filter_[dbo_type];
    return (entries.find(id) != entries.end());
}

/**
Removes all defined filter properties.
  */
void BufferFilter::clearPropertyFilter()
{
    PropertyFilter::iterator itm, itmend = property_filter_.end();
    FilterEntries::iterator it, itend;
    for (itm = property_filter_.begin(); itm != itmend; ++itm)
    {
        FilterEntries& entries = itm->second;
        it = entries.begin();
        itend = entries.end();
        for (; it != itend; ++it)
            delete it->second;
    }

    property_filter_.clear();
}

/**
Returns all defined filter properties.
@return Defined filter properties.
  */
std::multimap<std::string, Property> BufferFilter::getProperties() const
{
    std::multimap<std::string, Property> props;

    PropertyFilter::const_iterator itm, itmend = property_filter_.end();
    FilterEntries::const_iterator it, itend;
    for (itm = property_filter_.begin(); itm != itmend; ++itm)
    {
        const FilterEntries& entries = itm->second;
        it = entries.begin();
        itend = entries.end();
        for (; it != itend; ++it)
        {
            props.insert(std::pair<std::string, Property>(itm->first, it->second->getProperty()));
        }
    }

    return props;
}

/**
Sets the filter rule for the given DBO type. Note that rules that already exist for the given DBO
type will just get overwritten by new ones.
@param dbo_type DBO type associated with the given filter rule.
@param rule Filter rule for buffers of the given DBO type.
  */
void BufferFilter::setRule(const std::string& dbo_type, BufferFilterRule rule)
{
    if (rules_.find(dbo_type) != rules_.end())
    {
        rules_[dbo_type]->setRule((int)rule);
        return;
    }

    //    if( unusable_ )
    //    {
    //        rules_[ dbo_type ] = new BufferFilterRuleEntry( dbo_type, (int)rule );
    //    }
    //    else
    //    {
    Configuration& config = addNewSubConfiguration("BufferFilterRuleEntry");
    BufferFilterRuleEntry::getConfig(config, dbo_type, (int)rule);
    generateSubConfigurable(config.getClassId(), config.getInstanceId());
    //    }
}

/**
Sets one or more filter rules from the given DBOVariable. If the variable is of meta type
the given rule will be added for all defined subvariable DBO types. Note that rules that already
exist for a given DBO type will just get overwritten by new ones.
@param var DBOVariable the filter rule should be added for.
@param rule Filter rule for the given DBOVariable.
  */
void BufferFilter::setRule(DBOVariable* var, BufferFilterRule rule)
{
    bool is_meta = var->isMetaVariable();

    if (!is_meta)
    {
        setRule(var->getDBOType(), rule);
        return;
    }

    // add subvariables
    const std::map<std::string, std::string>& subs = var->getSubVariables();
    std::map<std::string, std::string>::const_iterator it, itend = subs.end();
    for (it = subs.begin(); it != itend; ++it)
        setRule(it->first, rule);
}

/**
Sets the filters base rule, which will be used if no rule is defined for a specific buffers DBO
type.
@param rule Filter rule to be used as base rule.
  */
void BufferFilter::setBaseRule(BufferFilterRule rule)
{
    if (base_rule_)
    {
        base_rule_->setRule((int)rule);
        return;
    }

    // TODO FIXXMEE
    assert(false);
    //    if( unusable_ )
    //    {
    //        base_rule_ = new BufferFilterRuleEntry( DBO_UNDEFINED, (int)rule );
    //    }
    //    else
    //    {
    //        Configuration& config = addNewSubConfiguration( "BufferFilterRuleEntryBase" );
    //        BufferFilterRuleEntry::getConfig( config, (int)DBO_UNDEFINED, (int)rule );
    //        generateSubConfigurable( config.getClassId(), config.getInstanceId() );
    //    }
}

/**
Returns the filter rule assigned to the given Buffer. If no specific rule can be found
for the buffers DBO type, the base rule will be returned. Note that the rule will
be defaulted to BLOCK if no rule is present!
@param buffer Given Buffer.
@return Filter rule assigned to the given buffers DBO type.
  */
BufferFilter::BufferFilterRule BufferFilter::getRule(Buffer* buffer)
{
    assert(buffer);

    const std::string& dbo_type = buffer->dboType();
    if (rules_.find(dbo_type) != rules_.end())
        return (BufferFilterRule)(rules_[dbo_type]->filterRule());

    if (base_rule_)
        return (BufferFilterRule)(base_rule_->filterRule());

    // I'm mister restrictive!
    return BufferFilter::BLOCK;
}

/**
Returns all defined filter rules.
@return Defined filter rules.
  */
std::map<std::string, BufferFilter::BufferFilterRule> BufferFilter::getRules() const
{
    std::map<std::string, BufferFilter::BufferFilterRule> rules;

    Rules::const_iterator it, itend = rules_.end();
    for (it = rules_.begin(); it != itend; ++it)
        rules[it->first] = (BufferFilter::BufferFilterRule)it->second->filterRule();

    return rules;
}

/**
Checks if the defined filter properties are all present in the given buffer.
@param buffer Input buffer to be checked.
@return True if all properties defined for the buffers DBO type are present in
    the buffer (present meaning equal string id and integer data type), otherwise false.
  */
bool BufferFilter::checkProperties(Buffer* buffer)
{
    assert(buffer);

    const std::string& dbo_type = buffer->dboType();

    if (property_filter_.find(dbo_type) == property_filter_.end())
        return true;

    FilterEntries& entries = property_filter_[dbo_type];
    FilterEntries::iterator it, itend = entries.end();
    for (it = entries.begin(); it != itend; ++it)
    {
        if (!it->second->exists(buffer))
            return false;
    }

    return true;
}

/**
Returns all erroneous (non-present) properties for the given buffer in a printable
format. Returns an empty string if no filter rules are defined or bad.
@param buffer Given buffer.
@return Defined properties that are not present in the given buffer listed in a string.
  */
std::string BufferFilter::getErrorProperties(Buffer* buffer)
{
    assert(buffer);

    const std::string& dbo_type = buffer->dboType();
    std::string ret = "";

    if (property_filter_.find(dbo_type) == property_filter_.end())
        return ret;

    FilterEntries& entries = property_filter_[dbo_type];
    FilterEntries::iterator it, itend = entries.end();
    for (it = entries.begin(); it != itend; ++it)
    {
        if (!it->second->exists(buffer))
        {
            ret += "\n\t" + it->first;
        }
    }

    return ret;
}

/**
Removes all filter rules.
  */
void BufferFilter::clearRules()
{
    Rules::iterator it, itend = rules_.end();
    for (it = rules_.begin(); it != itend; ++it)
        delete it->second;
    rules_.clear();
}

/**
Removes the base filter rule.
  */
void BufferFilter::clearBaseRule()
{
    if (!base_rule_)
        return;
    delete base_rule_;
    base_rule_ = NULL;
}

/**
Clears the filter.
  */
void BufferFilter::clear()
{
    clearRules();
    clearBaseRule();
    clearPropertyFilter();
}

/**
 */
void BufferFilter::generateSubConfigurable(std::string class_id, std::string instance_id)
{
    // special tag for the base rule
    if (class_id == "BufferFilterRuleEntryBase")
    {
        if (base_rule_)
            delete base_rule_;
        base_rule_ = new BufferFilterRuleEntry(class_id, instance_id, this);
    }

    if (class_id == "BufferFilterRuleEntry")
    {
        BufferFilterRuleEntry* entry = new BufferFilterRuleEntry(class_id, instance_id, this);
        rules_[entry->dboType()] = entry;
    }

    if (class_id == "BufferFilterEntry")
    {
        BufferFilterEntry* entry = new BufferFilterEntry(class_id, instance_id, this);
        property_filter_[entry->dboType()][entry->getID()] = entry;
    }
}

/**
 */
void BufferFilter::checkSubConfigurables() {}

/**
Checks if a specific rule has already been added for the given DBO type.
@param type DBO type.
@return True if a specific filter rule exists for the given DBO type, otherwise false.
  */
bool BufferFilter::hasRule(const std::string& type) const
{
    return (rules_.find(type) != rules_.end());
}

/**
Checks if a base filter rule is defined.
@return True if a base filter rule is defined, false otherwise.
  */
bool BufferFilter::hasBaseRule() const { return base_rule_ != NULL; }

/**
Returns the base filter rule, which is used if no specific filter rule is defined for a certain
buffer.
@return The filters base filter rule.
  */
BufferFilter::BufferFilterRule BufferFilter::getBaseRule() const
{
    if (!base_rule_)
        throw std::runtime_error("BufferFilter: getBaseRule: No base rule set.");
    return (BufferFilter::BufferFilterRule)base_rule_->filterRule();
}

/**
Returns the specific rule for the given DBO type.
@param type DBO type.
@return Specific filter rule for the given DBO type.
  */
BufferFilter::BufferFilterRule BufferFilter::getRule(const std::string& type)
{
    if (!hasRule(type))
        throw std::runtime_error("BufferFilter: getRule: No rule for given DBO type.");
    return (BufferFilter::BufferFilterRule)rules_[type]->filterRule();
}
