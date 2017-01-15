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
#include "data.h"
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
Buffer::Buffer(PropertyList properties, std::string dbo_type)
: dbo_type_(dbo_type), last_one_(false)
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
    arrays_.clear();
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

    dbo_type_="";
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
        arrays_bool_ [id] = ArrayListTemplate<bool> ();
        arrays_.push_back(dynamic_cast<ArrayListBase *> (&arrays_bool_ [id]));
        break;
    case PropertyDataType::CHAR:
        assert (arrays_char_.count(id) == 0);
        arrays_char_ [id] = ArrayListTemplate<char> ();
        arrays_.push_back(dynamic_cast<ArrayListBase *> (&arrays_char_ [id]));
        break;
    case PropertyDataType::UCHAR:
        assert (arrays_uchar_.count(id) == 0);
        arrays_uchar_ [id] = ArrayListTemplate<unsigned char> ();
        arrays_.push_back(dynamic_cast<ArrayListBase *> (&arrays_uchar_ [id]));
        break;
    case PropertyDataType::INT:
        assert (arrays_int_.count(id) == 0);
        arrays_int_ [id] = ArrayListTemplate<int> ();
        arrays_.push_back(dynamic_cast<ArrayListBase *> (&arrays_int_ [id]));
        break;
    case PropertyDataType::UINT:
        assert (arrays_uint_.count(id) == 0);
        arrays_uint_ [id] = ArrayListTemplate<unsigned int> ();
        arrays_.push_back(dynamic_cast<ArrayListBase *> (&arrays_uint_ [id]));
        break;
    case PropertyDataType::LONGINT:
        assert (arrays_long_int_.count(id) == 0);
        arrays_long_int_ [id] = ArrayListTemplate<long int> ();
        arrays_.push_back(dynamic_cast<ArrayListBase *> (&arrays_long_int_ [id]));
        break;
    case PropertyDataType::ULONGINT:
        assert (arrays_ulong_int_.count(id) == 0);
        arrays_ulong_int_ [id] = ArrayListTemplate<unsigned long int> ();
        arrays_.push_back(dynamic_cast<ArrayListBase *> (&arrays_ulong_int_ [id]));
        break;
    case PropertyDataType::FLOAT:
        assert (arrays_float_.count(id) == 0);
        arrays_float_ [id] = ArrayListTemplate<float> ();
        arrays_.push_back(dynamic_cast<ArrayListBase *> (&arrays_float_ [id]));
        break;
    case PropertyDataType::DOUBLE:
        assert (arrays_double_.count(id) == 0);
        arrays_double_ [id] = ArrayListTemplate<double> ();
        arrays_.push_back(dynamic_cast<ArrayListBase *> (&arrays_double_ [id]));
        break;
    case PropertyDataType::STRING:
        assert (arrays_string_.count(id) == 0);
        arrays_string_ [id] = ArrayListTemplate<std::string> ();
        arrays_.push_back(dynamic_cast<ArrayListBase *> (&arrays_string_ [id]));
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
    addProperty (property.getId(), property.getDataType());
}

ArrayListTemplate<bool> &Buffer::getBool (const std::string &id)
{
    if (arrays_bool_.count(id) == 0)
        throw std::runtime_error ("Buffer: getBool: unknown id "+id);

    return arrays_bool_[id];
}

ArrayListTemplate<char> &Buffer::getChar (const std::string id)
{
    if (arrays_char_.count(id) == 0)
        throw std::runtime_error ("Buffer: getChar: unknown id "+id);

    return arrays_char_[id];
}

ArrayListTemplate<unsigned char> &Buffer::getUChar (const std::string &id)
{
    if (arrays_uchar_.count(id) == 0)
        throw std::runtime_error ("Buffer: getUChar: unknown id "+id);

    return arrays_uchar_[id];
}

ArrayListTemplate<int> &Buffer::getInt (const std::string &id)
{
    if (arrays_int_.count(id) == 0)
        throw std::runtime_error ("Buffer: getInt: unknown id "+id);

    return arrays_int_[id];
}

ArrayListTemplate<unsigned int> &Buffer::getUInt (const std::string &id)
{
    if (arrays_uint_.count(id) == 0)
        throw std::runtime_error ("Buffer: getUInt: unknown id "+id);

    return arrays_uint_[id];
}

ArrayListTemplate<long int> &Buffer::getLongInt (const std::string &id)
{
    if (arrays_long_int_.count(id) == 0)
        throw std::runtime_error ("Buffer: getLongInt: unknown id "+id);

    return arrays_long_int_[id];
}

ArrayListTemplate<unsigned long int> &Buffer::getULongInt (const std::string &id)
{
    if (arrays_ulong_int_.count(id) == 0)
        throw std::runtime_error ("Buffer: getULongInt: unknown id "+id);

    return arrays_ulong_int_[id];
}

ArrayListTemplate<float> &Buffer::getFloat (const std::string &id)
{
    if (arrays_float_.count(id) == 0)
        throw std::runtime_error ("Buffer: getFloat: unknown id "+id);

    return arrays_float_[id];
}

ArrayListTemplate<double> &Buffer::getDouble (const std::string &id)
{
    if (arrays_double_.count(id) == 0)
        throw std::runtime_error ("Buffer: getDouble: unknown id "+id);

    return arrays_double_[id];
}

ArrayListTemplate<std::string> &Buffer::getString (const std::string &id)
{
    if (arrays_string_.count(id) == 0)
        throw std::runtime_error ("Buffer: getString: unknown id "+id);

    return arrays_string_[id];
}

void Buffer::seizeBuffer (Buffer &org_buffer)
{
    // TODO
    assert (false);
//    logdbg  << "Buffer: seizeBuffer: start";

//    logdbg  << "Buffer: seizeBuffer: this: cont " << containers_.size() << " max_index " << max_index_ << " max_used_index " << max_used_index_ << " first " << first_write_;
//    logdbg  << "Buffer: seizeBuffer: org: cont " << org_buffer->containers_.size() << " max_index " << org_buffer->max_index_ << " max_used_index " << org_buffer->max_used_index_ << " first " << org_buffer->first_write_;

//    assert (org_buffer);
//    assert (isFull() || first_write_);

//    logdbg  << "Buffer: seizeBuffer: containers";
//    std::vector <PropertyContainer *> org_containers = org_buffer->containers_;

//    logdbg  << "Buffer: seizeBuffer: inserting ";
//    containers_.insert (containers_.end(), org_containers.begin(), org_containers.end());
//    org_buffer->containers_.clear();
//    //all your containers belong to us

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

//    logdbg  << "Buffer: seizeBuffer: end";
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

    std::vector <ArrayListBase *>::const_iterator it;

    for (it = arrays_.begin(); it != arrays_.end(); it++)
    {
        if (size < (*it)->size())
            size = (*it)->size();
    }
    return size;
}

bool Buffer::firstWrite ()
{
    std::vector <ArrayListBase *>::const_iterator it;

    for (it = arrays_.begin(); it != arrays_.end(); it++)
    {
        if ((*it)->size() > 0)
            return false;
    }

    return true;
}
