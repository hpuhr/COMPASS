/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROPERTYLIST_H_
#define PROPERTYLIST_H_

#include "property.h"
#include "logger.h"

#include <vector>

/**
 * @brief List of Property instances
 *
 * Uses copy constructor for assignment.
 *
 */
// TODO maybe rework to map?
class PropertyList
{
protected:
    /// Container with all properties
    std::vector <Property> properties_;

public:
    /// @brief Constructor
    PropertyList()
    {
        logdbg << "PropertyList: constructor";
    }
    /// @brief Desctructor. Calls clear.
    virtual ~PropertyList()
    {
        logdbg << "PropertyList: destructor: start";
        clear();
        logdbg << "PropertyList: destructor: end";
    }

    /// @brief Copy constructor
    PropertyList(const PropertyList& org)
    {
        properties_ = org.properties_;
        //loginf << "PropertyList: constructor: properties " << properties_.size();
    }

    /// @brief Copy operator
    void operator= (const PropertyList &org)
    {
        properties_ = org.properties_;
    }

    void addPropertyList (const PropertyList &org)
    {
        properties_.insert(properties_.end(), org.properties_.begin(), org.properties_.end());
    }

    /// @brief Adds a property
    void addProperty (std::string id, PropertyDataType type)
    {
        logdbg << "PropertyList: addProperty: start";
        logdbg << "PropertyList: addProperty:  id '" << id << "' type " << Property::asString(type);
        assert (!id.empty());

        if(hasProperty(id))
        {
            logwrn << "PropertyList: addProperty: property " << id << " already added";
            return;
        }

        properties_.push_back (Property (id, type));
        logdbg << "PropertyList: addProperty: end";
    }

    /// @brief Adds a property
    void addProperty (Property &property)
    {
        logdbg << "PropertyList: addProperty: start";

        if(hasProperty(property.name()))
        {
            logwrn << "PropertyList: addProperty: property " << property.name() << " already added";
            return;
        }

        properties_.push_back (property);
        logdbg << "PropertyList: addProperty: end";
    }

    /// @brief Return container with all properties
    const std::vector <Property> &properties () const
    {
        return properties_;
    }

    const Property &at (unsigned int index) const
    {
        assert (index < properties_.size());
        return properties_.at(index);
    }

    /**
     * @brief Removes a property
     *
     * \exception std::runtime_error if identifier not found
     */
    void removeProperty (const std::string &id)
    {
        logdbg << "PropertyList: removeProperty: start";
        assert (hasProperty(id));

        std::vector <Property>::iterator it;

        for (it=properties_.begin(); it != properties_.end(); it++)
        {
            if (it->name().compare (id) == 0)
            {
                properties_.erase (it);
                logdbg << "PropertyList: removeProperty: end";
                return;
            }
        }
        logerr << "PropertyList: removeProperty: property " << id << " could not be removed";
        assert (false);
    }

    /**
     * @brief Returns a property by id
     *
     * \exception std::runtime_error if identifier not found
     */
    const Property& get (const std::string& id) const
    {
        logdbg << "PropertyList: get: start";
        assert (hasProperty(id));

        std::vector <Property>::const_iterator it;

        for (it=properties_.begin(); it != properties_.end(); it++)
        {
            if (it->name().compare (id) == 0)
            {
                return *it;
            }
        }
        logerr << "PropertyList: get: property " << id << " not found";
        assert (false);
    }

    /**
     * @brief Returns index of a property
     *
     * \exception std::runtime_error if identifier not found
     */
    unsigned int getPropertyIndex (const std::string& id) const
    {
        logdbg << "PropertyList: getPropertyIndex: start";
        if (!hasProperty(id))
            throw std::runtime_error ("PropteryList: getPropertyIndex: property "+id+" does not exists");

        unsigned int cnt=0;
        for (auto it : properties_)
        {
            if (it.name() == id)
            {
                return cnt;
            }
            cnt++;
        }
        throw std::runtime_error("PropteryList: getPropertyIndex: property "+id+" not found");
    }

    /// @brief Returns flag indicating if property is in list
    bool hasProperty (const std::string& id) const
    {
        logdbg << "PropertyList: hasProperty: start";

        for (auto it : properties_)
        {
            if (it.name() == id)
                return true;
        }

        return false;
    }

    /// @brief Returns flag indicating if property with given indexis in list
    bool hasProperty (unsigned int index) const
    {
        return index < properties_.size();
    }
    /// @brief Removes all properties
    void clear ()
    {
        logdbg << "PropertyList: clear: start";
        properties_.clear();
        logdbg << "PropertyList: clear: end";
    }

    /// @brief Return number of properties in list
    unsigned int size () const
    {
        return properties_.size();
    }
};


#endif /* PROPERTYLIST_H_ */
