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
 * DBResultBufferTemplate.h
 *
 *  Created on: Nov 27, 2011
 *      Author: sk
 */

#ifndef DBRESULTBUFFERTEMPLATE_H_
#define DBRESULTBUFFERTEMPLATE_H_

#include "Logger.h"

/**
 * @brief Early version of Buffer/ArrayTemplate structure. Not used anymore.
 */
template <class T>
class BufferTemplate
{
public:
  BufferTemplate(int size = 10000)
  {
    logdbg  << "DBResultBufferTemplate: contructor";
    size_ = size;
    buffer_ = 0;
    used_=0;

    init ();
  };
  virtual ~BufferTemplate()
  {
    logdbg  << "DBResultBufferTemplate: destructor";
    assert (buffer_);
    delete buffer_;
    buffer_ = 0;
  };

  void clear ()
  {
    logdbg  << "DBResultBufferTemplate: clear";
    used_ = 0;
  };
  bool isFull ()
  {
    //logdbg  << "DBResultBufferTemplate: isFull";
    return (used_ == size_-1);
  };

  T *getNewElement ()
  {
    //logdbg  << "DBResultBufferTemplate: getNewElement: used " << used_ << " size " << size_;

    if (used_ >= size_)
      throw std::runtime_error("DBResultBufferTemplate: getNewElement: used up to capacity");

    used_++;

    assert (buffer_);

    return &(buffer_ [used_-1]);
  };

private:
  int size_;
  int used_;
  T *buffer_;

  void init ()
  {
    logdbg  << "DBResultBufferTemplate: init";
    assert (!buffer_);

    buffer_ = new T [size_];

    logdbg  << "DBResultBufferTemplate: init: allocated "<< size_*sizeof(T)  << " bytes";
  };
};

#endif /* DBRESULTBUFFERTEMPLATE_H_ */
