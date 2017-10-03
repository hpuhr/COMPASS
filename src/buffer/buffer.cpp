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

/*
 * Buffer.cpp
 *
 *  Created on: Dec 8, 2011
 *      Author: sk
 */

#include "boost/date_time/posix_time/posix_time.hpp"

#include "buffer.h"
#include "logger.h"
#include "string.h"

unsigned int Buffer::ids_ = 0;

/**
 * Creates an empty buffer withput an DBO type
 *
 */
Buffer::Buffer()
{
    logdbg  << "Buffer: constructor: start";

    id_ = ids_;
    ++ids_;

    logdbg  << "Buffer: constructor: end";
}


/**
 * Creates a buffer from a PropertyList and a DBO type. Sets member to initial values.
 *
 * \param member_list PropertyList defining all properties
 * \param type DBO type
 */
Buffer::Buffer(PropertyList properties, const std::string &dbo_name)
: dbo_name_(dbo_name), last_one_(false)
//, first_write_(true),
//search_active_(false), search_key_pos_(-1), search_key_min_ (-1), search_key_max_ (-1)
{
    logdbg  << "Buffer: constructor: start";

    id_ = ids_;
    ++ids_;

    for (unsigned int cnt=0; cnt < properties.size(); cnt++)
        addProperty(properties.at(cnt));

    //init();

    logdbg  << "Buffer: constructor: end";
}

/**
 * Calls clear.
 */
Buffer::~Buffer()
{
    logdbg  << "Buffer: destructor: start";

    properties_.clear();
    arrays_bool_.clear();
    arrays_char_.clear();
    arrays_uchar_.clear();
    arrays_int_.clear();
    arrays_uint_.clear();
    arrays_long_int_.clear();
    arrays_ulong_int_.clear();
    arrays_float_.clear();
    arrays_double_.clear();
    arrays_string_.clear();

//    last_one_=false;

    logdbg  << "Buffer: destructor: end";
}

/**
 * \exception std::runtime_error if unknown property type
 */
//void Buffer::print (unsigned int num_elements)
//{
//    Property *prop=0;
//    unsigned int index_old=index_;
//
//    if (num_elements > getSize())
//        num_elements = getSize();
//
//    unsigned int num_properties = member_list_.getNumProperties();
//    std::vector <Property*> * properties= member_list_.getProperties();
//
//    std::stringstream ss;
//
//    for (unsigned int cnt=0; cnt < num_properties; cnt++)
//    {
//        prop=properties->at(cnt);
//
//        ss << "(" << PROPERTY_DATA_TYPE_STRINGS[(PROPERTY_DATA_TYPE)prop->data_type_int_] << ") " << prop->id_ << "  ";
//    }
//
//    loginf  << "Buffer: print: properties: " << ss.str();
//
//
//    setIndex (0);
//    for (unsigned int index=0; index < num_elements; index++)
//    {
//        if (index != 0)
//            incrementIndex();
//
//        ss.str(std::string());
//
//        for (unsigned int cnt=0; cnt < num_properties; cnt++)
//        {
//            prop=properties->at(cnt);
//
//            ss << Utils::String::getPropertyValueString(adresses_->at(cnt), (PROPERTY_DATA_TYPE)prop->data_type_int_) << " ";
//        }
//        loginf  << "Buffer: print: data "<< index <<": " << ss.str();
//    }
//    setIndex(index_old);
//}

