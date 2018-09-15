#ifndef ARRAYVECTOR_H
#define ARRAYVECTOR_H

#include <vector>
#include "logger.h"
#include "property.h"
#include "stringconv.h"

template <class T>
class ArrayVectorTemplate : public ArrayListBase
{
public:
    ArrayVectorTemplate() : ArrayListBase () {}

    virtual ~ArrayVectorTemplate () {}

    virtual void clear() override
    {
        std::fill (data_.begin(),data_.begin(), {});

        setAllNone();
    }

    /// @brief Returns const reference to a specific value
    const T &get (size_t index)
    {
        assert (!isNone(index));

        if (isNone(index))
            throw std::out_of_range ("ArrayVectorTemplate: get of None value "+std::to_string(index));

        return data_.at(index);
    }

    const std::string getAsString (size_t index) override
    {
        if (isNone(index))
            throw std::out_of_range ("ArrayVectorTemplate: getAsString of None value "+std::to_string(index));

        return Utils::String::getValueString (data_.at(at));
    }

    void set (size_t index, T value)
    {
        data_[index] = value;

        unsetNone(index);
    }

    virtual void setNone(size_t index) override
    {
        ArrayListBase::setNone(index);
    }

    void addData (ArrayVectorTemplate<T> &other)
    {
        logdbg << "ArrayVectorTemplate: addData: data size " << data_.size() << " none flags size " << none_flags_.size();

        data_.insert(data_.end(), other.data_.begin(), other.data_.end());
        none_flags_.insert(none_flags_.end(), other.none_flags_.begin(), other.none_flags_.end());
        assert (data_.size() == none_flags_.size());

        other.data_.clear();
        other.none_flags_.clear();

        logdbg << "ArrayVectorTemplate: addData: end data size " << data_.size() << " none flags size "
               << none_flags_.size();
    }

    ArrayVectorTemplate<T>& operator*=(double factor)
    {
        for (auto &data_it : data_)
                data_it *= factor;

        return *this;
    }

    std::set<T> distinctValues (size_t index=0)
    {
        std::set<T> values;

        T value;

        size_t list_cnt=0;
        size_t size = data_.size();

        for (; index < size; index++)
        {
                if (!none_flags_.at(list_cnt)->at(list_row_cnt)) // not for none
                {
                    value = array_list->at(list_row_cnt);
                    if (values.count(value) == 0)
                        values.insert(value);
                }
            }
        }
        return values;
    }


private:
    /// Data containers
    std::vector <T> data_;
};

#endif // ARRAYVECTOR_H
