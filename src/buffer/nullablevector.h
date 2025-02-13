#pragma once

#include "buffer.h"
#include "property.h"
#include "stringconv.h"
#include "json.hpp"

#include <QDateTime>

#include <vector>
#include <memory>
// #include <stdexcept>
// #include <bitset>
// #include <type_traits>
// #include <cstddef>

//#include <bitset>

#define PAGE_SIZE_EXPR 32*1024

const bool BUFFER_PEDANTIC_CHECKING = false;

template <typename T>
class NullableVector
{
    friend class Buffer;

    const unsigned int page_size = PAGE_SIZE_EXPR;

    //typedef std::array<T, PAGE_SIZE_EXPR> DATA_CONTAINER;

    // If T is bool, use std::bitset<N>; otherwise, use std::array<T, N>.
    // using DATA_CONTAINER = std::conditional_t<
    //     std::is_same<T, bool>::value,
    //     std::vector<bool>,
    //     std::array<T, PAGE_SIZE_EXPR>>;

    //typedef std::vector<T> DATA_CONTAINER;
    typedef std::array<T, PAGE_SIZE_EXPR> DATA_CONTAINER;;

    typedef std::array<uint8_t, PAGE_SIZE_EXPR/8 + 1> NULL_CONTAINER;

    std::unique_ptr<DATA_CONTAINER>& getValuePage(unsigned int page_index)
    {
        if (BUFFER_PEDANTIC_CHECKING)
        {
            assert (page_index < value_pages_.size());
            assert (value_pages_.at(page_index));
        }

        return value_pages_.at(page_index);
    }

    const std::unique_ptr<DATA_CONTAINER>& getValuePage(unsigned int page_index) const
    {
        if (BUFFER_PEDANTIC_CHECKING)
        {
            assert (page_index < value_pages_.size());
            assert (value_pages_.at(page_index));
        }

        return value_pages_.at(page_index);
    }

    std::unique_ptr<NULL_CONTAINER>& getNullPage(unsigned int page_index)
    {
        if (BUFFER_PEDANTIC_CHECKING)
        {
            assert (page_index < null_flag_pages_.size());
            assert (null_flag_pages_.at(page_index));
        }

        return null_flag_pages_.at(page_index);
    }

    const std::unique_ptr<NULL_CONTAINER>& getNullPage(unsigned int page_index) const
    {
        if (BUFFER_PEDANTIC_CHECKING)
        {
            assert (page_index < null_flag_pages_.size());
            assert (null_flag_pages_.at(page_index));
        }

        return null_flag_pages_.at(page_index);
    }

    uint8_t getNullByte(unsigned int page_index, unsigned int offset) const
    {
        if (BUFFER_PEDANTIC_CHECKING)
        {
            assert (offset / 8 < PAGE_SIZE_EXPR);
        }

        return getNullPage(page_index)->at(offset/8);
    }

    uint8_t& getNullByteRef(unsigned int page_index, unsigned int offset) const
    {
        if (BUFFER_PEDANTIC_CHECKING)
        {
            assert (offset / 8 < PAGE_SIZE_EXPR);
        }

        return getNullPage(page_index)->at(offset/8);
    }

public:
    NullableVector(const NullableVector&) = delete; // Disable copy constructor
    NullableVector& operator=(const NullableVector&) = delete; // Disable copy assignment operator

    virtual ~NullableVector() {}

    void renameProperty(const std::string& name) { property_.rename(name); }

    // The clear() function resets all allocated pages.
    // It sets every value to the default (T{}) and marks all entries as null.
    void clear();

    void clearData();

    T get(unsigned int index) const;

    T& getRef(unsigned int index);

    const std::string getAsString(unsigned int index) const;

    void set(unsigned int index, const T& value, bool adjust_buffer_size=true);

    void setFromFormat(unsigned int index, const std::string& format, const std::string& value_str, bool debug=false);

    void setAll(T value);

    void append(unsigned int index, T value);

    void appendFromFormat(unsigned int index, const std::string& format,
                          const std::string& value_str);

    void setNull(unsigned int index); // does not adjust buffer size
    void setAllNull();

    NullableVector<T>& operator*=(double factor);
    std::set<T> distinctValues(unsigned int index=0);

    std::map<T, unsigned int> distinctValuesWithCounts(unsigned int index = 0);

    std::tuple<bool,T,T> minMaxValues(unsigned int index = 0); // set, min, max

    std::tuple<bool,T,T> minMaxValuesSorted(unsigned int index = 0); // set, min, max

    std::map<T, std::vector<unsigned int>> distinctValuesWithIndexes(unsigned int from_index, unsigned int to_index);
    std::map<T, std::vector<unsigned int>> distinctValuesWithIndexes(
        const std::vector<unsigned int>& indexes);

    std::map<T, unsigned int> uniqueValuesWithIndexes();

    std::vector<unsigned int> nullValueIndexes(unsigned int from_index, unsigned int to_index);