//void Buffer::printLong (unsigned int num_elements)
//{
//    Property *prop=0;
//    unsigned int index_old=index_;
//
//    if (num_elements > getSize())
//        num_elements = getSize();
//
//    unsigned int num_properties = member_list_.getNumProperties();
//    std::vector <Property*> * properties= member_list_.getProperties();
//
//    std::stringstream ss;
//
////    for (unsigned int cnt=0; cnt < num_properties; cnt++)
////    {
////        prop=properties->at(cnt);
////
////
////    }
////
////    loginf  << "Buffer: print: properties: " << ss.str();
//
//
//    setIndex (0);
//    for (unsigned int index=0; index < num_elements; index++)
//    {
//        if (index != 0)
//            incrementIndex();
//
//        ss.str(std::string());
//
//        for (unsigned int cnt=0; cnt < num_properties; cnt++)
//        {
//            prop=properties->at(cnt);
//
//            ss << "(" << PROPERTY_DATA_TYPE_STRINGS[(PROPERTY_DATA_TYPE)prop->data_type_int_] << ") " << prop->id_ << "  ";
//
//            ss << Utils::String::getPropertyValueString(adresses_->at(cnt), (PROPERTY_DATA_TYPE)prop->data_type_int_) << std::endl;
//        }
//        loginf  << "Buffer: print: data "<< index <<": " << ss.str();
//    }
//    setIndex(index_old);
//}


/**
 * \param id Unique property identifier
 * \param type Property data type
 *
 * \exception std::runtime_error if property id already in use
 */
void Buffer::addProperty (std::string id, PropertyDataType type)
{
    logdbg  << "Buffer: addProperty:  id '" << id << "' type " << Property::asString(type);

    assert (!id.empty());

    if (properties_.hasProperty(id))
        throw std::runtime_error ("Buffer: addProperty: property "+id+" already exists");

    switch (type)
    {
    case PropertyDataType::BOOL:
        assert (arrays_bool_.count(id) == 0);
        arrays_bool_ [id] = std::shared_ptr<ArrayListTemplate<bool>> (new ArrayListTemplate<bool>());
        break;
    case PropertyDataType::CHAR:
        assert (arrays_char_.count(id) == 0);
        arrays_char_ [id] = std::shared_ptr<ArrayListTemplate<char>> (new ArrayListTemplate<char>());
        break;
    case PropertyDataType::UCHAR:
        assert (arrays_uchar_.count(id) == 0);
        arrays_uchar_ [id] = std::shared_ptr<ArrayListTemplate<unsigned char>> (new ArrayListTemplate<unsigned char>());
        break;
    case PropertyDataType::INT:
        assert (arrays_int_.count(id) == 0);
        arrays_int_ [id] = std::shared_ptr<ArrayListTemplate<int>> (new ArrayListTemplate<int>());
        break;
    case PropertyDataType::UINT:
        assert (arrays_uint_.count(id) == 0);
        arrays_uint_ [id] = std::shared_ptr<ArrayListTemplate<unsigned int>> (new ArrayListTemplate<unsigned int>());
        break;
    case PropertyDataType::LONGINT:
        assert (arrays_long_int_.count(id) == 0);
        arrays_long_int_ [id] = std::shared_ptr<ArrayListTemplate<long>> (new ArrayListTemplate<long>());
        break;
    case PropertyDataType::ULONGINT:
        assert (arrays_ulong_int_.count(id) == 0);
        arrays_ulong_int_ [id] = std::shared_ptr<ArrayListTemplate<unsigned long>> (new ArrayListTemplate<unsigned long>());
        break;
    case PropertyDataType::FLOAT:
        assert (arrays_float_.count(id) == 0);
        arrays_float_ [id] = std::shared_ptr<ArrayListTemplate<float>> (new ArrayListTemplate<float>());
        break;
    case PropertyDataType::DOUBLE:
        assert (arrays_double_.count(id) == 0);
        arrays_double_ [id] = std::shared_ptr<ArrayListTemplate<double>> (new ArrayListTemplate<double>());
        break;
    case PropertyDataType::STRING:
        assert (arrays_string_.count(id) == 0);
        arrays_string_ [id] = std::shared_ptr<ArrayListTemplate<std::string>> (new ArrayListTemplate<std::string>());
        break;
    default:
        logerr  <<  "Buffer: addProperty: unknown property type " << Property::asString(type);
        throw std::runtime_error ("Buffer: addProperty: unknown property type "+Property::asString(type));
    }

    properties_.addProperty(id,type);

    logdbg  << "Buffer: addProperty: end";
}

void Buffer::addProperty (const Property &property)
{
    addProperty (property.name(), property.dataType());
}

