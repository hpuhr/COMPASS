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
 * BufferSet.h
 *
 *  Created on: Mar 21, 2012
 *      Author: sk
 */

#ifndef BufferSet_H_
#define BufferSet_H_

#include <list>
#include <boost/thread.hpp>

class Buffer;
class BufferSetObserver;

/**
 * @brief Aggregation of Buffer instances
 *
 * Thread safe, simple container class which holds Buffers in a list.
 * A Buffer has to be created and deleted outside of the class, although a delete all function exists for that purpose.
 *
 */
class BufferSet
{
public:
  /// @brief Constructor
  BufferSet();
  /// @brief Destructor
  virtual ~BufferSet();

  /// @brief Pushes a buffer onto the list
  void addBuffer (Buffer *buffer);
  /// @brief Returns removed buffer from the list
  Buffer *popBuffer ();

  /// @brief Pushes a list of buffers onto the list
  void addBuffers (std::list<Buffer *> buffers);
  /// @brief Returns all removed buffers from the list
  std::list<Buffer *>popBuffers ();
  /// @brief Returns copied buffer list
  std::list<Buffer *>getBuffers ();

  /// @brief Returns if buffers are in the list
  bool hasData ();
  /// @brief Removes and deletes all buffers in the list
  void clearAndDelete ();
  /// @brief Removes all buffers from the list
  void clear ();

  /// @brief Returns list size
  unsigned int getSize ();

  /// @brief Adds an observer
  void addObserver (BufferSetObserver *observer);
  /// @brief Removes an observer
  void removeObserver (BufferSetObserver *observer);

private:
  /// Container with all buffers
  std::list <Buffer *> data_;
  /// Mutex for list access thread-safety
  boost::mutex mutex_;
  boost::mutex observer_mutex_;

  /// Container with registered observers
  std::vector <BufferSetObserver *> observers_;

  /// Notifies all observers about a buffer addition
  void notifyObserversAdd (Buffer *buffer);
  /// Notifies all observers about a clear event
  void notifyObserversClear ();
};

#endif /* BufferSet_H_ */