    std::vector<unsigned int> nullValueIndexes(const std::vector<unsigned int>& indexes);

    // void convertToStandardFormat(const std::string& from_format)
    // {
    //     logdbg << "NullableVector " << property_.name() << ": convertToStandardFormat";

    //     static_assert(std::is_integral<T>::value, "only defined for integer types");

    //     if (from_format != "octal")
    //     {
    //         logerr << "NullableVector: convertToStandardFormat: unknown format '" << from_format
    //                << "'";
    //         assert(false);
    //     }

    //     unsigned int data_size = size();

    //     for (unsigned int cnt=0; cnt < data_size; ++cnt)
    //     {
    //         if (!isNull(cnt))
    //             getRef(cnt) = std::stoi(std::to_string(get(cnt)), 0, 8);
    //     }
    // }

    unsigned int size() const // only content size, buffer holds full size
    {
        // Iterate backward over the pages.
        for (unsigned int page = value_pages_.size(); page > 0; )
        {
            // Get the last allocated page index.
            unsigned int page_index = --page;

            // If this page is not allocated, skip it.
            if (!getValuePage(page_index))
                continue;

            // Iterate backward within the page.
            for (unsigned int offset = page_size; offset > 0; )
            {
                unsigned int element_index = --offset;
                unsigned int global_index = page_index * page_size + element_index;

                // Use the existing isNull() to check whether the element is set.
                if (!isNull(global_index))
                {
                    // Found the last non-null element; return its index plus one.
                    if (BUFFER_PEDANTIC_CHECKING)
                    {
                        // loginf << "NullableVector " << property_.name() << ": size: non-null " << global_index + 1;
                    }
                    return global_index + 1;
                }
            }
        }
        // If no element is found (or no page is allocated), return 0.

        if (BUFFER_PEDANTIC_CHECKING)
        {
            //loginf << "NullableVector " << property_.name() << ": size: 0 since nothing";
        }

        return 0;
    }

    bool isNull(unsigned int index) const
    {
        unsigned int page_index = index / page_size;
        unsigned int offset = index % page_size;

        // if (BUFFER_PEDANTIC_CHECKING)
        //     loginf << "NullableVector " << property_.name() << ": isNull: index " << index
        //            << " page_index " << page_index << " offset " << offset
        //            << " null_flag_pages_.size() " << null_flag_pages_.size();

        if (page_index >= null_flag_pages_.size() || !getNullPage(page_index))
        {
            // if (BUFFER_PEDANTIC_CHECKING)
            //     loginf << "NullableVector " << property_.name() << ": isNull: page doesn't exist";

            return true; // if page doesn't exist, treat as NULL
        }

        return (getNullByte(page_index, offset) & (1 << (offset % 8))) != 0;
    }

    bool isAlwaysNull() const
    {
        unsigned int n = size();
        if (n == 0)
        {
            // No data: you might consider this vacuously true,
            // but here we choose to return false.
            return false;
        }
        for (unsigned int index = 0; index < n; ++index) {
            if (!isNull(index))
            {
                return false; // Found a non-null element.
            }
        }
        return true; // All elements are null.
    }

    bool isNeverNull() const
    {
        unsigned int n = size();
        if (n == 0) {
            // No data: again, you might say it is vacuously never null,
            // but here we choose to return false.
            return false;
        }
        for (unsigned int index = 0; index < n; ++index)
        {
            if (isNull(index))
            {
                return false; // Found an element that is null.
            }
        }
        return true; // Every element is non-null.
    }

    void swapData (unsigned int index1, unsigned int index2)
    {
        bool index1_null = isNull(index1);
        bool index2_null = isNull(index2);

        unsigned int data_size = size();

        if (index1_null && index2_null)
            return;
        else if (!index1_null && !index2_null)
        {
            assert (index1 < data_size);
            assert (index2 < data_size);

            T val = get(index1);
            set(index1, get(index2));
            set(index2, val);
        }
        else if (index1_null && !index2_null)
        {
            assert (index2 < data_size);

            set(index1, get(index2));
            setNull(index2);
        }
        else if (!index1_null && index2_null)
        {
            assert (index1 < data_size);

            set(index2, get(index1));
            setNull(index1);
        }
    }

    std::string propertyName() const
    {
        return property_.name();
    }

    std::string propertyID() const
    {
        return property_.name() + "(" + property_.dataTypeString() + ")";
    }

    std::vector<unsigned int> sortPermutation()
    {
        if (size() < buffer_.size())
            resizeDataTo(buffer_.size());

        assert (size() == buffer_.size());
        std::vector<unsigned int> p (size());

        std::iota(p.begin(), p.end(), 0);
        std::sort(p.begin(), p.end(),
                  [&](unsigned int i, unsigned int j){

                      bool is_i_null = isNull(i);
                      bool is_j_null = isNull(j);

                      if (is_i_null && is_j_null)
                          return false; // same not smaller
                      else if (!is_i_null && is_j_null)
                          return false; // not null < null = false
                      else if (is_i_null && !is_j_null)
                          return true; // null < not null = true
                      else
                          return get(i) < get(j);
                  });
        return p;
    }