ArrayListTemplate<bool> &Buffer::getBool (const std::string &id)
{
    if (arrays_bool_.count(id) == 0)
        throw std::runtime_error ("Buffer: getBool: unknown id "+id);

    return *arrays_bool_[id];
}

ArrayListTemplate<char> &Buffer::getChar (const std::string id)
{
    if (arrays_char_.count(id) == 0)
        throw std::runtime_error ("Buffer: getChar: unknown id "+id);

    return *arrays_char_[id];
}

ArrayListTemplate<unsigned char> &Buffer::getUChar (const std::string &id)
{
    if (arrays_uchar_.count(id) == 0)
        throw std::runtime_error ("Buffer: getUChar: unknown id "+id);

    return *arrays_uchar_[id];
}

ArrayListTemplate<int> &Buffer::getInt (const std::string &id)
{
    if (arrays_int_.count(id) == 0)
        throw std::runtime_error ("Buffer: getInt: unknown id "+id);

    return *arrays_int_[id];
}

ArrayListTemplate<unsigned int> &Buffer::getUInt (const std::string &id)
{
    if (arrays_uint_.count(id) == 0)
        throw std::runtime_error ("Buffer: getUInt: unknown id "+id);

    return *arrays_uint_[id];
}

ArrayListTemplate<long int> &Buffer::getLongInt (const std::string &id)
{
    if (arrays_long_int_.count(id) == 0)
        throw std::runtime_error ("Buffer: getLongInt: unknown id "+id);

    return *arrays_long_int_[id];
}

ArrayListTemplate<unsigned long int> &Buffer::getULongInt (const std::string &id)
{
    if (arrays_ulong_int_.count(id) == 0)
        throw std::runtime_error ("Buffer: getULongInt: unknown id "+id);

    return *arrays_ulong_int_[id];
}

ArrayListTemplate<float> &Buffer::getFloat (const std::string &id)
{
    if (arrays_float_.count(id) == 0)
        throw std::runtime_error ("Buffer: getFloat: unknown id "+id);

    return *arrays_float_[id];
}

ArrayListTemplate<double> &Buffer::getDouble (const std::string &id)
{
    if (arrays_double_.count(id) == 0)
        throw std::runtime_error ("Buffer: getDouble: unknown id "+id);

    return *arrays_double_[id];
}

ArrayListTemplate<std::string> &Buffer::getString (const std::string &id)
{
    if (arrays_string_.count(id) == 0)
        throw std::runtime_error ("Buffer: getString: unknown id "+id);

    return *arrays_string_[id];
}

void Buffer::renameBool (const std::string &id, const std::string &id_new)
{
    assert (arrays_bool_.count(id) == 1);
    assert (arrays_bool_.count(id_new) == 0);

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());

    std::shared_ptr<ArrayListTemplate<bool>> array_list = arrays_bool_.at(id);
    arrays_bool_.erase(id);
    arrays_bool_[id_new]=array_list;
}

void Buffer::renameChar (const std::string &id, const std::string &id_new)
{
    assert (arrays_char_.count(id) == 1);
    assert (arrays_char_.count(id_new) == 0);

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());

    std::shared_ptr<ArrayListTemplate<char>> array_list = arrays_char_.at(id);
    arrays_char_.erase(id);
    arrays_char_[id_new]=array_list;
}

void Buffer::renameUChar (const std::string &id, const std::string &id_new)
{
    assert (arrays_uchar_.count(id) == 1);
    assert (arrays_uchar_.count(id_new) == 0);

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());

    std::shared_ptr<ArrayListTemplate<unsigned char>> array_list = arrays_uchar_.at(id);
    arrays_uchar_.erase(id);
    arrays_uchar_[id_new]=array_list;
}

void Buffer::renameInt (const std::string &id, const std::string &id_new)
{
    assert (arrays_int_.count(id) == 1);
    assert (arrays_int_.count(id_new) == 0);

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());

    std::shared_ptr<ArrayListTemplate<int>> array_list = arrays_int_.at(id);
    arrays_int_.erase(id);
    arrays_int_[id_new]=array_list;
}

