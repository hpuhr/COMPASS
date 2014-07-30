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
 * ArrayTemplateManager.h
 *
 *  Created on: Dec 6, 2011
 *      Author: sk
 */

#ifndef ARRAYTEMPLATEMANAGER_H_
#define ARRAYTEMPLATEMANAGER_H_

#include <vector>
#include <boost/thread/mutex.hpp>

#include "ArrayTemplate.h"
#include "Global.h"
#include "Logger.h"
#include "String.h"

/**
 * @brief Common interface of ArrayTemplateManagers
 *
 * Provides management functions for a ArrayTemplateBase (a.k.a. page) collection.
 */
class ArrayTemplateManagerBase
{
public:
  /// @brief Constructor
	ArrayTemplateManagerBase () {};
	/// @brief Desctructor
	virtual ~ArrayTemplateManagerBase () {};

	/// @brief Returns index of a free page
	virtual unsigned int getFreeIndex ()=0;
	/// @brief Returns free page pointer
	virtual ArrayTemplateBase *getFree ()=0;
	/// @brief Returns page at index i
	virtual ArrayTemplateBase * at (const unsigned int i)=0;
	/// @brief Marks page at index i as unused
	virtual void freeIndex (unsigned int i)=0;
	/// @brief Returns size of one element in bytes
	virtual unsigned int getBaseSizeInBytes ()=0;
	/// @brief Returns number of used pages
	virtual unsigned int getUsedPages ()=0;
	/// @brief Returns number of free pages
	virtual unsigned int getFreePages ()=0;
	//virtual unsigned int getMemSizeInBytes ()=0;
};

/**
 * @brief Template for memory page collection management, derived from ArrayTemplateManagerBase
 *
 * Manager for collection of ArrayTemplateBase (a.k.a. page) of a specific data type. Used for fast manual memory management.
 * Thread safe.
 */
template <class T>
class ArrayTemplateManager : public ArrayTemplateManagerBase
{
public:
	ArrayTemplateManager(PROPERTY_DATA_TYPE type, unsigned int size)
  {
    logdbg  << "MemoryPageTemplateManager: contructor: start";
    size_=size;
    type_=type;
    logdbg  << "MemoryPageTemplateManager: contructor: end";
  };
  virtual ~ArrayTemplateManager()
  {
    logdbg  << "MemoryPageTemplateManager: destructor";
  };

  ArrayTemplateBase * at (const unsigned int i)
  {
  	logdbg  << "MemoryPageTemplateManager: at: start";
  	boost::mutex::scoped_lock l(mutex_);
  	assert (i < pages_.size());

  	logdbg  << "MemoryPageTemplateManager: at: end";
    return (ArrayTemplateBase *) (pages_.at(i));
  };

  ArrayTemplateBase * getFree ()
  {
  	logdbg  << "MemoryPageTemplateManager: getFree: start";
  	boost::mutex::scoped_lock l(mutex_);

  	unsigned int index=0;

  	if (pages_indexes_free_.size() > 0)
  	{
  		index = pages_indexes_free_.back();
  		pages_indexes_free_.pop_back();
  		pages_.at(index)->clear();
  		return pages_.at(index);
  	}

  	// no free page fount
  	index = createFreeMemoryPage ();

		logdbg  << "MemoryPageTemplateManager: getFree: end";
		return pages_.at(index);
  };

  unsigned int getFreeIndex ()
  {
  	logdbg  << "MemoryPageTemplateManager: getFreeIndex: start";
  	typename std::vector<ArrayTemplate<T>*>::iterator it;

  	boost::mutex::scoped_lock l(mutex_);

  	unsigned int index=0;

  	if (pages_indexes_free_.size() > 0)
  	{
  		index = pages_indexes_free_.back();
  		pages_indexes_free_.pop_back();
  		pages_.at(index)->clear();
  		return index;
  	}

  	// no free page fount
  	index = createFreeMemoryPage ();
		logdbg  << "MemoryPageTemplateManager: getFreeIndex: end";
		return index;
  };

  void freeIndex (unsigned int i)
  {
  	logdbg  << "MemoryPageTemplateManager: freeIndex: start: i " << i;
  	assert (i < pages_.size());

  	boost::mutex::scoped_lock l(mutex_);

  	std::vector<unsigned int>::iterator it;

  	it = find(pages_indexes_free_.begin(), pages_indexes_free_.end(), i);

  	if (it != pages_indexes_free_.end())
  	{
  		throw std::runtime_error ("MemoryTemplateManager: freeIndex: index already free");
  	}

  	pages_indexes_free_.push_back(i);
  	logdbg  << "MemoryPageTemplateManager: freeIndex: end";
  }

