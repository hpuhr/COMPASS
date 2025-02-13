#include "nullablevector.h"
#include "timeconv.h"

#include "util/tbbhack.h"

using namespace Utils;

template <>
    NullableVector<bool>& NullableVector<bool>::operator*=(double factor)
{
    bool tmp_factor = static_cast<bool>(factor);

    unsigned int data_size = contentSize();

    for (unsigned int index=0; index < data_size; ++index)
    {
        if (!isNull(index))
            set(index, get(index) & tmp_factor);
    }

    return *this;
}

template <>
void NullableVector<bool>::setFromFormat(unsigned int index, const std::string& format,
                                            const std::string& value_str, bool debug)
{
    logdbg << "OldNullableVector " << property_.name() << ": setFromFormat";
    bool value;

    if (format == "invert")
    {
        if (value_str == "0")
            value = 1;
        else if (value_str == "1")
            value = 0;
        else
        {
            logerr << "OldNullableVector: setFromFormat: unknown bool value '" << value_str << "'";
            assert(false);
        }
    }
    else
    {
        logerr << "OldNullableVector: setFromFormat: unknown format '" << format << "'";
        assert(false);
    }

    if (debug)
        loginf << "OldNullableVector: setFromFormat: index " << index << " value_str '" << value_str
               << "' value '" << value << "'";

    set(index, value);
}

template <>
void NullableVector<bool>::append(unsigned int index, bool value)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": append: index " << index << " value '"
           << value << "'";

    if (index >= contentSize() || isNull(index))
        set(index, value);
    else
        set(index, get(index) | value);
}

template <>
void NullableVector<std::string>::append(unsigned int index, std::string value)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": append: index " << index << " value '"
           << value << "'";

    if (index >= contentSize() || isNull(index))
        set(index, value);
    else
    {
        getRef(index) += ";" + value;
    }

    // logdbg << "ArrayListTemplate: append: size " << size_ << " max_size " << max_size_;
}

template <>
nlohmann::json NullableVector<boost::posix_time::ptime>::asJSON(unsigned int max_size)
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
            list.push_back(Time::toString(get(cnt)));
    }

    return list;
}

// template class NullableVector<bool>;
// template class NullableVector<char>;
// template class NullableVector<unsigned char>;
// template class NullableVector<int>;
// template class NullableVector<unsigned int>;
// template class NullableVector<long>;
// template class NullableVector<unsigned long>;
// template class NullableVector<float>;
// template class NullableVector<double>;
// template class NullableVector<std::string>;
// template class NullableVector<nlohmann::json>;
// template class NullableVector<boost::posix_time::ptime>;
