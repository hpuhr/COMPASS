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

#ifndef TRANSFORMATIONBASE_H
#define TRANSFORMATIONBASE_H

#include "TransformationVariable.h"
#include "Property.h"

#include <stdexcept>
#include "Data.h"

using namespace Utils::Data;

/**
@brief Serves as a base class for all classes that want to operate on buffers by using transformation variables

This class was introduced for convenience to provide an easy to use basis for operations on buffers using
transformation variable, as the name says most likely transformations that are applied to a buffers data
rows.

There are three sets of transformation variables that may be used.
- Input variables: Variables a transformation expects as an input. Those may be partially optional.
- Output variables: Variables a transformation needs to write its output data to. Those may be partially optional too.
- Hidden variables: Variables that are only for internal usage. Those cannot be retrieved from outside.

Note that the Transformation has to create the variables it needs itself. Input and output variables then can be accessed
from the outside to be properly configured for various DBO types. Internally the transformation relies on the correct
configuration of its variables and retrieves the hopefully correct properties from input and output buffers.

Hidden variables are mostly created in the input buffer to store additional internal information.

There are many convenience methods implemented in this class. Input variables can be prechecked for existence in the
input buffer, output variables that are not present in the output buffer can be created, hidden variables can be created
in the input buffer etc.

The create*Properties() methods will prefetch and store important information from a buffer, like the needed properties.
There are also a lot of quick access methods to functionality like isNan, setNan etc. They can be used via a quick access key
that can be retrieved for a transformation variable by calling iKey, oKey, hKey. For some quick access methods like *IsNan()
it may be vital to use the *SetAddresses() methods, in order to set the current property data pointers.

All fast access methods begin either with i, o, or h to signalize input, output or hidden variable methods.
  */
class TransformationBase : public Configurable
{
public:
    typedef std::map<std::string,int> PropertyKeyMap;
    typedef std::vector<int> PropertyIndices;
    //typedef std::vector<Property*> Properties;

    /// @brief Constructor
    //TransformationBase();
    /// @brief Copy constructor
    //TransformationBase( const TransformationBase& cpy );
    /// @brief Configurable constructor
    TransformationBase( std::string class_id, std::string instance_id, Configurable *parent );
    /// @brief Destructor
    virtual ~TransformationBase();

    /// @brief Return the number of input transformation variables
    unsigned int numberInputVariables() const;
    /// @brief Returns the idx'th input transformation variable
    TransformationVariable* getInputVariable( int idx );
    /// @brief Returns the input transformation variable of the given name
    TransformationVariable* getInputVariable( const std::string& name );
    /// @brief Sets the input transformation variables
    void setInputVariables( const TransformationVariables& vars );

    /// @brief Return the number of output transformation variables
    unsigned int numberOutputVariables() const;
    /// @brief Returns the idx'th output transformation variable
    TransformationVariable* getOutputVariable( int idx );
    /// @brief Returns the output transformation variable of the given name
    TransformationVariable* getOutputVariable( const std::string& name );
    /// @brief Sets the output transformation variables
    void setOutputVariables( const TransformationVariables& vars );

    /// @brief Assignment operator
    TransformationBase& operator=( const TransformationBase& rhs );

    /// Returns all erroneous input variables in a printable string format
    std::string getErrorVariables( Buffer* buffer );

    virtual void generateSubConfigurable( std::string class_id, std::string instance_id );

protected:
    virtual void checkSubConfigurables();

    /// @brief Adds an input transformation variable of the given name
    TransformationVariable* addInputVariable( const std::string& name );
    /// @brief Adds an input transformation variable
    TransformationVariable* addInputVariable( const std::string& name, PropertyDataType data_type, const std::string& default_id="" );
    /// @brief Adds an output transformation variable of the given name
    TransformationVariable* addOutputVariable( const std::string& name );
    /// @brief Adds an output transformation variable
    TransformationVariable* addOutputVariable( const std::string& name, PropertyDataType data_type, const std::string& default_id="" );
    /// @brief Adds a hidden transformation variable of the given name
    TransformationVariable* addHiddenVariable( const std::string& name );
    /// @brief Adds a hidden transformation variable
    TransformationVariable* addHiddenVariable( const std::string& name, PropertyDataType data_type, const std::string& default_id="" );

