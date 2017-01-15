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

#ifndef BUFFERFILTER_H
#define BUFFERFILTER_H

#include "Global.h"
#include "Property.h"
#include "Configurable.h"

#include <map>
#include <set>

class Buffer;
class DBOVariable;


/**
@brief Represents a configurable Property entry of the BufferFilter class.

The Property is used as a filter rule and can be checked on existence in a given Buffer. The Property consists
of a string id and a data type and belongs to a specific DBO.
  */
class BufferFilterEntry : public Configurable
{
public:
    /// @brief Constructor
    //BufferFilterEntry( const std::string &dbo_type );
    /// @brief Configurable constructor
    BufferFilterEntry( const std::string& class_id,
                       const std::string& instance_id,
                       Configurable* parent );
    /// @brief Destructor
    virtual ~BufferFilterEntry();

    /// @brief Sets the filter Property
    void setProperty( const Property& prop );
    /// @brief Sets the filter Property
    void setProperty( const std::string& id, PropertyDataType data_type );
    /// @brief Sets the filter Property from a DBO variable
    void setProperty( DBOVariable* var );

    /// @brief Ordering for filter entries
    bool operator<( const BufferFilterEntry& other ) const;

    /// @brief Checks for existence of the filter Property in the given buffer
    bool exists( Buffer* buffer ) const;

    /// @brief Retrieves the Property from the entry
    Property getProperty() const;
    /// @brief Returns the Property id of the entry
    const std::string& getID() const;
    /// @brief Returns the data type of the entry as integer number
    PropertyDataType dataType() const;
    /// @brief Returns the DBO type the Property is assigned to as integer number
    const std::string &dboType() const;

    /// @brief Retrieves a configuration and fills in the given filter entry data
    static void getConfig( Configuration& config, const std::string &dbo_type, const std::string& id, PropertyDataType data_type );

private:
    /// DBO type of the Property
    std::string dbo_type_;
    /// Property id
    std::string id_;
    /// Property data type
    PropertyDataType data_type_;
    std::string data_type_str_;
};

/**
@brief Represents a configurable filter rule entry of the BufferFilter class.

Filter rules can be used to decide treatment of a Buffer on a per-DBO-type basis. See class BufferFilter for
possible filter rules.
  */
class BufferFilterRuleEntry : public Configurable
{
public:
    /// @brief Constructor
    //BufferFilterRuleEntry( const std::string &dbo_type, int filter_rule );
    /// @brief Configurable constructor
    BufferFilterRuleEntry( const std::string& class_id,
                           const std::string& instance_id,
                           Configurable* parent );
    /// @brief Destructor
    virtual ~BufferFilterRuleEntry();

    /// @brief Sets the filter rule as integer number
    void setRule( int filter_rule );
    /// @brief Returns the DBO type as integer number
    const std::string &dboType() const;
    /// @brief Returns the filter rule as integer number
    int filterRule() const;

    /// @brief Retrieves a configuration and fills in the given rule entry data
    static void getConfig( Configuration& config, const std::string &dbo_type, int filter_rule );

private:
    /// DBO type the rule is assigned to
    std::string dbo_type_;
    /// Filter rule, possible values defined in enum BufferFilterRule
    int filter_rule_;
};

/**
@brief A filter structure for Buffer objects.

This is a convenience class that allows to discriminate further treatment of an incoming Buffer depending on various constraints.
The class can currently be used to filter a Buffer in two ways.

- Check the existence of specific properties (see class Property) in the buffer.
- Define treatment rules on a per-DBO-type basis.

Properties: Always provide a valid data type! The data type of a defined property will always be compared to the property
data type in the buffer. If a Meta DBO variable is used all its subvariables will be added as DBO specific properties. Adding
the same property id twice for a DBO type will throw an exception.

Rules: Additionally to the specific rules a base rule can be defined that is applied if no specific rule can be found for a given DBO type. Existing
rules for a specific DBO type will just be overwritten by new ones. If neither a base rule or a specific rule is present for a given buffer, the
buffer will always result in the BLOCK rule!

Small example. Here all DB_OBJECT_TYPE's will be blocked except DBO_PLOTS. Further a specific property has to be present in the buffer.
@code
//define the filter using rules
BufferFilter filter;
filter.setBaseRule( BufferFilter::BLOCK );
filter.setRule( DBO_PLOTS, BufferFilter::FORWARD );

//define the filter using a property
filter.addPropertyToFilter( DBO_PLOTS, "lurch", P_TYPE_INT );

//handle buffer depending on filter maybe like this:
if( !filter.checkProperties( buffer ) || getRule( buffer ) == BufferFilter::BLOCK )
{
    delete buffer;
    return;
}
else
{
    //Buffer has to be of DBO type DBO_PLOTS with a property "lurch" of data type integer.
    //...
}
@endcode
  */
