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
 * PropertyContainer.cpp
 *
 *  Created on: Dec 8, 2011
 *      Author: sk
 */

#include "PropertyContainer.h"
#include "Logger.h"
#include "Global.h"
#include "MemoryManager.h"

/**
 * Initializes members, creates ArrayTemplates.
 *
 * \param member_list List with reqired properties
 * \param create_array_templates Flag indicating if ArrayTemplates should be created
 */
PropertyContainer::PropertyContainer(PropertyList member_list, bool create_array_templates)
: member_list_ (member_list)
{
  logdbg  << "PropertyContainer: constructor: start";
  size_=MemoryManager::getInstance().getSize();

  index_=0;
  max_index_=MemoryManager::getInstance().getSize()-1;
  max_used_index_=0;


  if (create_array_templates)
  {
    createArrayTemplates();
    assert (array_templates_.size() == array_template_is_shallow_copy_.size());

    unsigned int num = member_list_.getNumProperties();

    for (unsigned int cnt=0; cnt < num; cnt++)
    {
      size_increments_.push_back(array_templates_.at(cnt)->getSize());
      base_addresses_.push_back(array_templates_.at(cnt)->getArrayBasePointer());
      addresses_.push_back(array_templates_.at(cnt)->getArrayBasePointer());
    }
  }
  logdbg  << "PropertyContainer: constructor: end";
}

/**
 * Call clear.
 */
PropertyContainer::~PropertyContainer()
{
  logdbg  << "PropertyContainer: destructor: start";
  clear ();
  logdbg  << "PropertyContainer: destructor: end";
}

/**
 * Iterates through all Properties, creates an ArrayTemplate for each one and fills the managment containers.
 */
void PropertyContainer::createArrayTemplates ()
{
  logdbg  << "PropertyContainer: createArrayTemplates: start";

  std::vector <Property*>::iterator it;

  for (it =member_list_.getProperties()->begin(); it != member_list_.getProperties()->end(); it++)
  {
    Property *current = *it;
    assert (current);
    int free = MemoryManager::getInstance().getArrayTemplateManager((PROPERTY_DATA_TYPE) current->data_type_int_)->getFreeIndex();
    ArrayTemplateBase *tmp = MemoryManager::getInstance().getArrayTemplateManager((PROPERTY_DATA_TYPE) current->data_type_int_)->at(free);
    assert (tmp);
    array_templates_.push_back (tmp);
    array_template_is_shallow_copy_.push_back(false);
    array_templates_indexes_.push_back (free);
  }
  logdbg  << "PropertyContainer: createArrayTemplates: end";
}

/**
 * Iterates through all ArrayTemplates, deletes those which are not shallow copies, clear management containers.
 */
void PropertyContainer::clear ()
{
  logdbg  << "PropertyContainer: clear: start properties " << array_templates_.size();

  assert (array_templates_.size() == array_template_is_shallow_copy_.size());
  if (!(array_templates_.size() == array_templates_indexes_.size()))
  {
    logerr << " at" << array_templates_.size() << "ati " << array_templates_indexes_.size();
    assert (false);
  }
  for (unsigned int cnt=0; cnt < array_templates_.size(); cnt++)
  {
    unsigned int index_del = array_templates_indexes_.at(cnt);
    if (!array_template_is_shallow_copy_.at(cnt))
    {
      if (!MemoryManager::getInstance().getConstructed())
      {
        logerr  << "PropertyContainer: clear: MemoryManager already destroyed";
        break;
      }
      logdbg  << "PropertyContainer: clear: deleting " << index_del << " in MemoryManager";
      MemoryManager::getInstance().getArrayTemplateManager((PROPERTY_DATA_TYPE)  member_list_.getProperty(cnt)->data_type_int_)->freeIndex(index_del);
    }
    else
    {
      //logdbg  << "PropertyContainer: clear: not deleting " << array_templates_indexes_.at(cnt) << " since shallow";
    }
  }

  array_templates_.clear();
  array_template_is_shallow_copy_.clear();
  array_templates_indexes_.clear();
  logdbg  << "PropertyContainer: clear: end";
}

void PropertyContainer::setPropertyValuesNan (unsigned int property_index)
{
  assert (property_index < array_templates_.size());
  array_templates_.at(property_index)->setArrayNan();
}

/**
 * Performes checks, and sets addresses.
 */
void PropertyContainer::setIndex (unsigned int index) // set index of element adresses in PropertyContainer
{
  logdbg  << "PropertyContainer: setIndex: start";
  assert (index <= max_index_);
  index_=index;

  if (index_ > max_used_index_)
    max_used_index_=index_;

  assert (addresses_.size() == base_addresses_.size());
  assert (size_increments_.size() == addresses_.size());

  for (unsigned int cnt=0; cnt < addresses_.size(); cnt++)
  {
    addresses_.at(cnt) = ((unsigned char*) base_addresses_.at(cnt) + index_*size_increments_.at(cnt));
  }
  logdbg  << "PropertyContainer: setIndex: end";
}
void *PropertyContainer::get (unsigned int property) // get property data of current element
{
  logdbg  << "PropertyContainer: get1";
  assert (property < addresses_.size());

  return addresses_.at(property);
}
void *PropertyContainer::get (unsigned int index, unsigned int property)
{
  logdbg  << "PropertyContainer: get2";
  assert (index <= max_index_);
  setIndex (index);

  assert (property < addresses_.size());

  return addresses_.at(property);
}

/**
 * \exception std::runtime_error if index already at maximum
 */