void Buffer::renameUInt (const std::string &id, const std::string &id_new)
{
    assert (arrays_uint_.count(id) == 1);
    assert (arrays_uint_.count(id_new) == 0);

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());

    std::shared_ptr<ArrayListTemplate<unsigned int>> array_list = arrays_uint_.at(id);
    arrays_uint_.erase(id);
    arrays_uint_[id_new]=array_list;
}

void Buffer::renameLongInt (const std::string &id, const std::string &id_new)
{
    assert (arrays_long_int_.count(id) == 1);
    assert (arrays_long_int_.count(id_new) == 0);

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());

    std::shared_ptr<ArrayListTemplate<long int>> array_list = arrays_long_int_.at(id);
    arrays_long_int_.erase(id);
    arrays_long_int_[id_new]=array_list;
}

void Buffer::renameULongInt (const std::string &id, const std::string &id_new)
{
    assert (arrays_ulong_int_.count(id) == 1);
    assert (arrays_ulong_int_.count(id_new) == 0);

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());

    std::shared_ptr<ArrayListTemplate<unsigned long>> array_list = arrays_ulong_int_.at(id);
    arrays_ulong_int_.erase(id);
    arrays_ulong_int_[id_new]=array_list;
}

void Buffer::renameFloat (const std::string &id, const std::string &id_new)
{
    assert (arrays_float_.count(id) == 1);
    assert (arrays_float_.count(id_new) == 0);

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());

    std::shared_ptr<ArrayListTemplate<float>> array_list = arrays_float_.at(id);
    arrays_float_.erase(id);
    arrays_float_[id_new]=array_list;
}

void Buffer::renameDouble (const std::string &id, const std::string &id_new)
{
    assert (arrays_double_.count(id) == 1);
    assert (arrays_double_.count(id_new) == 0);

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());

    std::shared_ptr<ArrayListTemplate<double>> array_list = arrays_double_.at(id);
    arrays_double_.erase(id);
    arrays_double_[id_new]=array_list;
}

void Buffer::renameString (const std::string &id, const std::string &id_new)
{
    assert (arrays_string_.count(id) == 1);
    assert (arrays_string_.count(id_new) == 0);

    assert (properties_.hasProperty(id));
    Property old_property = properties_.get(id);
    properties_.removeProperty(id);
    properties_.addProperty(id_new, old_property.dataType());

    std::shared_ptr<ArrayListTemplate<std::string>> array_list = arrays_string_.at(id);
    arrays_string_.erase(id);
    arrays_string_[id_new]=array_list;
}