    void sortByPermutation(const std::vector<unsigned int>& perm)
    {
        std::vector<bool> done(perm.size());

        for (unsigned int i = 0; i < perm.size(); ++i)
        {
            if (done.at(i))
                continue;

            assert (i < done.size());
            done.at(i) = true;
            unsigned int prev_j = i;

            assert (i < perm.size());
            unsigned int j = perm.at(i);
            while (i != j)
            {
                swapData(prev_j, j);

                assert (j < done.size());
                done.at(j) = true;
                prev_j = j;
                assert (j < perm.size());
                j = perm.at(j);
            }
        }
    }

    nlohmann::json asJSON(unsigned int max_size=0);

private:
    Property property_;
    Buffer& buffer_;

    std::vector<std::unique_ptr<DATA_CONTAINER>> value_pages_;       // Stores values
    std::vector<std::unique_ptr<NULL_CONTAINER>> null_flag_pages_; // Stores NULL flags (bit-packed)

    NullableVector(Property& property, Buffer& buffer);

    void ensurePageExists(unsigned int index)
    {
        unsigned int page_index = index / page_size;

        if (BUFFER_PEDANTIC_CHECKING)
        {
            // loginf << "NullableVector " << property_.name() << ": ensurePageExists: start index " << index
            //        << " page_index " << page_index << " value_pages " << value_pages_.size()
            //        << " size " << size();
        }

        if (page_index >= value_pages_.size())
        {
            value_pages_.resize(page_index + 1);
            null_flag_pages_.resize(page_index + 1);
        }

        assert (page_index < value_pages_.size());

        if (!value_pages_.at(page_index))
        {
            // getValuePage(page_index) = std::make_unique<T[]>(page_size);
            // getNullPage(page_index) = std::make_unique<uint8_t[]>(page_size / 8 + 1); // Bit-packed storage

            value_pages_[page_index] = std::make_unique<DATA_CONTAINER>();
            null_flag_pages_[page_index] = std::make_unique<NULL_CONTAINER>(); // Bit-packed storage

            //std::fill_n(*getNullPage(page_index), page_size / 8 + 1, 0); // Initialize NULL flags to 0
            std::fill(getNullPage(page_index)->begin(), getNullPage(page_index)->end(), 0xFF);
        }

        if (BUFFER_PEDANTIC_CHECKING)
        {
            // loginf << "NullableVector " << property_.name() << ": ensurePageExists: end value_pages "
            //        << value_pages_.size() << " size " << size();
        }
    }

    // void unsetNull(unsigned int index)
    // {
    //     ensurePageExists(index);
    //     unsigned int page_index = index / page_size;
    //     unsigned int offset = index % page_size;

    //     getNullByteRef(page_index, offset) &= ~(1 << (offset % 8));
    // }

    // Resizes (or reinitializes) the data pages so that the total number
    // of elements is at least 'new_size'. Every element in a newly allocated page
    // is set to T{}.
    void resizeDataTo(unsigned int new_size)
    {
        // Compute how many pages are required to hold 'size' elements.
        unsigned int required_pages = (new_size == 0 ? 0 : ((new_size - 1) / page_size) + 1);

        // Allocate a new vector for the pages.
        std::vector<std::unique_ptr<DATA_CONTAINER>> new_value_pages;
        new_value_pages.reserve(required_pages);

        // For each page, allocate and fill with default-constructed T.
        for (unsigned int i = 0; i < required_pages; ++i)
        {
            auto page = std::make_unique<DATA_CONTAINER>();
            //std::fill_n(page.get(), page_size, T{});
            std::fill(page->begin(), page->end(), T{});
            new_value_pages.push_back(std::move(page));
        }

        // Replace the old pages with the new ones.
        value_pages_ = std::move(new_value_pages);

        // new data size set by caller
        // if (buffer_.size_ < size())  // set new data size
        //     buffer_.size_ = size();
    }

    // Resizes (or reinitializes) the null flag pages so that the total number
    // of elements is at least 'size'. Every flag in a newly allocated page
    // is set to 0xFF (i.e. every element is marked as null).
    void resizeNullTo(unsigned int size)
    {
        unsigned int required_pages = (size == 0 ? 0 : ((size - 1) / page_size) + 1);

        std::vector<std::unique_ptr<NULL_CONTAINER>> new_null_flag_pages;
        new_null_flag_pages.reserve(required_pages);

        // Allocate each page; note that each page needs room for (page_size/8 + 1) bytes.
        for (unsigned int i = 0; i < required_pages; ++i)
        {
            auto page = std::make_unique<NULL_CONTAINER>();
            //std::fill_n(page.get(), page_size / 8 + 1, 0xFF);
            std::fill(page->begin(), page->end(), 0xFF);
            new_null_flag_pages.push_back(std::move(page));
        }

        null_flag_pages_ = std::move(new_null_flag_pages);
    }

