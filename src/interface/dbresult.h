/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBRESULT_H_
#define DBRESULT_H_

#include <memory>

class Buffer;

/**
 * @brief Generalized result of database query
 *
 * Simple encapsulation of a buffer (result data from query) and a flag indicating if a buffer was
 * set.
 */
class DBResult
{
  public:
    /// @brief Constructor with parameters
    DBResult(std::shared_ptr<Buffer> buffer) : contains_data_(true), buffer_(buffer) {}
    /// @brief Default constructor
    DBResult() : contains_data_(false) {}
    /// @brief Destructor
    virtual ~DBResult() {}

    /// @brief Sets the result buffer
    void buffer(std::shared_ptr<Buffer> buffer)
    {
        buffer_ = buffer;
        contains_data_ = true;
    }
    /// @brief Returns the result buffer
    std::shared_ptr<Buffer> buffer() const { return buffer_; }

    /// @brief Returns if contains data flag was set
    bool containsData() { return contains_data_; }

  private:
    /// @brief Contains result data flag
    bool contains_data_;
    /// @brief Result data buffer
    std::shared_ptr<Buffer> buffer_;
};

#endif /* DBRESULT_H_ */