class BufferFilter : public Configurable
{
public:
    /**
    Defines how a buffer shall be treated depending on its DBO type. Feel free to add more codes here.
    The existing codes refer to the handling of buffers in the computation framework.
    FORWARD = Just let the buffer through
    BLOCK = Block the buffer (and maybe destroy it?)
    TRANSFORM = Transform the buffer
    */
    enum BufferFilterRule { FORWARD=0, BLOCK, TRANSFORM };

    typedef std::map<std::string,BufferFilterRuleEntry*> Rules; // by dbo type
    typedef std::map<std::string,BufferFilterEntry*> FilterEntries;
    typedef std::map<std::string,FilterEntries> PropertyFilter; // by dbo type

    /// @brief Constructor
    //BufferFilter();
    /// @brief Configurable constructor
    BufferFilter( const std::string& class_id,
                  const std::string& instance_id,
                  Configurable* parent );
    /// @brief Destructor
    virtual ~BufferFilter();

    /// @brief Adds a DBO type specific Property to the filter
    void addPropertyToFilter( const std::string &dbo_type, const Property& id );
    /// @brief Adds a DBO type specific Property to the filter
    void addPropertyToFilter( const std::string &dbo_type, const std::string& id, PropertyDataType data_type );
    /// @brief Adds one or more DBO type specific properties to the filter
    void addPropertyToFilter( DBOVariable* var );
    /// @brief Clears all properties
    void clearPropertyFilter();
    /// @brief Checks the existence of all defined properties in the given buffer
    bool checkProperties( Buffer* buffer );
    /// @brief Checks if the given Property id already exists for the given DBO type
    bool hasProperty( const std::string &dbo_type, const std::string& id );
    /// @brief Returns all defined properties
    std::multimap<std::string,Property> getProperties() const; // by dbo type

    /// @brief Sets a filter rule for the given DBO type
    void setRule( const std::string &dbo_type, BufferFilterRule rule );
    /// @brief Sets one or more filter rules using the given DBOVariable
    void setRule( DBOVariable* var, BufferFilterRule rule );
    /// @brief Sets a base filter rule
    void setBaseRule( BufferFilterRule rule );
    /// @brief Returns a filter rule for the given buffer
    BufferFilterRule getRule( Buffer* buffer );
    /// @brief Returns a filter rule for the given DBO type
    BufferFilterRule getRule( const std::string &type );
    /// @brief Returns all defined filter rules
    std::map<std::string,BufferFilterRule> getRules() const; // by dbo type
    /// @brief Checks if a specific rule has been defined for the given DBO type
    bool hasRule( const std::string &type ) const;
    /// @brief Checks if a base filter rule has been defined
    bool hasBaseRule() const;
    /// @brief Returns the base filter rule
    BufferFilterRule getBaseRule() const;
    /// @brief Removes the base filter rule
    void clearBaseRule();
    /// @brief Clears all filter rules
    void clearRules();

    /// @brief Clears the filter
    void clear();

    /// @brief Returns all erroneous (non-existing) properties for the given buffer in a printable format
    std::string getErrorProperties( Buffer* buffer );

    virtual void generateSubConfigurable( std::string class_id, std::string instance_id );

protected:
    virtual void checkSubConfigurables();

private:
    /// Base filter rule
    BufferFilterRuleEntry* base_rule_;
    /// DBO type specific filter rules
    Rules rules_;
    /// Filter properties
    PropertyFilter property_filter_;
};

#endif //BUFFERFILTER_H