    // ------------------------------------------------------------------
    // The addData() function appends all elements from 'other' to the end
    // of the current vector. It uses resizeDataTo() and resizeNullTo() to
    // ensure enough capacity, then copies each element (and its null flag).
    //
    // Note: This implementation assumes that the logical size of the vector
    // is defined as size() == (number of pages) * page_size.
    // ------------------------------------------------------------------
    void addData(NullableVector<T>& other)
    {
        // Calculate the number of elements currently held and the number in 'other'
        unsigned int current_size = buffer_.size();
        unsigned int other_size = other.size();
        unsigned int new_total_size = current_size + other_size;

        if (BUFFER_PEDANTIC_CHECKING)
        {
            loginf << "NullableVector " << property_.name() << ": addData: start current_size " << current_size
                   << "  buffer_.data_size_ " <<  buffer_.size_
                   << " other_size " << other_size << " new_total_size " << new_total_size;
        }

        // Extend the current container to accommodate new elements.
        // (These functions add new pages without touching existing ones.)
        // resizeDataTo(new_total_size);
        // resizeNullTo(new_total_size);

        // Copy each element from 'other' into our container.
        for (unsigned int i = 0; i < other_size; ++i)
        {
            // Compute target index where the element should be placed.
            unsigned int target_index = current_size + i;
            // unsigned int target_page  = target_index / page_size;
            // unsigned int target_offset = target_index % page_size;

            // Compute source page and offset in 'other'.
            unsigned int src_page   = i / page_size;
            unsigned int src_offset = i % page_size;

            // If the source element is NULL, mark our corresponding element as NULL.
            if (other.isNull(i))
            {
                if (!isNull(target_index))
                    setNull(target_index);

                //(*null_flag_pages_.at(target_index))[target_offset / 8] |= (1 << (target_offset % 8));
                //getNullByteRef(target_index, target_offset) |= (1 << (target_offset % 8));
            }
            else
            {
                // Copy the data value from 'other'.
                //(*value_pages_[target_page])[target_offset] = (*other.value_pages_[src_page])[src_offset];

                if (BUFFER_PEDANTIC_CHECKING)
                {
                    // assert (target_page < value_pages_.size());
                    // assert (value_pages_.at(target_page));
                    // assert (target_offset < getValuePage(target_page)->size());

                    assert (src_page < other.value_pages_.size());
                    assert (other.value_pages_.at(src_page));
                    assert (src_offset < other.getValuePage(src_page)->size());
                }

                //getValuePage(target_page)->at(target_offset) =  other.getValuePage(src_page)->at(src_offset);
                // Mark the new element as non-null.
                //(*null_flag_pages_.at(target_index))[target_offset / 8] &= ~(1 << (target_offset % 8));

                //getNullByteRef(target_index, target_offset) &= ~(1 << (target_offset % 8));
                set(target_index, other.getValuePage(src_page)->at(src_offset), false); // size ajdusted after
            }
        }

        // new size set in buffer
    }
    //void copyData(NullableVector<T>& other);
    // ------------------------------------------------------------------
    // cutToSize(unsigned int new_size)
    //
    // This function truncates the container so that only the first new_size
    // elements remain. Any elements beyond new_size are discarded. In practice,
    // we do two things:
    //  1. Reduce the number of allocated pages to only those required to store
    //     new_size elements.
    //  2. In the last (partially used) page, mark any entries after new_size as NULL.
    // ------------------------------------------------------------------
    void cutToSize(unsigned int new_size)
    {
        // Compute the number of pages required to hold new_size elements.
        unsigned int required_pages = (new_size == 0 ? 0 : ((new_size - 1) / page_size) + 1);

        // If new_size is 0, clear all pages.
        if (new_size == 0)
        {
            value_pages_.clear();
            null_flag_pages_.clear();

            return;
        }

        // If we have more pages allocated than needed, shrink the vectors.
        if (value_pages_.size() > required_pages)
        {
            value_pages_.resize(required_pages);
            null_flag_pages_.resize(required_pages);
        }

        // In the last page, if new_size does not end exactly on a page boundary,
        // mark all entries after new_size as "removed" (by setting them to NULL).
        unsigned int remainder = new_size % page_size;

        // If new_size is a multiple of page_size, then remainder == 0 means the
        // last page is entirely valid.
        if (remainder == 0)
            remainder = page_size;

        unsigned int last_page = required_pages - 1;

        for (unsigned int i = remainder; i < page_size; ++i)
        {
            // Optionally reset the value to the default.
            (*value_pages_[last_page])[i] = T{};
            // Mark the element as NULL in the bit-packed null flags.
            //unsigned int byte_index = i / 8;
            //unsigned int bit_index  = i % 8;

            //(*null_flag_pages_.at(last_page))[byte_index] |= (1 << bit_index);

            getNullByteRef(last_page, i) |= (1 << i % 8);
        }
    }

