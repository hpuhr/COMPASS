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

#ifndef TRANSFORMATIONFACTORY_H
#define TRANSFORMATIONFACTORY_H

#include "Singleton.h"

#include <string>
#include <map>

class Configurable;
class Transformation;


/**
@brief Singleton factory that creates Transformation instances.

Transformations can be created in a configurable and non-configurable way. A Transformation
has to be registered at the Factory using its ID before it can be created. The registered
transformations are then duplicated using their clone() method.

@todo Since the creation of transformations is handled using the clone() method, it may be easy
to introduce prototypes to this class if needed. Just convert it to a Configurable, make the register
method explicit and use specific unique strings to register them instead of the transformation internal
fixed ids...
  */
class TransformationFactory : public Singleton
{
public:
    typedef std::map<std::string,Transformation*> Transformations;

    /**
    @brief Returns the singleton instance

    Returns the singleton instance.
    @return Instance of the singleton.
      */
    static TransformationFactory& getInstance()
    {
        static TransformationFactory instance;
        return instance;
    }

    /// @brief Destructor
    virtual ~TransformationFactory();

    /// @brief Clones the registered transformation of the given id
    Transformation* createTransformation( const std::string& id );
    /// @brief Clones the registered transformation of the given id as a subconfigurable of the given parent
    Transformation* createTransformation( const std::string& class_id,
                                          const std::string& instance_id,
                                          Configurable* parent,
                                          const std::string& id );

    /// @brief Returns all registered transformations
    const Transformations& getRegisteredTransformations() const { return trafos_; }

private:
    /// @brief Unregisters all transformations
    void unregisterAll();
    /// @brief Registers all needed transformations
    void registerAll();
    /// @brief Registers the given transformation
    void registerTransformation( Transformation* trafo );

    /// @brief Private constructor
    TransformationFactory();

    /// Registered transformations
    Transformations trafos_;
};

#endif //TRANSFORMATIONFACTORY_H