void PropertyContainer::incrementIndex ()
{
  logdbg  << "PropertyContainer: incrementIndex";
  if (index_ <= max_index_)
  {
    index_++;

    if (index_ > max_used_index_)
      max_used_index_=index_;

    for (unsigned int cnt=0; cnt < addresses_.size(); cnt++)
    {
      addresses_.at(cnt) = ((unsigned char*) addresses_.at(cnt) + size_increments_.at(cnt));
    }
  }
  else
  {
    throw std::runtime_error ("PropertyContainer: incrementIndex: index already at maximum");
  }
}
/**
 * \exception std::runtime_error if index already at minimum
 */
void PropertyContainer::decrementIndex ()
{
  logdbg  << "PropertyContainer: decrementIndex";
  if (index_ > 0)
  {
    index_--;

    for (unsigned int cnt=0; cnt < addresses_.size(); cnt++)
    {
      addresses_.at(cnt) = ((unsigned char*) addresses_.at(cnt) - size_increments_.at(cnt));
    }
  }
  else
  {
    throw std::runtime_error ("PropertyContainer: decrementIndex: already min");
  }
}

bool PropertyContainer::isMaxIndex ()
{
  logdbg  << "PropertyContainer: isMaxIndex";
  return (index_ == max_index_);
}

bool PropertyContainer::isMinIndex ()
{
  logdbg  << "PropertyContainer: isMinIndex";
  return (index_ == 0);
}


void PropertyContainer::addProperty (std::string id, PROPERTY_DATA_TYPE type)
{
  logdbg  << "PropertyContainer: addProperty: start";
  logdbg  << "PropertyContainer: addProperty:  id '" << id << "' type " << type;
  assert (!id.empty());
  assert (type < P_TYPE_SENTINEL);

  assert (array_templates_.size() == array_template_is_shallow_copy_.size());

  if (member_list_.hasProperty(id))
    throw std::runtime_error ("PropertyContainer: addProperty: property "+id+" already exists");
  member_list_.addProperty(id,type);

  int free = MemoryManager::getInstance().getArrayTemplateManager(type)->getFreeIndex();
  ArrayTemplateBase *tmp = MemoryManager::getInstance().getArrayTemplateManager(type)->at(free);
  assert (tmp);
  array_templates_.push_back (tmp);
  array_template_is_shallow_copy_.push_back(false);
  array_templates_indexes_.push_back (free);

  size_increments_.push_back(tmp->getSize());
  base_addresses_.push_back(tmp->getArrayBasePointer());
  addresses_.push_back(tmp->getArrayBasePointer());

  setIndex(index_);

  logdbg  << "PropertyContainer: addProperty: end";
}

PropertyContainer *PropertyContainer::getShallowCopy ()
{
  assert (array_templates_.size() == array_template_is_shallow_copy_.size());
  assert (array_templates_.size() == array_templates_indexes_.size());

  PropertyContainer *shallow_copy = new PropertyContainer (member_list_, false);

  for (unsigned int cnt=0; cnt < array_templates_.size(); cnt++)
  {
    shallow_copy->array_templates_.push_back (array_templates_.at(cnt));
    shallow_copy->array_template_is_shallow_copy_.push_back (true);
  }
  shallow_copy->array_templates_indexes_=array_templates_indexes_;
  shallow_copy->index_=index_;
  shallow_copy->max_index_=max_index_;
  shallow_copy->max_used_index_=max_used_index_;
  shallow_copy->base_addresses_=base_addresses_;
  shallow_copy->addresses_=addresses_;
  shallow_copy->size_increments_=size_increments_;

  assert (shallow_copy->array_templates_.size() == shallow_copy->array_template_is_shallow_copy_.size());
  assert (shallow_copy->array_templates_.size() == shallow_copy->array_templates_indexes_.size());

  return shallow_copy;
}


unsigned int PropertyContainer::getMinKey (unsigned int key_pos)
{
  assert (key_pos < base_addresses_.size());
  return *(unsigned int*) ((unsigned char*) base_addresses_.at(key_pos));
}

unsigned int PropertyContainer::getMaxKey (unsigned int key_pos)
{
  assert (key_pos < base_addresses_.size());
  return *(unsigned int*) ((unsigned char*) base_addresses_.at(key_pos) + max_used_index_*size_increments_.at(key_pos));
}

int PropertyContainer::getIndexForKey (unsigned int key_pos, unsigned int key)// -1 if not found, positive is found index
{
  assert (key >= getMinKey(key_pos) && key <= getMaxKey(key_pos));

  return binarySearch(key_pos, 0, max_used_index_, key);
}

/**
 * Uses while loop to use a binary search algorithm to find a specific key value.
 *
 * Returns index of found key or -1.
 *
 * \param key_pos Property index of key
 * \param index_start First index to be searched
 * \param index_last Last index to be searched
 * \param key Key value to be found
 */
int PropertyContainer::binarySearch(unsigned int key_pos, int index_start, int index_last, unsigned int key)
{
  int mid;
  while (index_start <= index_last)
  {
    mid = (index_start + index_last) / 2; // compute mid point
    //loginf  << "PropertyContainer: binarySearch: mid " << mid << " start " << index_start << " last " << index_last << " key " << key;
    if (key > *(unsigned int*) ((unsigned char*) base_addresses_.at(key_pos) + mid*size_increments_.at(key_pos)))
      index_start = mid + 1;  // repeat search in top half.
    else if (key < *(unsigned int*) ((unsigned char*) base_addresses_.at(key_pos) + mid*size_increments_.at(key_pos)))
      index_last = mid - 1; // repeat search in bottom half.
    else
      return mid; // found it. return position
  }
  //loginf  << "PropertyContainer: binarySearch: not found mid " << mid << " start " << index_start << " last " << index_last << " key " << key;
  return -1; // failed to find key
}