    // ------------------------------------------------------------------
    // cutUpToIndex(unsigned int index)
    //
    // This function removes all elements _before_ the given index, so that
    // the element originally at global index 'index' becomes the new first element.
    // In doing so the new logical size becomes (old size - index). Since our data is
    // stored in pages, we allocate new pages for the resulting data and copy over
    // the remaining elements (including their null flags).
    // ------------------------------------------------------------------
    void cutUpToIndex(unsigned int index)
    {
        unsigned int old_size = size();
        // If index is beyond or equal to the current size, remove everything.
        if (index >= old_size)
        {
            value_pages_.clear();
            null_flag_pages_.clear();
            return;
        }

        unsigned int new_size = old_size - index;
        unsigned int new_pages = (new_size == 0 ? 0 : ((new_size - 1) / page_size) + 1);

        // Allocate new vectors for the pages.
        std::vector<std::unique_ptr<DATA_CONTAINER>> new_value_pages;
        new_value_pages.reserve(new_pages);
        std::vector<std::unique_ptr<NULL_CONTAINER>> new_null_flag_pages;
        new_null_flag_pages.reserve(new_pages);

        // Allocate each new page and (optionally) initialize with default values.
        for (unsigned int i = 0; i < new_pages; ++i)
        {
            auto new_data_page = std::make_unique<DATA_CONTAINER>();
            std::fill(new_data_page->begin(), new_data_page->end(), T{});
            //std::fill_n(new_data_page.get(), page_size, T{});
            new_value_pages.push_back(std::move(new_data_page));

            auto new_null_page = std::make_unique<NULL_CONTAINER>();
            // Initialize all bits to 0 (i.e. "not null") before copying in the proper bits.
            //std::fill_n(new_null_page.get(), page_size / 8 + 1, 0);
            std::fill(new_null_page->begin(), new_null_page->end(), 0);
            new_null_flag_pages.push_back(std::move(new_null_page));
        }

        // Copy each remaining element from the old storage to the new storage.
        // new_index will run from 0 to new_size - 1.
        for (unsigned int new_index = 0; new_index < new_size; ++new_index)
        {
            unsigned int old_index = new_index + index;
            unsigned int old_page   = old_index / page_size;
            unsigned int old_offset = old_index % page_size;
            unsigned int new_page   = new_index / page_size;
            unsigned int new_offset = new_index % page_size;

            // Copy the value.
            (*new_value_pages[new_page])[new_offset] = (*value_pages_[old_page])[old_offset];

            // Copy the corresponding null flag.
            unsigned int old_byte = old_offset / 8;
            unsigned int old_bit  = old_offset % 8;
            bool was_null = ((*null_flag_pages_[old_page])[old_byte] & (1 << old_bit)) != 0;
            unsigned int new_byte = new_offset / 8;
            unsigned int new_bit  = new_offset % 8;
            if (was_null)
            {
                (*new_null_flag_pages[new_page])[new_byte] |= (1 << new_bit);
            }
            else
            {
                (*new_null_flag_pages[new_page])[new_byte] &= ~(1 << new_bit);
            }
        }

        // Replace the old storage with the new storage.
        value_pages_ = std::move(new_value_pages);
        null_flag_pages_ = std::move(new_null_flag_pages);
    }