void Buffer::seizeBuffer (Buffer &org_buffer)
{
    logdbg  << "Buffer: seizeBuffer: start";

    logdbg  << "Buffer: seizeBuffer: full " << full() << " size " << size() << " first write " << firstWrite();

    assert (full() || firstWrite()); //|| first_write_

    //logdbg  << "Buffer: seizeBuffer: containers";
    //std::vector <PropertyContainer *> org_containers = org_buffer->containers_;

    assert (arrays_bool_.size() == org_buffer.arrays_bool_.size());
    assert (arrays_char_.size() == org_buffer.arrays_char_.size());
    assert (arrays_uchar_.size() == org_buffer.arrays_uchar_.size());
    assert (arrays_int_.size() == org_buffer.arrays_int_.size());
    assert (arrays_uint_.size() == org_buffer.arrays_uint_.size());
    assert (arrays_long_int_.size() == org_buffer.arrays_long_int_.size());
    assert (arrays_ulong_int_.size() == org_buffer.arrays_ulong_int_.size());
    assert (arrays_float_.size() == org_buffer.arrays_float_.size());
    assert (arrays_double_.size() == org_buffer.arrays_double_.size());
    assert (arrays_string_.size() == org_buffer.arrays_string_.size());

    logdbg  << "Buffer: seizeBuffer: inserting ";
    org_buffer.properties_.clear();

    for (auto it : arrays_bool_)
        it.second->addData(*org_buffer.arrays_bool_.at(it.first));
    org_buffer.arrays_bool_.clear();
    for (auto it : arrays_char_)
        it.second->addData(*org_buffer.arrays_char_.at(it.first));
    org_buffer.arrays_char_.clear();
    for (auto it : arrays_uchar_)
        it.second->addData(*org_buffer.arrays_uchar_.at(it.first));
    org_buffer.arrays_uchar_.clear();
    for (auto it : arrays_int_)
        it.second->addData(*org_buffer.arrays_int_.at(it.first));
    org_buffer.arrays_int_.clear();
    for (auto it : arrays_uint_)
        it.second->addData(*org_buffer.arrays_uint_.at(it.first));
    org_buffer.arrays_uint_.clear();
    for (auto it : arrays_long_int_)
        it.second->addData(*org_buffer.arrays_long_int_.at(it.first));
    org_buffer.arrays_long_int_.clear();
    for (auto it : arrays_ulong_int_)
        it.second->addData(*org_buffer.arrays_ulong_int_.at(it.first));
    org_buffer.arrays_ulong_int_.clear();
    for (auto it : arrays_float_)
        it.second->addData(*org_buffer.arrays_float_.at(it.first));
    org_buffer.arrays_float_.clear();
    for (auto it : arrays_double_)
        it.second->addData(*org_buffer.arrays_double_.at(it.first));
    org_buffer.arrays_double_.clear();
    for (auto it : arrays_string_)
        it.second->addData(*org_buffer.arrays_string_.at(it.first));
    org_buffer.arrays_string_.clear();

    logdbg  << "Buffer: seizeBuffer: size " << size();

    //containers_.insert (containers_.end(), org_containers.begin(), org_containers.end());
    //org_buffer->containers_.clear();
    //all your containers belong to us

//    logdbg  << "Buffer: seizeBuffer: setting indexes";

//    max_index_ = containers_.size()*size_per_container_-1;

//    if (first_write_)
//    {
//        max_used_index_ =  org_buffer->max_used_index_;
//    }
//    else
//    {
//        max_used_index_ +=  org_buffer->getSize();
//    }

//    org_buffer->index_=0;
//    org_buffer->max_index_=0;
//    org_buffer->max_used_index_=0;

//    first_write_=false;

//    logdbg  << "Buffer: seizeBuffer: this: cont " << containers_.size() << " max_index " << max_index_ << " max_used_index " << max_used_index_ << " first " << first_write_;
//    logdbg  << "Buffer: seizeBuffer: org: cont " << org_buffer->containers_.size() << " max_index " << org_buffer->max_index_ << " max_used_index " << org_buffer->max_used_index_ << " first " << org_buffer->first_write_;

//    if (search_active_)
//    {
//        updateContainerKeyInfo ();
//    }

    logdbg  << "Buffer: seizeBuffer: end";
}

//Buffer *Buffer::transferData ()
//{
//    assert (isFull());
//    Buffer *buffer = new Buffer (member_list_, dbo_type_);
//    buffer->seizeBuffer(this);
//    init();

//    if (search_active_)
//    {
//        buffer->activateKeySearch (search_key_pos_);

//        updateContainerKeyInfo ();
//    }

//    return buffer;
//}

/**
 * Creates exact copy of this buffer, but data contents are shallow copied by getting shallow copies of the PropertyContainers.
 */
Buffer *Buffer::getShallowCopy ()
{
    //TODO FIXME
    assert (false);
    return new Buffer();
//    Buffer *shallow_copy = new Buffer (member_list_, dbo_type_);
//    shallow_copy->id_ = id_;

//    shallow_copy->index_=index_;
//    shallow_copy->max_index_=max_index_;
//    shallow_copy->max_used_index_=max_used_index_;
//    shallow_copy->index_container_=index_container_;
//    shallow_copy->index_in_container_=index_in_container_;
//    shallow_copy->size_per_container_ = size_per_container_;

//    for (unsigned int cnt=0; cnt < containers_.size(); cnt++)
//    {
//        shallow_copy->containers_.push_back(containers_.at(cnt)->getShallowCopy());
//    }

//    shallow_copy->first_write_=first_write_;
//    shallow_copy->empty_=empty_;
//    shallow_copy->adresses_=adresses_;
//    shallow_copy->last_one_=last_one_;

//    assert (shallow_copy->containers_.size() == containers_.size());

//    if (search_active_)
//    {
//        shallow_copy->activateKeySearch (search_key_pos_);
//    }

//    return shallow_copy;
}

