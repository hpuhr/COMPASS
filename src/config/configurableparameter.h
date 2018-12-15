#ifndef CONFIGURABLPPARAMETER_H
#define CONFIGURABLPPARAMETER_H

#include <tinyxml2.h>
#include <string>

class Configurable;

/**
 * @brief Configuration parameter template class
 *
 * @details Several types of parameters are implement here:
 * "ParameterBool", "ParameterInt", "ParameterUnsignedInt", "ParameterFloat", "ParameterDouble", "ParameterString"
 *
 * For all such types one template class was implemented to unify setting and retrieving the XML configuration.
 */
template <class T> class ConfigurableParameter
{
public:
    /// Constructor, initializes members
    explicit ConfigurableParameter () {}

    /// Copy constructor, uses assignment operator
    ConfigurableParameter(const ConfigurableParameter &source);

    /// Assignment operator, make deep copy and discards the pointer.
    virtual ConfigurableParameter& operator= (const ConfigurableParameter& source);

    /// Destructor
    virtual ~ConfigurableParameter () {}

    /// Parameter identifier
    std::string parameter_id_;
    /// Template pointer to real value
    T* pointer_{nullptr};
    /// Template configuration value
    T config_value_;
    /// Template default value as given by registerParameter
    T default_value_;

    /// Parses an element from the XML configuration and sets parameter_id_ and config_value_ (using a stringstream)
    void parseElement (const tinyxml2::XMLElement *element);

    /**
     * Returns the parameter type as string
     *
     * \exception std::runtime_error if unknown class is used
     */
    std::string getParameterType () const;

    /// Returns parameter identifier
    const std::string& getParameterId () const;

    /// Returns parameter value as string (using a stringstream)
    std::string getParameterValue () const;

    /// Sets pointer_ to default value if valid, otherwise sets config_value_ to default_value_
    void resetToDefault ();

protected:
    void setConfigValue (const tinyxml2::XMLAttribute* attribute);
};

//template<> void A<int>::AFnc(); // <- note, no function body

template<> void ConfigurableParameter<bool>::setConfigValue (const tinyxml2::XMLAttribute* attribute);
template<> void ConfigurableParameter<int>::setConfigValue (const tinyxml2::XMLAttribute* attribute);
template<> void ConfigurableParameter<unsigned int>::setConfigValue (const tinyxml2::XMLAttribute* attribute);
template<> void ConfigurableParameter<float>::setConfigValue (const tinyxml2::XMLAttribute* attribute);
template<> void ConfigurableParameter<double>::setConfigValue (const tinyxml2::XMLAttribute* attribute);
template<> void ConfigurableParameter<std::string>::setConfigValue (const tinyxml2::XMLAttribute* attribute);

template<> std::string ConfigurableParameter<bool>::getParameterType () const;
template<> std::string ConfigurableParameter<int>::getParameterType () const;
template<> std::string ConfigurableParameter<unsigned int>::getParameterType () const;
template<> std::string ConfigurableParameter<float>::getParameterType () const;
template<> std::string ConfigurableParameter<double>::getParameterType () const;
template<> std::string ConfigurableParameter<std::string>::getParameterType () const;

template<> std::string ConfigurableParameter<bool>::getParameterValue () const;
template<> std::string ConfigurableParameter<int>::getParameterValue () const;
template<> std::string ConfigurableParameter<unsigned int>::getParameterValue () const;
template<> std::string ConfigurableParameter<float>::getParameterValue () const;
template<> std::string ConfigurableParameter<double>::getParameterValue () const;
template<> std::string ConfigurableParameter<std::string>::getParameterValue () const;

#endif // CONFIGURABLPPARAMETER_H