    // Removes all elements whose indexes appear in indexes_to_remove.
    // The indexes in indexes_to_remove are assumed to be sorted in ascending order.
    void removeIndexes(const std::vector<unsigned int>& indexes_to_remove) {
        // If there's nothing to remove, do nothing.
        if (indexes_to_remove.empty())
            return;

        unsigned int old_size = size();
        unsigned int num_to_remove = indexes_to_remove.size();

        // If all (or more than) elements are to be removed, clear the container.
        if (num_to_remove >= old_size) {
            value_pages_.clear();
            null_flag_pages_.clear();
            return;
        }

        // The new size is the old size minus the number of removed elements.
        unsigned int new_size = old_size - num_to_remove;
        unsigned int new_pages = (new_size == 0 ? 0 : ((new_size - 1) / page_size) + 1);

        // Allocate new vectors for the data and null pages.
        std::vector<std::unique_ptr<DATA_CONTAINER>> new_value_pages;
        new_value_pages.reserve(new_pages);
        std::vector<std::unique_ptr<NULL_CONTAINER>> new_null_flag_pages;
        new_null_flag_pages.reserve(new_pages);

        // Allocate each new page.
        for (unsigned int i = 0; i < new_pages; ++i) {
            auto data_page = std::make_unique<DATA_CONTAINER>();
            // Initialize with default-constructed T.
            //std::fill_n(data_page.get(), page_size, T{});
            std::fill(data_page->begin(), data_page->end(), T{});
            new_value_pages.push_back(std::move(data_page));

            auto null_page = std::make_unique<NULL_CONTAINER>();
            // Initialize the null flag page to 0 (i.e. "not null").
            //std::fill_n(null_page.get(), page_size / 8 + 1, 0);
            std::fill(null_page->begin(), null_page->end(), 0);
            new_null_flag_pages.push_back(std::move(null_page));
        }

        // Now iterate through all elements of the old container and copy
        // over those not marked for removal.
        unsigned int new_index = 0;      // Position in the new container.
        unsigned int remove_pos = 0;     // Position in the indexes_to_remove vector.

        for (unsigned int old_index = 0; old_index < old_size; ++old_index) {
            // If the current index is in indexes_to_remove, skip it.
            if (remove_pos < indexes_to_remove.size() &&
                indexes_to_remove[remove_pos] == old_index)
            {
                ++remove_pos;
                continue;
            }

            // Determine source positions.
            unsigned int old_page = old_index / page_size;
            unsigned int old_offset = old_index % page_size;
            // Determine destination positions.
            unsigned int new_page = new_index / page_size;
            unsigned int new_offset = new_index % page_size;

            // Copy the data element.
            (*new_value_pages[new_page])[new_offset] = (*value_pages_[old_page])[old_offset];

            // Copy the corresponding null flag.
            unsigned int old_byte = old_offset / 8;
            unsigned int old_bit  = old_offset % 8;
            bool was_null = ((*null_flag_pages_[old_page])[old_byte] & (1 << old_bit)) != 0;
            unsigned int new_byte = new_offset / 8;
            unsigned int new_bit  = new_offset % 8;
            if (was_null)
                (*new_null_flag_pages[new_page])[new_byte] |= (1 << new_bit);
            else
                (*new_null_flag_pages[new_page])[new_byte] &= ~(1 << new_bit);

            ++new_index;
        }

        // Replace the old storage with the new storage.
        value_pages_ = std::move(new_value_pages);
        null_flag_pages_ = std::move(new_null_flag_pages);
    }

};

template <class T>
NullableVector<T>::NullableVector(Property& property, Buffer& buffer)
    : property_(property), buffer_(buffer)
{
}

template <class T>
void NullableVector<T>::clear()
{
    // Iterate over all pages that have been allocated.
    for (unsigned int page_index = 0; page_index < value_pages_.size(); ++page_index)
    {
        // Only operate on pages that have been allocated.
        if (getValuePage(page_index))
        {
            // Reset all values to default-constructed T.
            //std::fill_n(getValuePage(page_index).get(), page_size, T{});
            std::fill(getValuePage(page_index)->begin(), getValuePage(page_index)->end(), T{});

            // Mark every element as null by setting all bits to 1.
            // The storage size is (page_size/8 + 1) bytes.
            //std::fill_n(getNullPage(page_index).get(), page_size / 8 + 1, 0xFF);
            std::fill(getNullPage(page_index)->begin(), getNullPage(page_index)->end(), 0xFF);
        }
    }
}

template <class T>
void NullableVector<T>::clearData()
{
    value_pages_.clear();
    null_flag_pages_.clear();
}

template <class T>
T NullableVector<T>::get(unsigned int index) const
{
    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(size() <= buffer_.size_);
        assert(index < size());
    }

    if (isNull(index))
    {
        logerr << "NullableVector " << property_.name() << ": get: index " << index << " is null";
        assert (false);
    }

    unsigned int page_index = index / page_size;
    unsigned int offset = index % page_size;
    return (*getValuePage(page_index))[offset];
}

template <class T>
T& NullableVector<T>::getRef(unsigned int index)
{
    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(size() <= buffer_.size_);
        assert(index < size());
    }

    if (isNull(index))
    {
        logerr << "NullableVector " << property_.name() << ": getRef: index " << index << " is null";
        assert (false);
    }

    unsigned int page_index = index / page_size;
    unsigned int offset = index % page_size;

    return getValuePage(page_index)->at(offset);
}

template <class T>
const std::string NullableVector<T>::getAsString(unsigned int index) const
{
    logdbg << "NullableVector " << property_.name() << ": getAsString";
    return Utils::String::getValueString(get(index));
}

template <class T>
void NullableVector<T>::set(unsigned int index, const T& value, bool adjust_buffer_size)
{
    if (BUFFER_PEDANTIC_CHECKING)
    {
        //assert(size() <= buffer_.size_);

        //loginf << "NullableVector " << property_.name() << ": set: start index " << index << " size " << size();
    }

    ensurePageExists(index);

    unsigned int page_index = index / page_size;
    unsigned int offset = index % page_size;
    (*getValuePage(page_index))[offset] = value;

    // clear NULL bit (indicating valid value)
    (*getNullPage(page_index))[offset / 8] &= ~(1 << (offset % 8));

    if (adjust_buffer_size && buffer_.size_ < index + 1)  // set new data size
        buffer_.size_ = index + 1;

    if (BUFFER_PEDANTIC_CHECKING)
    {
        //loginf << "NullableVector " << property_.name() << ": set: end size " << size();
        assert (!isNull(index));
        assert (index <= size());
    }
}

