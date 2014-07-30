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

#include "Buffer.h"
#include "Logger.h"
#include "MemoryManager.h"
#include "PropertyContainer.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "Data.h"
#include "String.h"

unsigned int Buffer::ids_ = 0;



/**
 * Creates a buffer from a PropertyList and a DBO type. Sets member to initial values and calls init().
 *
 * \param member_list PropertyList defining all properties
 * \param type DBO type
 */
Buffer::Buffer(PropertyList member_list, DB_OBJECT_TYPE type)
: member_list_(member_list), dbo_type_(type), index_(0), max_index_(0), max_used_index_(0),
  size_per_container_(MemoryManager::getInstance().getSize()), last_one_(false), search_active_(false), search_key_pos_(-1),
  search_key_min_ (-1), search_key_max_ (-1)
{
    logdbg  << "Buffer: constructor: start";

    id_ = ids_;
    ++ids_;

    init();

    logdbg  << "Buffer: constructor: end";
}

/**
 * Calls clear.
 */
Buffer::~Buffer()
{
    logdbg  << "Buffer: destructor: start";
    clear ();
    logdbg  << "Buffer: destructor: end";
}

void Buffer::init()
{
    assert (index_==0);
    assert (max_index_==0);
    assert (max_used_index_==0);

    adresses_=0;

    first_write_=true;
    empty_=true;
}

/**
 * If emtpy, one page is allocated. If index exceeds currently allocated memory, allocateUpTo is called. Then, various index
 * variables are set, setIndex is called on the appropriate PropertyContainer and adresses_ is retrieved from the same.
 */
void Buffer::setIndex (unsigned int index)
{
    logdbg  << "Buffer: setIndex: start";

    if (empty_)
    {
        allocateOnePage();
        adresses_ = containers_.at(0)->getAdresses();
        empty_=false;
    }

    if (index > max_index_)
    {
        allocateUpTo(index);
    }

    assert (index <= max_index_);

    if (index > max_used_index_)
        max_used_index_ = index;

    index_=index;
    index_container_ = (unsigned int) (index_/size_per_container_);
    index_in_container_= (unsigned int) (index_%size_per_container_);

    logdbg  << "Buffer: setIndex: index " << index_ << " index_container " << index_container_ << " index_in_container " << index_in_container_ << " containers " << containers_.size();

    if (index_!=0)
        assert (index_container_ < containers_.size());

    containers_.at(index_container_)->setIndex(index_in_container_);
    adresses_ = containers_.at(index_container_)->getAdresses();

    logdbg  << "Buffer: setIndex: end";
}
void *Buffer::get (unsigned int property)
{
    logdbg  << "Buffer: get1: start";

    if (empty_)
    {
        setIndex(0);
    }

    assert (index_container_ < containers_.size());

    logdbg  << "Buffer: get1: end";
    return containers_.at(index_container_)->get (property);
}

/**
 * May return 0 if setIndex was not called.
 */
std::vector<void *>* Buffer::getAdresses()
{
    if (empty_)
    {
        setIndex(0);
    }

    return adresses_;
}


void *Buffer::get (unsigned int index, unsigned int property)
{
    logdbg  << "Buffer: get2: start";
    setIndex (index);
    logdbg  << "Buffer: get2: end";
    return get (property);
}

/**
 * Can be used for high performance read/write.
 */
void Buffer::incrementIndex ()
{
    logdbg  << "Buffer: incrementIndex: start";

    if (empty_)
    {
        setIndex(0);
    }

    if (!containers_.at(index_container_)->isMaxIndex())
    {
        index_+=1;
        index_in_container_+=1;
        containers_.at(index_container_)->incrementIndex ();
        if (index_ > max_used_index_)
            max_used_index_=index_;
    }
    else
        setIndex (index_+1); // allocates on its own

    first_write_=false;

    logdbg  << "Buffer: incrementIndex: end";
}

/**
 * Can be used for high performance read/write.
 */
void Buffer::decrementIndex ()
{
    logdbg  << "Buffer: incrementIndex: start";

    assert (!empty_);
    assert (index_ != 0);

    if (!containers_.at(index_container_)->isMinIndex())
    {
        index_-=1;
        index_in_container_-=1;
        containers_.at(index_container_)->decrementIndex ();
    }
    else
        setIndex (index_-1); // allocates on its own

    first_write_=false;

    logdbg  << "Buffer: incrementIndex: end";
}

