#include "propertylist.h"
#include "logger.h"

PropertyList::PropertyList()
{
    logdbg << "PropertyList: constructor";
}

PropertyList::~PropertyList()
{
    logdbg << "PropertyList: destructor: start";
    clear();
    logdbg << "PropertyList: destructor: end";
}

PropertyList::PropertyList(const PropertyList& org)
{
    properties_ = org.properties_;
    // loginf << "PropertyList: constructor: properties " << properties_.size();
}


void PropertyList::operator=(const PropertyList& org)
{
    properties_ = org.properties_;
}

void PropertyList::addPropertyList(const PropertyList& org)
{
    properties_.insert(properties_.end(), org.properties_.begin(), org.properties_.end());
}

void PropertyList::addProperty(std::string id, PropertyDataType type)
{
    logdbg << "PropertyList: addProperty: start";
    logdbg << "PropertyList: addProperty:  id '" << id << "' type " << Property::asString(type);
    assert(!id.empty());

    if (hasProperty(id))
    {
        logwrn << "PropertyList: addProperty: property " << id << " already added";
        return;
    }

    properties_.push_back(Property(id, type));
    logdbg << "PropertyList: addProperty: end";
}

void PropertyList::addProperty(Property& property)
{
    logdbg << "PropertyList: addProperty: start";

    if (hasProperty(property.name()))
    {
        logwrn << "PropertyList: addProperty: property " << property.name() << " already added";
        return;
    }

    properties_.push_back(property);
    logdbg << "PropertyList: addProperty: end";
}

void PropertyList::addProperty(const Property& property)
{
    logdbg << "PropertyList: addProperty: start";

    if (hasProperty(property.name()))
    {
        logwrn << "PropertyList: addProperty: property " << property.name() << " already added";
        return;
    }

    properties_.push_back(property);
    logdbg << "PropertyList: addProperty: end";
}

const Property& PropertyList::at(unsigned int index) const
{
    assert(index < properties_.size());
    return properties_.at(index);
}

void PropertyList::removeProperty(const std::string& id)
{
    logdbg << "PropertyList: removeProperty: start";
    assert(hasProperty(id));

    std::vector<Property>::iterator it;

    for (it = properties_.begin(); it != properties_.end(); it++)
    {
        if (it->name().compare(id) == 0)
        {
            properties_.erase(it);
            logdbg << "PropertyList: removeProperty: end";
            return;
        }
    }
    logerr << "PropertyList: removeProperty: property " << id << " could not be removed";
    assert(false);
}

const Property& PropertyList::get(const std::string& id) const
{
    logdbg << "PropertyList: get: start";
    assert(hasProperty(id));

    std::vector<Property>::const_iterator it;

    for (it = properties_.begin(); it != properties_.end(); it++)
    {
        if (it->name().compare(id) == 0)
        {
            return *it;
        }
    }
    logerr << "PropertyList: get: property " << id << " not found";
    throw std::runtime_error("PropertyList: get: property " + id + " not found");
}

unsigned int PropertyList::getPropertyIndex(const std::string& id) const
{
    logdbg << "PropertyList: getPropertyIndex: start";
    if (!hasProperty(id))
        throw std::runtime_error("PropertyList: getPropertyIndex: property " + id +
                                 " does not exists");

    unsigned int cnt = 0;
    for (auto& it : properties_)
    {
        if (it.name() == id)
        {
            return cnt;
        }
        cnt++;
    }
    throw std::runtime_error("PropertyList: getPropertyIndex: property " + id + " not found");
}

bool PropertyList::hasProperty(const std::string& id) const
{
    logdbg << "PropertyList: hasProperty: start";

    for (auto& it : properties_)
    {
        if (it.name() == id)
            return true;
    }

    return false;
}

bool PropertyList::hasProperty(const Property& prop) const
{
    logdbg << "PropertyList: hasProperty: start";

    for (auto& it : properties_)
    {
        if (it.name() == prop.name() && it.dataType() == prop.dataType())
            return true;
    }

    return false;
}

void PropertyList::clear()
{
    logdbg << "PropertyList: clear: start";
    properties_.clear();
    logdbg << "PropertyList: clear: end";
}

void PropertyList::print () const
{
    for (auto& it : properties_)
    {
        loginf << "Property id '" << it.name() << "' type " << it.dataTypeString();
    }
}