template <class T>
void NullableVector<T>::setFromFormat(
    unsigned int index, const std::string& format, const std::string& value_str, bool debug)
{
    logdbg << "NullableVector " << property_.name() << ": setFromFormat";
    T value;

    if (format == "octal")
    {
        value = std::stoi(value_str, 0, 8);
    }
    else if (format == "hexadecimal")
    {
        value = std::stoi(value_str, 0, 16);
    }
    else if (format == "epoch_tod_ms")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else if (format == "epoch_tod_s")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(1000 * std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else if (format == "bool")
    {
        if (value_str == "0" || value_str == "false")
            value = 'N';
        else if (value_str == "1"  || value_str == "true")
            value = 'Y';
    }
    else if (format == "bool_invert")
    {
        if (value_str == "1"  || value_str == "true")
            value = 'N';
        else if (value_str == "0" || value_str == "false")
            value = 'Y';
    }
    else
    {
        logerr << "NullableVector: setFromFormat: unknown format '" << format << "'";
        assert(false);
    }

    if (debug)
        loginf << "NullableVector: setFromFormat: index " << index << " value_str '" << value_str
               << "' value '" << value << "'";

    set(index, value);
}

template <class T>
void NullableVector<T>::setAll(T value)
{
    unsigned int data_size = size();

    for (unsigned int index=0; index < data_size; ++index)
        set(index, value);
}

template <class T>
void NullableVector<T>::append(unsigned int index, T value)
{
    if (index >= size() || isNull(index))
        set(index, value);
    else
    {
        getRef(index) += value;
    }
}

template <class T>
void NullableVector<T>::appendFromFormat(unsigned int index, const std::string& format,
                      const std::string& value_str)
{
    logdbg << "NullableVector " << property_.name() << ": appendFromFormat";
    T value;

    if (format == "octal")
    {
        value = std::stoi(value_str, 0, 8);
    }
    else if (format == "hexadecimal")
    {
        value = std::stoi(value_str, 0, 16);
    }
    else if (format == "epoch_tod_ms")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else if (format == "epoch_tod_s")
    {
        QDateTime date_time;
        date_time.setMSecsSinceEpoch(1000 * std::stoul(value_str));
        value = Utils::String::timeFromString(date_time.toString("hh:mm:ss.zzz").toStdString());
    }
    else
    {
        logerr << "NullableVector: appendFromFormat: unknown format '" << format << "'";
        assert(false);
    }

    append(index, value);
}

template <class T>
void NullableVector<T>::setNull(unsigned int index)
{
    ensurePageExists(index);
    unsigned int page_index = index / page_size;
    unsigned int offset = index % page_size;

    // set NULL bit
    (*getNullPage(page_index))[offset / 8] |= (1 << (offset % 8));
}

template <class T>
void NullableVector<T>::setAllNull()
{
    clear();
}

template <class T>
NullableVector<T>& NullableVector<T>::operator*=(double factor)
{
    logdbg << "NullableVector " << property_.name() << ": operator*=";

    unsigned int data_size = size();

    for (unsigned int index=0; index < data_size; ++index)
    {
        if (!isNull(index))
            getRef(index) *= factor;
    }

    return *this;
}

template <class T>
std::set<T> NullableVector<T>::distinctValues(unsigned int index)
{
    logdbg << "NullableVector " << property_.name() << ": distinctValues";

    std::set<T> values;

    T value;

    unsigned int data_size = size();

    for (; index < data_size; ++index)
    {
        if (!isNull(index))  // not for null
        {
            value = get(index);

            if (!values.count(value))
                values.insert(value);
        }
    }

    return values;
}

template <class T>
std::map<T, unsigned int> NullableVector<T>::distinctValuesWithCounts(unsigned int index)
{
    logdbg << "NullableVector " << property_.name() << ": distinctValuesWithCounts";

    std::map<T, unsigned int> values;

    T value;

    unsigned int data_size = size();

    for (; index < data_size; ++index)
    {
        if (!isNull(index))  // not for null
        {
            value = get(index);
            values[value] += 1;
        }
    }

    return values;
}

template <class T>
std::tuple<bool,T,T> NullableVector<T>::minMaxValues(unsigned int index) // set, min, max
{
    bool set = false;
    T min{}, max{};

    unsigned int data_size = size();

    for (; index < data_size; ++index)
    {
        if (!isNull(index))  // not for null
        {
            if (!set)
            {
                min = get(index);
                max = get(index);
                set = true;
            }
            else
            {
                min = std::min(min, get(index));
                max = std::max(max, get(index));
            }
        }
    }

    return std::tuple<bool,T,T> {set, min, max};
}

template <class T>
std::tuple<bool,T,T> NullableVector<T>::minMaxValuesSorted(unsigned int index) // set, min, max
{
    bool min_set = false;
    bool max_set = false;
    T min, max;

    if (!size())
        return std::tuple<bool,T,T> {min_set && max_set, min, max};

    unsigned int data_size = size();

    for (unsigned int tmp_index=index; tmp_index < data_size; ++tmp_index)
    {
        if (!isNull(tmp_index))  // not for null
        {
            if (!min_set)
            {
                min = get(tmp_index);
                min_set = true;
                break;
            }
        }
    }

    for (int tmp_index=data_size-1; tmp_index >= 0 && tmp_index >= index; --tmp_index)
    {
        //loginf << "UGA: minMaxValuesSorted: tmp_index " << tmp_index << " index " << index << " size " << data_.size();

        if (!isNull(tmp_index))  // not for null
        {
            if (!max_set)
            {
                max = get(tmp_index);
                max_set = true;
                break;
            }
        }
    }

    return std::tuple<bool,T,T> {min_set && max_set, min, max};
}

template <class T>
std::map<T, std::vector<unsigned int>> NullableVector<T>::distinctValuesWithIndexes(
    unsigned int from_index, unsigned int to_index)
{
    logdbg << "NullableVector " << property_.name() << ": distinctValuesWithIndexes";

    std::map<T, std::vector<unsigned int>> values;

    assert(from_index <= to_index);

    unsigned int data_size = size();

    if (from_index + 1 > data_size)  // no data
        return values;

    for (unsigned int index = from_index; index <= to_index; ++index)
    {
        if (!isNull(index))  // not for null
        {
            values[get(index)].push_back(index);
        }
    }

    logdbg << "NullableVector " << property_.name() << ": distinctValuesWithIndexes: done with "
           << values.size();
    return values;
}

template <class T>
std::map<T, std::vector<unsigned int>> NullableVector<T>::distinctValuesWithIndexes(
    const std::vector<unsigned int>& indexes)
{
    logdbg << "NullableVector " << property_.name() << ": distinctValuesWithIndexes";

    std::map<T, std::vector<unsigned int>> values;

    for (auto index : indexes)
    {
        if (!isNull(index))  // not for null
        {
            values[get(index)].push_back(index);
        }
    }

    logdbg << "NullableVector " << property_.name() << ": distinctValuesWithIndexes: done with "
           << values.size();
    return values;
}

template <class T>
std::map<T, unsigned int> NullableVector<T>::uniqueValuesWithIndexes()
{
    logdbg << "NullableVector " << property_.name() << ": uniqueValuesWithIndexes";

    std::map<T, unsigned int> values;
    unsigned int data_size = size();

    T value;

    for (unsigned int index = 0; index < data_size; ++index)
    {
        if (!isNull(index))  // not for null
        {
            value = get(index);

            assert (!values.count(value));
            values[value] = index;
        }
    }

    logdbg << "NullableVector " << property_.name() << ": uniqueValuesWithIndexes: done with "
           << values.size();
    return values;
}

template <class T>
std::vector<unsigned int> NullableVector<T>::nullValueIndexes(unsigned int from_index, unsigned int to_index)
{
    logdbg << "NullableVector " << property_.name() << ": nullValueIndexes";

    std::vector<unsigned int> indexes;

    assert(from_index <= to_index);

    for (unsigned int index = from_index; index <= to_index; ++index)
    {
        if (isNull(index))  // not for null
        {
            indexes.push_back(index);
        }
    }

    logdbg << "NullableVector " << property_.name() << ": nullValueIndexes: done with "
           << indexes.size();
    return indexes;
}

template <class T>
std::vector<unsigned int> NullableVector<T>::nullValueIndexes(const std::vector<unsigned int>& indexes)
{
    logdbg << "NullableVector " << property_.name() << ": nullValueIndexes";

    std::vector<unsigned int> ret_indexes;

    for (auto index : indexes)
    {
        if (isNull(index))  // not for null
        {
            ret_indexes.push_back(index);
        }
    }

    logdbg << "NullableVector " << property_.name() << ": nullValueIndexes: done with "
           << ret_indexes.size();
    return ret_indexes;
}

template <class T>
nlohmann::json NullableVector<T>::asJSON(unsigned int max_size)
{
    nlohmann::json list = nlohmann::json::array();

    unsigned int size = buffer_.size();

    if (max_size != 0)
        size = std::min(size, max_size);

    for (unsigned int cnt=0; cnt < size; ++cnt)
    {
        if (isNull(cnt))
            list.push_back(nlohmann::json());
        else
            list.push_back(get(cnt));
    }

    return list;
}

template <>
NullableVector<bool>& NullableVector<bool>::operator*=(double factor);

template <>
void NullableVector<bool>::setFromFormat(unsigned int index, const std::string& format,
                                         const std::string& value_str, bool debug);

template <>
void NullableVector<bool>::append(unsigned int index, bool value);

template <>
void NullableVector<std::string>::append(unsigned int index, std::string value);

template <>
nlohmann::json NullableVector<boost::posix_time::ptime>::asJSON(unsigned int max_size);