bool Buffer::full ()
{
    return size()%BUFFER_ARRAY_SIZE == 0;
}
/**
 * Only to be called if all values have been written into buffer. Key search is not updated on by
 * setIndex or incrementBuffer!
 */
//void Buffer::activateKeySearch (unsigned int key_pos)
//{
//    assert (!search_active_);
//    assert (key_pos < member_list_.getNumProperties());
//    search_active_=true;
//    search_key_pos_=key_pos;

//    if (containers_.size() > 0)
//        updateContainerKeyInfo ();
//}

/**
 * \exception std::runtime_error if key is not monotonous.
 */
//void Buffer::updateContainerKeyInfo ()
//{
//    assert (search_active_);
//    assert (search_key_pos_ >= 0);

//    if (containers_.size() == 0)
//        return;

//    search_key_min_ = containers_.front()->getMinKey(search_key_pos_);
//    search_key_max_ = containers_.back()->getMaxKey(search_key_pos_);

//    if (search_key_min_ >= search_key_max_)
//    {
//        logerr  << "Buffer: updateContainerKeyInfo: new min " << search_key_min_ << " >= max " << search_key_max_ << " containers " << containers_.size();
//        throw std::runtime_error ("Buffer: updateContainerKeyInfo: min >= max");
//    }
//}

//bool Buffer::setIndexForKey (unsigned int key)
//{
//    assert (search_active_);
//    assert (search_key_pos_ >= 0);
//    assert (search_key_min_ > 0 && search_key_max_ > 0);
//    assert (containers_.size() > 0);

//    if (key < (unsigned int)search_key_min_ || key > (unsigned int)search_key_max_)
//    {
//        logdbg  << "Buffer: setIndexForKey: key " << key << " out of bounds, min " << search_key_min_ << " max " << search_key_max_;
//        return false;
//    }
//    assert (search_key_min_ < search_key_max_);

//    //  boost::posix_time::ptime start_time;
//    //  boost::posix_time::ptime stop_time;
//    //
//    //  start_time= boost::posix_time::microsec_clock::local_time();

//    float index_range_step = ((float)search_key_max_ - search_key_min_+1)/(float)containers_.size();
//    unsigned int property_container_index = floor((float)(key-search_key_min_)/index_range_step); //roundToNearest

//    if (property_container_index >= containers_.size())
//    {
//        logerr << "Buffer: setIndexForKey: property_container_index too big " << property_container_index << " index_range_step "
//                << index_range_step << " range " << ((float)search_key_max_ - search_key_min_) << " containers " <<
//                (float)containers_.size();
//        return false;
//    }

//    //loginf  << "Buffer: setIndexForKey: while loop";
//    bool found=false;
//    PropertyContainer *container;
//    unsigned int size = containers_.size();
//    while (property_container_index >= 0 && property_container_index < size)
//    {
//        container = containers_.at(property_container_index);
//        //loginf  << "Buffer: setIndexForKey: while loop iteration";
//        if (key >= container->getMinKey(search_key_pos_) && key <= container->getMaxKey(search_key_pos_)) // in this one
//        {
//            int index = container->getIndexForKey (search_key_pos_, key);
//            if (index >= 0)
//            {
//                //loginf  << "Buffer: setIndexForKey: found, setting index " << index;
//                setIndex (property_container_index*size_per_container_+index); //(index_/size_per_container_
//                found=true;
//                break;
//            }
//            else // not found
//            {
//                //loginf  << "Buffer: setIndexForKey: not found";
//                break;
//            }
//        }
//        else // has to be above
//        {
//            if (key < container->getMinKey(search_key_pos_)) // in between
//            {
//                //loginf  << "Buffer: setIndexForKey: decrement index";
//                property_container_index--;
//            }
//            else
//            {
//                //loginf  << "Buffer: setIndexForKey: increment index";
//                property_container_index++;
//            }
//        }
//    }