void Buffer::clear ()
{
    logdbg  << "Buffer: clear: start type " << dbo_type_ << " size " << getSize();

    logdbg  << "Buffer: clear: deleting containers";
    for (unsigned int cnt=0; cnt < containers_.size(); cnt ++)
    {
        delete containers_.at(cnt);
    }

    logdbg  << "Buffer: clear: clearing containers";
    containers_.clear();
    logdbg  << "Buffer: clear: end";

}
void Buffer::allocateUpTo (unsigned int index)
{
    logdbg  << "Buffer: allocateUpTo: start";
    while (max_index_ < index)
        allocateOnePage ();
    logdbg  << "Buffer: allocateUpTo: end";
}

void Buffer::allocateOnePage ()
{
    logdbg  << "Buffer: allocateOnePage: start";

    PropertyContainer *cont = new PropertyContainer (member_list_);
    assert (cont);
    containers_.push_back (cont);
    max_index_ = containers_.size()*size_per_container_-1;
    logdbg  << "Buffer: allocateOnePage: end";
}

/**
 * \exception std::runtime_error if unknown property type
 */
void Buffer::print (unsigned int num_elements)
{
    Property *prop=0;
    unsigned int index_old=index_;

    if (num_elements > getSize())
        num_elements = getSize();

    unsigned int num_properties = member_list_.getNumProperties();
    std::vector <Property*> * properties= member_list_.getProperties();

    std::stringstream ss;

    for (unsigned int cnt=0; cnt < num_properties; cnt++)
    {
        prop=properties->at(cnt);

        ss << "(" << PROPERTY_DATA_TYPE_STRINGS[(PROPERTY_DATA_TYPE)prop->data_type_int_] << ") " << prop->id_ << "  ";
    }

    loginf  << "Buffer: print: properties: " << ss.str();


    setIndex (0);
    for (unsigned int index=0; index < num_elements; index++)
    {
        if (index != 0)
            incrementIndex();

        ss.str(std::string());

        for (unsigned int cnt=0; cnt < num_properties; cnt++)
        {
            prop=properties->at(cnt);

            ss << Utils::String::getPropertyValueString(adresses_->at(cnt), (PROPERTY_DATA_TYPE)prop->data_type_int_) << " ";
        }
        loginf  << "Buffer: print: data "<< index <<": " << ss.str();
    }
    setIndex(index_old);
}

void Buffer::printLong (unsigned int num_elements)
{
    Property *prop=0;
    unsigned int index_old=index_;

    if (num_elements > getSize())
        num_elements = getSize();

    unsigned int num_properties = member_list_.getNumProperties();
    std::vector <Property*> * properties= member_list_.getProperties();

    std::stringstream ss;

//    for (unsigned int cnt=0; cnt < num_properties; cnt++)
//    {
//        prop=properties->at(cnt);
//
//
//    }
//
//    loginf  << "Buffer: print: properties: " << ss.str();


    setIndex (0);
    for (unsigned int index=0; index < num_elements; index++)
    {
        if (index != 0)
            incrementIndex();

        ss.str(std::string());

        for (unsigned int cnt=0; cnt < num_properties; cnt++)
        {
            prop=properties->at(cnt);

            ss << "(" << PROPERTY_DATA_TYPE_STRINGS[(PROPERTY_DATA_TYPE)prop->data_type_int_] << ") " << prop->id_ << "  ";

            ss << Utils::String::getPropertyValueString(adresses_->at(cnt), (PROPERTY_DATA_TYPE)prop->data_type_int_) << std::endl;
        }
        loginf  << "Buffer: print: data "<< index <<": " << ss.str();
    }
    setIndex(index_old);
}


/**
 * \param id Unique property identifier
 * \param type Property data type
 *
 * \exception std::runtime_error if property id already in use
 */
void Buffer::addProperty (std::string id, PROPERTY_DATA_TYPE type)
{
    logdbg  << "Buffer: addProperty: start";
    logdbg  << "Buffer: addProperty:  id '" << id << "' type " << type;
    assert (!id.empty());
    assert (type < P_TYPE_SENTINEL);

    if (member_list_.hasProperty(id))
        throw std::runtime_error ("Buffer: addProperty: property "+id+" already exists");
    member_list_.addProperty(id,type);

    for (unsigned int cnt=0; cnt < containers_.size(); cnt++)
    {
        containers_.at(cnt)->addProperty (id, type);
    }

    setIndex(index_);

    logdbg  << "Buffer: addProperty: end";
};