    /// @brief Return the number of hidden transformation variables
    unsigned int numberHiddenVariables() const;
    /// @brief Returns the idx'th hidden transformation variable
    TransformationVariable* getHiddenVariable( int idx );
    /// @brief Returns the hidden transformation variable of the given name
    TransformationVariable* getHiddenVariable( const std::string& name );

    /// @brief Reads all needed input properties from the given buffer
    void createInputProperties( Buffer* buffer );
    /// @brief Reads all needed output properties from the given buffer
    void createOutputProperties( Buffer* buffer );
    /// @brief Reads all needed hidden properties from the given buffer
    void createHiddenProperties( Buffer* buffer );
    /// @brief Checks all input variables for existence in the given buffer
    bool checkInputVariables( Buffer* buffer );
    /// @brief Checks if the output variables are present in the given buffer and adds them if needed
    void bufferFromOutputVariables( Buffer** output, const std::string &dbo_type );
    /// @brief Adds the hidden variables to the given buffer if not already present
    void addHiddenProperties( Buffer* buffer );
    /// @brief Clears all stored Property data created by the create*Properties() methods
    void resetProperties();

    /// @brief Returns the quick access id for the input variable of the given name
    inline int iKey( const std::string& name );
    /// @brief Checks if the input property with the given quick access key exists
    inline bool iExists( int key ) { return ( properties_in_.hasProperty(key) != 0 ); }
    /// @brief Returns the input property with the given quick access key
    inline const Property& iProperty( int key ) { return properties_in_.at(key); }
    /// @brief Returns the index of the input property with the given quick access key
    inline int iIndex( int key ) { return indices_in_[ key ]; }
    /// @brief Checks if the input property with the given quick access key is NaN
    //inline bool iIsNan( int key ) { return isNan( properties_in_[ key ]->g, (*addresses_in_)[ indices_in_[ key ] ] ); }
    /// @brief Sets the input property with the given quick access key to NaN
    //inline void iSetNan( int key ) { setNan( properties_in_[ key ]->data_type_int_, (*addresses_in_)[ indices_in_[ key ] ] ); }
    /// @brief Returns the current data address pointer of the input property with the given quick access key
    //inline void* iPtr( int key ) { return (*addresses_in_)[ indices_in_[ key ] ]; }
    /// @brief Sets the current address pointer vector of the input buffer
    //inline void iSetAddresses( std::vector<void*>* addresses ) { addresses_in_ = addresses; }

    /// @brief Returns the quick access id for the output variable of the given name
    inline int oKey( const std::string& name );
    /// @brief Checks if the output property with the given quick access key exists
    inline bool oExists( int key ) { return ( properties_out_.hasProperty(key) != 0 ); }
    /// @brief Returns the output property with the given quick access key
    inline const Property& oProperty( int key ) { return properties_out_.at(key); }
    /// @brief Returns the index of the output property with the given quick access key
    inline int oIndex( int key ) { return indices_out_[ key ]; }
    /// @brief Checks if the output property with the given quick access key is NaN
    //inline bool oIsNan( int key ) { return isNan( properties_out_[ key ]->data_type_int_, (*addresses_out_)[ indices_out_[ key ] ] ); }
    /// @brief Sets the output property with the given quick access key to NaN
    //inline void oSetNan( int key ) { setNan( properties_out_[ key ]->data_type_int_, (*addresses_out_)[ indices_out_[ key ] ] ); }
    /// @brief Returns the current data address pointer of the output property with the given quick access key
    //inline void* oPtr( int key ) { return (*addresses_out_)[ indices_out_[ key ] ]; }
    /// @brief Sets the current address pointer vector of the output buffer
    //inline void oSetAddresses( std::vector<void*>* addresses ) { addresses_out_ = addresses; }