  void clear ()
  {
  	logdbg  << "MemoryPageTemplateManager: clear: start";
  	typename std::vector <ArrayTemplate<T>*>::iterator it;
  	boost::mutex::scoped_lock l(mutex_);

  	unsigned int cnt=0;
  	for (it=pages_.begin(); it != pages_.end(); it++)
  	{
  		assert (*it);
  		delete *it;
  	}
  	pages_.clear();
  	pages_indexes_free_.clear();
  	logdbg  << "MemoryPageTemplateManager: clear: end";
  }
  unsigned int getBaseSizeInBytes ()
  {
  	return sizeof (T);
  }

	unsigned int getUsedPages ()
	{
		return pages_.size() - pages_indexes_free_.size();
	}
	unsigned int getFreePages ()
	{
		return pages_indexes_free_.size();
	}

//	virtual unsigned int getMemSizeInBytes ()
//	{
//	    return pages_.size()*size_*sizeof (T);
//	}

private:
	/// Number of elements
  unsigned int size_;
  /// Container with all allocated pages
  std::vector <ArrayTemplate<T>*> pages_;
  /// Container for free page indexes
  std::vector <unsigned int> pages_indexes_free_;
  /// Thread protection
  boost::mutex mutex_;
  /// Data type of elements
  PROPERTY_DATA_TYPE type_;

  /// @brief Allocates and adds new page, returns its index
  unsigned int createFreeMemoryPage ()
  {
  	ArrayTemplate<T> *tmp = new ArrayTemplate<T> (type_, size_);
  	assert (tmp);
  	pages_.push_back (tmp);

  	return pages_.size()-1;
  };
};

/**
 * @brief Common class for instantiation of different types of ArrayTemplateManager
 *
 * \todo Check if factory pattern would be more elegant.
 */
class ArrayManagerBase
{
private:
  /// Number of elements in each page
	unsigned int size_;
	/// Data type of elements
	PROPERTY_DATA_TYPE type_;
	/// Reference to generated page manager
	ArrayTemplateManagerBase *manager_;
public:
	/**
	 * @brief Constructor
	 *
	 * Initializes members, creates manager_ based data type_.
	 *
	 * \exception std::runtime_error if unknown data type
	 */
	ArrayManagerBase (PROPERTY_DATA_TYPE type, unsigned int size)
	{
		type_=type;
		size_=size;
		manager_=0;

		switch (type_)
		{
			case P_TYPE_BOOL:
				manager_ = new ArrayTemplateManager<bool> (type_, size_);
				break;
			case P_TYPE_CHAR:
				manager_ = new ArrayTemplateManager<char> (type_, size_);
				break;
			case P_TYPE_INT:
				manager_ = new ArrayTemplateManager<int> (type_, size_);
				break;
			case P_TYPE_UCHAR:
				manager_ = new ArrayTemplateManager<unsigned char> (type_, size_);
				break;
			case P_TYPE_UINT:
				manager_ = new ArrayTemplateManager<unsigned int> (type_, size_);
				break;
            case P_TYPE_LONGINT:
                manager_ = new ArrayTemplateManager<long int> (type_, size_);
                break;
            case P_TYPE_ULONGINT:
                manager_ = new ArrayTemplateManager<unsigned long int> (type_, size_);
                break;
			case P_TYPE_STRING:
				manager_ = new ArrayTemplateManager<std::string> (type_, size_);
				break;
			case P_TYPE_FLOAT:
				manager_ = new ArrayTemplateManager<float> (type_, size_);
				break;
			case P_TYPE_DOUBLE:
				manager_ = new ArrayTemplateManager<double> (type_, size_);
				break;
      case P_TYPE_POINTER:
        manager_ = new ArrayTemplateManager<void*> (type_, size_);
        break;
			case P_TYPE_SENTINEL:
				throw std::runtime_error("ArrayManagerBase: constructor: data type sentinel requested");
				break;

			default:
				throw std::runtime_error("ArrayManagerBase: constructor: unspecified data type "+Utils::String::intToString(type_));
		}
	};

	/// @brief Destructor. Deletes the manager_.
	virtual ~ArrayManagerBase ()
	{
	  assert (manager_);
	  delete manager_;
	  manager_=0;
	};

	/// @brief Returns manager_
	ArrayTemplateManagerBase *getManager ()
	{
		assert (manager_);
		return manager_;
	}

	/// @brief Returns base size of element in manager_
  unsigned int getBaseSizeInBytes ()
  {
  	return manager_->getBaseSizeInBytes();
  }
};


#endif /* MEMORYPAGETEMPLATEMANAGER_H_ */
