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

#ifndef BUFFER_H_
#define BUFFER_H_

#include "propertylist.h"
#include "logger.h"

#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

class DBOVariableSet;

template <class T>
class NullableVector;

typedef std::tuple<std::map<std::string, std::shared_ptr<NullableVector<bool>>>,
                   std::map<std::string, std::shared_ptr<NullableVector<char>>>,
                   std::map<std::string, std::shared_ptr<NullableVector<unsigned char>>>,
                   std::map<std::string, std::shared_ptr<NullableVector<int>>>,
                   std::map<std::string, std::shared_ptr<NullableVector<unsigned int>>>,
                   std::map<std::string, std::shared_ptr<NullableVector<long int>>>,
                   std::map<std::string, std::shared_ptr<NullableVector<unsigned long int>>>,
                   std::map<std::string, std::shared_ptr<NullableVector<float>>>,
                   std::map<std::string, std::shared_ptr<NullableVector<double>>>,
                   std::map<std::string, std::shared_ptr<NullableVector<std::string>>>>
    ArrayListMapTupel;

template <class T, class Tuple>
struct Index;

template <class T, class... Types>
struct Index<T, std::tuple<T, Types...>>
{
    static const std::size_t value = 0;
};

template <class T, class U, class... Types>
struct Index<T, std::tuple<U, Types...>>
{
    static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
};

/**
 * @brief Fast, dynamic data container
 *
 * Encapsulates general data storage with access functions with maximum efficiency.
 * Performs basic checks, allocates new data when space is needed, but NOT thread-safe.
 *
 */
class Buffer
{
    template <class T>
    friend class NullableVector;

  public:
    /// @brief Default constructor.
    Buffer();
    /// @brief Constructor.
    Buffer(PropertyList properties, const std::string& dbo_name = "");
    /// @brief Desctructor.
    virtual ~Buffer();

    /// @brief Adds all containers of org_buffer and removes them from org_buffer.
    void seizeBuffer(Buffer& org_buffer);

    /// @brief Adds an additional property.
    void addProperty(std::string id, PropertyDataType type);
    void addProperty(const Property& property);

    /// @brief Returns boolean indicating if any data was ever written.
    bool firstWrite();

    /// @brief Returns boolean indicating if buffer is the last of one DB operation.
    bool lastOne() { return last_one_; }
    /// @brief Sets if buffer is the last one of one DB operation.
    void lastOne(bool last_one) { last_one_ = last_one; }

    /// @brief Returns the buffers id
    unsigned int id() const { return id_; }

    template <typename T>
    bool has(const std::string& id);

    template <typename T>
    NullableVector<T>& get(const std::string& id);

    template <typename T>
    void rename(const std::string& id, const std::string& id_new);

    /// @brief  Returns current size
    size_t size();
    void cutToSize(size_t size);

    /// @brief Returns PropertyList
    const PropertyList& properties();

    /// @brief Returns DBO type
    const std::string& dboName() { return dbo_name_; }

    /// @brief Sets DBO type
    void dboName(const std::string& dbo_name) { dbo_name_ = dbo_name; }

    bool isNone(const Property& property, unsigned int row_cnt);

    void transformVariables(DBOVariableSet& list,
                            bool tc2dbovar);  // tc2dbovar true for db->dbo, false dbo->db

    std::shared_ptr<Buffer> getPartialCopy(const PropertyList& partial_properties);

  protected:
    /// Unique buffer id, copied when getting shallow copies
    unsigned int id_;
    /// List of all properties
    PropertyList properties_;
    /// DBO type
    std::string dbo_name_;

    ArrayListMapTupel array_list_tuple_;
    size_t data_size_{0};

    /// Flag indicating if buffer is the last of a DB operation
    bool last_one_;

    static unsigned int ids_;

  private:
    template <typename T>
    inline std::map<std::string, std::shared_ptr<NullableVector<T>>>& getArrayListMap();
    template <typename T>
    void renameArrayListMapEntry(const std::string& id, const std::string& id_new);
    template <typename T>
    void seizeArrayListMap(Buffer& org_buffer);
};

#include "nullablevector.h"

template <typename T>
inline bool Buffer::has(const std::string& id)
{
    return getArrayListMap<T>().count(id) != 0;
}

template <typename T>
NullableVector<T>& Buffer::get(const std::string& id)
{
    if (!(std::get<Index<std::map<std::string, std::shared_ptr<NullableVector<T>>>,
              ArrayListMapTupel>::value>(array_list_tuple_)).count(id))
        logerr << "Buffer: get: id '" << id << "' type " << typeid(T).name() << " not found";

    assert ((std::get<Index<std::map<std::string, std::shared_ptr<NullableVector<T>>>,
             ArrayListMapTupel>::value>(array_list_tuple_)).count(id));

    return *(std::get<Index<std::map<std::string, std::shared_ptr<NullableVector<T>>>,
                            ArrayListMapTupel>::value>(array_list_tuple_))
                .at(id);
}

template <typename T>
void Buffer::rename(const std::string& id, const std::string& id_new)
{
    renameArrayListMapEntry<T>(id, id_new);

    assert(properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());
}

// private stuff

template <typename T>
std::map<std::string, std::shared_ptr<NullableVector<T>>>& Buffer::getArrayListMap()
{
    return std::get<
        Index<std::map<std::string, std::shared_ptr<NullableVector<T>>>, ArrayListMapTupel>::value>(
        array_list_tuple_);
}
template <typename T>
void Buffer::renameArrayListMapEntry(const std::string& id, const std::string& id_new)
{
    assert(getArrayListMap<T>().count(id) == 1);
    assert(getArrayListMap<T>().count(id_new) == 0);
    std::shared_ptr<NullableVector<T>> array_list = getArrayListMap<T>().at(id);
    getArrayListMap<T>().erase(id);
    getArrayListMap<T>()[id_new] = array_list;
}

template <typename T>
void Buffer::seizeArrayListMap(Buffer& org_buffer)
{
    assert(getArrayListMap<T>().size() == org_buffer.getArrayListMap<T>().size());

    for (auto it : getArrayListMap<T>())
        it.second->addData(*org_buffer.getArrayListMap<T>().at(it.first));

    org_buffer.getArrayListMap<T>().clear();
}

#endif /* BUFFER_H_ */
