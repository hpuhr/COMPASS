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
 * ArrayTemplate.h
 *
 *  Created on: Dec 6, 2011
 *      Author: sk
 */

#ifndef ARRAYTEMPLATE_H_
#define ARRAYTEMPLATE_H_

#include <cstring>
#include "Logger.h"
#include "Data.h"

/**
 * @brief Common interface for templates for an fixed-size array
 *
 * Was written for easy management of arrays of different data types.
 */
class ArrayTemplateBase
{
public:
  /// @brief Constructor
	ArrayTemplateBase () {};
	/// @brief Destructor
	virtual ~ArrayTemplateBase () {};
	/// @brief Pointer to the first element of the array
	virtual void *getArrayBasePointer () = 0;
	/// @brief Returns size of one element
	virtual unsigned int getSize ()=0;
	/// @brief Sets all elements to 0
	virtual void clear() =0;
	/// @brief Sets all elements to NaN
	virtual void setArrayNan()=0;
};

/**
 * @brief Template for arrays of different data types, derived from ArrayTemplateBase
 *
 */
template <class T>
class ArrayTemplate : public ArrayTemplateBase
{
public:
  /**
   * @brief Constructor
   *
   * Creates a fixes size array of elements of a defined data type.
   *
   * \param type element data type
   * \param size number of elements (default 10000)
   */
	ArrayTemplate(PROPERTY_DATA_TYPE type, unsigned int size = 10000)
  {
    logdbg  << "ArrayTemplate: contructor";
    size_ = size;
    buffer_ = 0;
    type_=type;

    init ();
  };

	/**
	 * @brief Destructor
	 *
	 * Deletes buffer_.
	 */
  virtual ~ArrayTemplate()
  {
    logdbg  << "ArrayTemplate: destructor";
    assert (buffer_);
    delete buffer_;
    buffer_ = 0;
  };

  /// @brief Returns reference to the ith element.
  T& operator[] (const unsigned int i)
  {
  	assert (i < size_);
    return (buffer_[i]);
  };

  void *getArrayBasePointer ()
  {
  	return (void*) buffer_;
  };

  unsigned int getSize ()
  {
  	return sizeof(T);
  }

  void clear ()
  {
    if (type_ != P_TYPE_STRING)
    {
      memset (getArrayBasePointer(), 0, size_*sizeof (T));
    }
  }

  void setArrayNan()
  {
    if (type_ != P_TYPE_STRING)
    {
      void *first = getArrayBasePointer();
      Utils::Data::setNan(type_, first);
      size_t size = sizeof (T);
      for (unsigned int cnt=1; cnt < size_; cnt++)
        memcpy ( (unsigned char*) first + cnt*size, first, size);
      //memset (getArrayBasePointer(), 0, size_*sizeof (T));
    }
  }

private:
  /// Number of elements in array
  unsigned int size_;
  /// Array base pointer
  T *buffer_;
  /// Element data type
  PROPERTY_DATA_TYPE type_;

  void init ()
  {
    logdbg  << "ArrayTemplate: init";
    assert (!buffer_);

    buffer_ = new T [size_];

    clear();

    logdbg  << "ArrayTemplate: init: allocated "<< size_*sizeof(T)  << " bytes";
  };

};

#endif /* MEMORYPAGETEMPLATE_H_ */