//    //  stop_time= boost::posix_time::microsec_clock::local_time();
//    //  boost::posix_time::time_duration diff =stop_time - start_time;
//    //loginf  << "Buffer: setIndexForKey: found " << found << " key " << key << " after " << diff.total_milliseconds() << " ms ";

//    if (found)
//        assert (*((unsigned int*)getAdresses()->at(search_key_pos_)) == key);

//    return found;
//}

//void Buffer::setPropertyValuesNan (unsigned int property_index)
//{
//    assert (property_index < member_list_.getNumProperties());
//    std::vector <PropertyContainer *>::iterator it;

//    for (it =  containers_.begin(); it !=  containers_.end(); it++)
//    {
//        (*it)->setPropertyValuesNan(property_index);
//    }
//}

//bool Buffer::isMaxIndex ()
//{
//    return index_ == max_used_index_;
//}

//void Buffer::deepCopyRecordFrom (Buffer *src)
//{
//    assert (src);
//    std::vector <Property*> *srcprops = src->getPropertyList()->getProperties ();
//    unsigned int size = srcprops->size();
//    assert (size <= adresses_->size());

//    std::vector <void*> *srcadresses = src->getAdresses();

//    for (unsigned int cnt=0; cnt < size; cnt++)
//    {
////        loginf << "Buffer: deepCopyRecordFrom: num " << cnt << " name " << srcprops->at(cnt)->id_ << " type " <<
////                srcprops->at(cnt)->data_type_int_;
//        Utils::Data::copy(srcadresses->at(cnt), adresses_->at(cnt), srcprops->at(cnt)->data_type_int_, srcprops->at(cnt)->size_, false, false);
//    }
//}

const size_t Buffer::size ()
{
    size_t size = 0;

    for (auto it : arrays_bool_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_char_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_uchar_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_int_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_uint_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_long_int_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_ulong_int_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_float_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_double_)
        if (it.second->size() > size)
            size = it.second->size();
    for (auto it : arrays_string_)
        if (it.second->size() > size)
            size = it.second->size();

    //loginf << "Buffer: size: " << size;
    return size;
}

bool Buffer::firstWrite ()
{
    std::vector <ArrayListBase *>::const_iterator it;

    for (auto it : arrays_bool_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_char_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_uchar_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_int_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_uint_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_long_int_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_ulong_int_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_float_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_double_)
        if (it.second->size() > 0)
            return false;
    for (auto it : arrays_string_)
        if (it.second->size() > 0)
            return false;

    return true;
}

bool Buffer::isNone (const Property& property, unsigned int row_cnt)
{
    switch (property.dataType())
    {
    case PropertyDataType::BOOL:
        assert (arrays_bool_.count(property.name()));
        return arrays_bool_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::CHAR:
        assert (arrays_char_.count(property.name()));
        return arrays_char_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::UCHAR:
        assert (arrays_uchar_.count(property.name()));
        return arrays_uchar_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::INT:
        assert (arrays_int_.count(property.name()));
        return arrays_int_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::UINT:
        assert (arrays_uint_.count(property.name()));
        return arrays_uint_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::LONGINT:
        assert (arrays_long_int_.count(property.name()));
        return arrays_long_int_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::ULONGINT:
        assert (arrays_ulong_int_.count(property.name()));
        return arrays_ulong_int_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::FLOAT:
        assert (arrays_float_.count(property.name()));
        return arrays_float_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::DOUBLE:
        assert (arrays_double_.count(property.name()));
        return arrays_double_.at(property.name())->isNone(row_cnt);
    case PropertyDataType::STRING:
        assert (arrays_string_.count(property.name()));
        return arrays_string_.at(property.name())->isNone(row_cnt);
    default:
        logerr  <<  "Buffer: isNone: unknown property type " << Property::asString(property.dataType());
        throw std::runtime_error ("Buffer: isNone: unknown property type "+Property::asString(property.dataType()));
    }

}
