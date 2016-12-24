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

static const unsigned int BUFFER_ARRAY_SIZE=10000;

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

#include <array>

class ArrayBase
{
public:
  /// @brief Constructor
    ArrayBase () {};
    /// @brief Destructor
    virtual ~ArrayBase () {};
    /// @brief Pointer to the first element of the array
    //virtual void *getArrayBasePointer () = 0;
    /// @brief Returns size of one element
    virtual unsigned int getSize ()=0;
    /// @brief Sets all elements to 0
    virtual void clear() =0;
    /// @brief Sets all elements to NaN
    virtual void setArrayNan()=0;
};

template <class T>
class ArrayTemplate2 : public ArrayBase
{
public:
    ArrayTemplate2()
    : array_(0), is_shallow_copy_(false)
  {
  };

  virtual ~ArrayTemplate2()
  {
  };

  ArrayTemplate2(const ArrayTemplate2& obj, bool deep) // deep copy, shallow copy
  {
      // TODO
      *this=obj;
  }

  void clear()
  {
      // TODO template specialization
  }

  void setArrayNan()
  {
      // TODO template specialization
  }

  const bool isShallowCopy () { return is_shallow_copy_; }
  void setIsShallowCopy (bool val) { is_shallow_copy_=val; } // TODO should be done by copy ctor
protected:
  std::array<T,BUFFER_ARRAY_SIZE> *array_;

  bool is_shallow_copy_;
};

//class ArrayTemplateBool : public ArrayTemplate2<bool>
//{
//public:
//    ArrayTemplateBool () {};
//    ~ArrayTemplateBool () {};
//};

//case P_TYPE_CHAR:
//                manager_ = new ArrayTemplateManager<char> (type_, size_);
//                break;
//            case P_TYPE_INT:
//                manager_ = new ArrayTemplateManager<int> (type_, size_);
//                break;
//            case P_TYPE_UCHAR:
//                manager_ = new ArrayTemplateManager<unsigned char> (type_, size_);
//                break;
//            case P_TYPE_UINT:
//                manager_ = new ArrayTemplateManager<unsigned int> (type_, size_);
//                break;
//            case P_TYPE_LONGINT:
//                manager_ = new ArrayTemplateManager<long int> (type_, size_);
//                break;
//            case P_TYPE_ULONGINT:
//                manager_ = new ArrayTemplateManager<unsigned long int> (type_, size_);
//                break;
//            case P_TYPE_STRING:
//                manager_ = new ArrayTemplateManager<std::string> (type_, size_);
//                break;
//            case P_TYPE_FLOAT:
//                manager_ = new ArrayTemplateManager<float> (type_, size_);
//                break;
//            case P_TYPE_DOUBLE:
//                manager_ = new ArrayTemplateManager<double> (type_, size_);
//                break;
//      case P_TYPE_POINTER:
//        manager_ = new ArrayTemplateManager<void*> (type_, size_);
#endif /* MEMORYPAGETEMPLATE_H_ */