void Buffer::seizeBuffer (Buffer *org_buffer)
{
    logdbg  << "Buffer: seizeBuffer: start";

    logdbg  << "Buffer: seizeBuffer: this: cont " << containers_.size() << " max_index " << max_index_ << " max_used_index " << max_used_index_ << " first " << first_write_;
    logdbg  << "Buffer: seizeBuffer: org: cont " << org_buffer->containers_.size() << " max_index " << org_buffer->max_index_ << " max_used_index " << org_buffer->max_used_index_ << " first " << org_buffer->first_write_;

    assert (org_buffer);
    assert (isFull() || first_write_);

    logdbg  << "Buffer: seizeBuffer: containers";
    std::vector <PropertyContainer *> org_containers = org_buffer->containers_;

    logdbg  << "Buffer: seizeBuffer: inserting ";
    containers_.insert (containers_.end(), org_containers.begin(), org_containers.end());
    org_buffer->containers_.clear();
    //all your containers belong to us

    logdbg  << "Buffer: seizeBuffer: setting indexes";

    max_index_ = containers_.size()*size_per_container_-1;

    if (first_write_)
    {
        max_used_index_ =  org_buffer->max_used_index_;
    }
    else
    {
        max_used_index_ +=  org_buffer->getSize();
    }

    org_buffer->index_=0;
    org_buffer->max_index_=0;
    org_buffer->max_used_index_=0;

    first_write_=false;

    logdbg  << "Buffer: seizeBuffer: this: cont " << containers_.size() << " max_index " << max_index_ << " max_used_index " << max_used_index_ << " first " << first_write_;
    logdbg  << "Buffer: seizeBuffer: org: cont " << org_buffer->containers_.size() << " max_index " << org_buffer->max_index_ << " max_used_index " << org_buffer->max_used_index_ << " first " << org_buffer->first_write_;

    if (search_active_)
    {
        updateContainerKeyInfo ();
    }

    logdbg  << "Buffer: seizeBuffer: end";
}

Buffer *Buffer::transferData ()
{
    assert (isFull());
    Buffer *buffer = new Buffer (member_list_, dbo_type_);
    buffer->seizeBuffer(this);
    init();

    if (search_active_)
    {
        buffer->activateKeySearch (search_key_pos_);

        updateContainerKeyInfo ();
    }

    return buffer;
}

/**
 * Creates exact copy of this buffer, but data contents are shallow copied by getting shallow copies of the PropertyContainers.
 */
Buffer *Buffer::getShallowCopy ()
{
    Buffer *shallow_copy = new Buffer (member_list_, dbo_type_);
    shallow_copy->id_ = id_;

    shallow_copy->index_=index_;
    shallow_copy->max_index_=max_index_;
    shallow_copy->max_used_index_=max_used_index_;
    shallow_copy->index_container_=index_container_;
    shallow_copy->index_in_container_=index_in_container_;
    shallow_copy->size_per_container_ = size_per_container_;

    for (unsigned int cnt=0; cnt < containers_.size(); cnt++)
    {
        shallow_copy->containers_.push_back(containers_.at(cnt)->getShallowCopy());
    }

    shallow_copy->first_write_=first_write_;
    shallow_copy->empty_=empty_;
    shallow_copy->adresses_=adresses_;
    shallow_copy->last_one_=last_one_;

    assert (shallow_copy->containers_.size() == containers_.size());

    if (search_active_)
    {
        shallow_copy->activateKeySearch (search_key_pos_);
    }

    return shallow_copy;
}

bool Buffer::isFull ()
{
    return (max_used_index_ == max_index_);
}
/**
 * Only to be called if all values have been written into buffer. Key search is not updated on by
 * setIndex or incrementBuffer!
 */
void Buffer::activateKeySearch (unsigned int key_pos)
{
    assert (!search_active_);
    assert (key_pos < member_list_.getNumProperties());
    search_active_=true;
    search_key_pos_=key_pos;

    if (containers_.size() > 0)
        updateContainerKeyInfo ();
}

/**
 * \exception std::runtime_error if key is not monotonous.
 */