    /// @brief Returns the quick access id for the hidden variable of the given name
    inline int hKey( const std::string& name );
    /// @brief Checks if the hidden property with the given quick access key exists
    inline bool hExists( int key ) { return ( properties_hidden_.hasProperty(key) != 0 ); }
    /// @brief Returns the hidden property with the given quick access key
    inline const Property& hProperty( int key ) { return properties_hidden_.at(key); }
    /// @brief Returns the index of the hidden property with the given quick access key
    inline int hIndex( int key ) { return indices_hidden_[ key ]; }
    /// @brief Checks if the hidden property with the given quick access key is NaN
    //inline bool hIsNan( int key ) { return isNan( properties_hidden_[ key ]->data_type_int_, (*addresses_in_)[ indices_hidden_[ key ] ] ); }
    /// @brief Sets the hidden property with the given quick access key to NaN
    //inline void hSetNan( int key ) { setNan( properties_hidden_[ key ]->data_type_int_, (*addresses_in_)[ indices_hidden_[ key ] ] ); }
    /// @brief Returns the current data address pointer of the hidden property with the given quick access key
    //inline void* hPtr( int key ) { return (*addresses_in_)[ indices_hidden_[ key ] ]; }

private:
    /// Input transformation variables
    TransformationVariables* vars_in_;
    /// Output transformation variables
    TransformationVariables* vars_out_;
    /// Hidden transformation variables
    TransformationVariables* vars_hidden_;
    /// Quick access keys for the input properties
    PropertyKeyMap keys_in_;
    /// Quick access keys for the output properties
    PropertyKeyMap keys_out_;
    /// Quick access keys for the hidden properties
    PropertyKeyMap keys_hidden_;
    /// Input property indices
    PropertyIndices indices_in_;
    /// Output property indices
    PropertyIndices indices_out_;
    /// Hidden property indices
    PropertyIndices indices_hidden_;
    /// Input properties
    PropertyList properties_in_;
    /// Output properties
    PropertyList properties_out_;
    /// Hidden properties
    PropertyList properties_hidden_;
    /// Current input data address pointers
    //std::vector<void*>* addresses_in_;
    /// Current output data address pointers
    //std::vector<void*>* addresses_out_;
};

/**
Returns the quick access id for the input variable of the given name.
The quick access key can be used subsequently to use alle the i* methods.
@param name Name of the input variable.
@return Quick access id for the input variable of the given name.
  */
int TransformationBase::iKey( const std::string& name )
{
    if( keys_in_.find( name ) == keys_in_.end() )
        throw std::runtime_error( "TransformationBase: iKey: Unknown variable name." );
    return keys_in_[ name ];
}

/**
Returns the quick access id for the output variable of the given name.
The quick access key can be used subsequently to use alle the o* methods.
@param name Name of the output variable.
@return Quick access id for the output variable of the given name.
  */
int TransformationBase::oKey( const std::string& name )
{
    if( keys_out_.find( name ) == keys_out_.end() )
        throw std::runtime_error( "TransformationBase: oKey: Unknown variable name." );
    return keys_out_[ name ];
}

/**
Returns the quick access id for the hidden variable of the given name.
The quick access key can be used subsequently to use alle the h* methods.
@param name Name of the hidden variable.
@return Quick access id for the hidden variable of the given name.
  */
int TransformationBase::hKey( const std::string& name )
{
    if( keys_hidden_.find( name ) == keys_hidden_.end() )
        throw std::runtime_error( "TransformationBase: hKey: Unknown variable name." );
    return keys_hidden_[ name ];
}

#endif //TRANSFORMATIONBASE_H
