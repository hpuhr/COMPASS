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

#pragma once

#include "json.hpp"

#include <string>
#include <set>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>

namespace license
{

/**
*/
struct License
{
    //!don't change order!
    enum class Type
    {
        Free = 0, //typically not part of a valid license -> unlicensed = Free
        Trial,
        Pro
    };

    enum class State
    {
        Valid = 0,
        Empty,
        Invalid,
        Expired
    };

    static std::string typeToString(Type t);
    static boost::optional<Type> typeFromString(const std::string& s);

    enum Component
    {
        ComponentProbIMMReconstructor = 101
    };

    static std::string componentToString(Component c);
    static boost::optional<Component> componentFromString(const std::string& s);
    
    static boost::optional<nlohmann::json> unpackageLicenseKey(const std::string& license_key);
    static std::vector<unsigned char> licenseBytes(const nlohmann::json& license_json);
    static std::vector<unsigned char> signatureBytes(const std::string& signature);

    static std::string stringFromState(State state);
    static std::string colorFromState(State state);

    bool isComplete() const;
    bool componentEnabled(Component c) const;

    std::pair<bool, std::string> read(const nlohmann::json& license_json);
    std::pair<bool, std::string> read(const std::string& license_key);
    std::pair<bool, std::string> verify() const;
    std::pair<State, std::string> state() const;

    void print() const;
    std::string componentsAsString() const;

    static const std::string ColorValid;
    static const std::string ColorInvalid;

    std::string                id;

    int                        version    = -1;
    Type                       type       = Type::Free;
    std::set<Component>        components;

    std::string                licensee;

    boost::posix_time::ptime   date_activation;
    boost::posix_time::ptime   date_expiration;
    boost::posix_time::ptime   created;

    std::string                info;

    std::string                signature;
    nlohmann::json             json_blob;
};

} // namespace license