void Buffer::updateContainerKeyInfo ()
{
    assert (search_active_);
    assert (search_key_pos_ >= 0);

    if (containers_.size() == 0)
        return;

    search_key_min_ = containers_.front()->getMinKey(search_key_pos_);
    search_key_max_ = containers_.back()->getMaxKey(search_key_pos_);

    if (search_key_min_ >= search_key_max_)
    {
        logerr  << "Buffer: updateContainerKeyInfo: new min " << search_key_min_ << " >= max " << search_key_max_ << " containers " << containers_.size();
        throw std::runtime_error ("Buffer: updateContainerKeyInfo: min >= max");
    }
}

bool Buffer::setIndexForKey (unsigned int key)
{
    assert (search_active_);
    assert (search_key_pos_ >= 0);
    assert (search_key_min_ > 0 && search_key_max_ > 0);
    assert (containers_.size() > 0);

    if (key < (unsigned int)search_key_min_ || key > (unsigned int)search_key_max_)
    {
        logdbg  << "Buffer: setIndexForKey: key " << key << " out of bounds, min " << search_key_min_ << " max " << search_key_max_;
        return false;
    }
    assert (search_key_min_ < search_key_max_);

    //  boost::posix_time::ptime start_time;
    //  boost::posix_time::ptime stop_time;
    //
    //  start_time= boost::posix_time::microsec_clock::local_time();

    float index_range_step = ((float)search_key_max_ - search_key_min_+1)/(float)containers_.size();
    unsigned int property_container_index = floor((float)(key-search_key_min_)/index_range_step); //roundToNearest

    if (property_container_index >= containers_.size())
    {
        logerr << "Buffer: setIndexForKey: property_container_index too big " << property_container_index << " index_range_step "
                << index_range_step << " range " << ((float)search_key_max_ - search_key_min_) << " containers " <<
                (float)containers_.size();
        return false;
    }

    //loginf  << "Buffer: setIndexForKey: while loop";
    bool found=false;
    PropertyContainer *container;
    unsigned int size = containers_.size();
    while (property_container_index >= 0 && property_container_index < size)
    {
        container = containers_.at(property_container_index);
        //loginf  << "Buffer: setIndexForKey: while loop iteration";
        if (key >= container->getMinKey(search_key_pos_) && key <= container->getMaxKey(search_key_pos_)) // in this one
        {
            int index = container->getIndexForKey (search_key_pos_, key);
            if (index >= 0)
            {
                //loginf  << "Buffer: setIndexForKey: found, setting index " << index;
                setIndex (property_container_index*size_per_container_+index); //(index_/size_per_container_
                found=true;
                break;
            }
            else // not found
            {
                //loginf  << "Buffer: setIndexForKey: not found";
                break;
            }
        }
        else // has to be above
        {
            if (key < container->getMinKey(search_key_pos_)) // in between
            {
                //loginf  << "Buffer: setIndexForKey: decrement index";
                property_container_index--;
            }
            else
            {
                //loginf  << "Buffer: setIndexForKey: increment index";
                property_container_index++;
            }
        }
    }


    //  stop_time= boost::posix_time::microsec_clock::local_time();
    //  boost::posix_time::time_duration diff =stop_time - start_time;
    //loginf  << "Buffer: setIndexForKey: found " << found << " key " << key << " after " << diff.total_milliseconds() << " ms ";

    if (found)
        assert (*((unsigned int*)getAdresses()->at(search_key_pos_)) == key);

    return found;
}

void Buffer::setPropertyValuesNan (unsigned int property_index)
{
    assert (property_index < member_list_.getNumProperties());
    std::vector <PropertyContainer *>::iterator it;

    for (it =  containers_.begin(); it !=  containers_.end(); it++)
    {
        (*it)->setPropertyValuesNan(property_index);
    }
}

bool Buffer::isMaxIndex ()
{
    return index_ == max_used_index_;
}

void Buffer::deepCopyRecordFrom (Buffer *src)
{
    assert (src);
    std::vector <Property*> *srcprops = src->getPropertyList()->getProperties ();
    unsigned int size = srcprops->size();
    assert (size <= adresses_->size());

    std::vector <void*> *srcadresses = src->getAdresses();

    for (unsigned int cnt=0; cnt < size; cnt++)
    {
//        loginf << "Buffer: deepCopyRecordFrom: num " << cnt << " name " << srcprops->at(cnt)->id_ << " type " <<
//                srcprops->at(cnt)->data_type_int_;
        Utils::Data::copy(srcadresses->at(cnt), adresses_->at(cnt), srcprops->at(cnt)->data_type_int_, srcprops->at(cnt)->size_, false, false);
    }
}
