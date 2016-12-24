/*
 * ArrayContainer.h
 *
 *  Created on: Apr 11, 2015
 *      Author: sk
 */

#ifndef ARRAYCONTAINER_H_
#define ARRAYCONTAINER_H_

#include <cassert>
#include <vector>
#include "ArrayTemplate.h"


class ArrayContainerBase
{
public:
    /// @brief Constructor
    ArrayContainerBase () {};
    /// @brief Destructor
    virtual ~ArrayContainerBase () {};

    virtual const size_t size ()=0;
    virtual const size_t maxSize ()=0;
};

template <class T>
class ArrayContainerTemplate : public ArrayContainerBase
{
public:
    /// @brief Constructor
    ArrayContainerTemplate(Property property)
    : property_ (property), size_(0), max_used_index_(0)
    {

    }
    /// @brief Destructor
    virtual ~ArrayContainerTemplate()
    {
        typename std::vector <ArrayTemplate2<T> *>::iterator it;

        for (it = array_templates_.begin(); it != array_templates_.end(); it++)
            delete *it;  // shallow copies have to be handled inside
        array_templates_.clear();
    }

    /// @brief Returns value to a given data item for an index
    T& at (size_t n)
    {
        if (n > size)
            allocatedUpToIndex (n);

        assert (n <= size);
        //assert (array_templates_.at(n / GLOBAL_ARRAY_SIZE));

        return array_templates_.at(n / BUFFER_ARRAY_SIZE)->at(n % BUFFER_ARRAY_SIZE);
    }
//    const T& at (size_t n)
//    {
//        if (n > size)
//            allocatedUpToIndex (n);
//
//        assert (n <= size);
//        //assert (array_templates_.at(n / GLOBAL_ARRAY_SIZE));
//
//        return array_templates_.at(n / BUFFER_ARRAY_SIZE)->at(n % BUFFER_ARRAY_SIZE);
//
//    }
    /// @brief Returns void pointer to a given data item at a given index

    /// @brief Returns a shallow copy, must be deleted by caller
    //ArrayContainerTemplate *getShallowCopy ();

    /// @brief Sets all values of a given Property to Nan (Not a number)
    void setPropertyValuesNan (unsigned int property_index)
    {
        //TODO
    }

    /// @brief Returns key value for the first position
    //unsigned int getMinKey (unsigned int key_pos);
    /// @brief Returns key value for the maximum used position
    //unsigned int getMaxKey (unsigned int key_pos);
    /// @brief Searches and returns key position for a given key value (and index), returns -1 if not found
    //int getIndexForKey (unsigned int key_pos, unsigned int key);

    void allocatedUpToIndex (size_t n)
    {
        while (size_ < n)
        {
            array_templates_.push_back (new ArrayTemplate2<T> ());
            size_ += BUFFER_ARRAY_SIZE;
        }
    }
protected:
    Property property_;

    /// Number of elements in all ArrayTemplates
    size_t size_;

    /// Maximal used index
    size_t max_used_index_;

    /// Container with all ArrayTemplates
    std::vector <ArrayTemplate2<T> *> array_templates_;


    /// @brief Searches for a key (internal)
    //int binarySearch(unsigned int key_pos, int index_start, int index_last, unsigned int key);

public:

    const size_t size ()
    {
        return max_used_index_+1;
    };

    /// @brief Returns the maximal size
    const size_t maxSize ()
    {
        return size_;
    }

    /// @brief Returns property list
    const Property &property ()
    {
        return property_;
    }
};

#endif /* ARRAYCONTAINER_H_ */
