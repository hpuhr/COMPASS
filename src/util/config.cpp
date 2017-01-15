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

#include <stdlib.h>
#include <limits>
#include <fstream>

#include "logger.h"
#include "config.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;

Config::Config (const std::string &config_filename)
: opened_ (false), config_filename_ (config_filename)
{
    try
    {
        loadFile ();
        assert (opened_);
    }
    catch (exception& e)
    {
        cout << "Config: constructor: exception " << e.what() << '\n';
        opened_ = false;
    }
}

Config::~Config()
{
    opened_=false;
}

void Config::loadFile()
{
    assert (!opened_);

    ifstream grab(config_filename_.c_str());

    //check file exists
    if (!grab)
    {
        throw std::runtime_error ("Config: loadFile: file '"+config_filename_+"' not found");
    }

    while(!grab.eof())
    {
        string param;
        string value;

        grab >> param;

        if (grab.fail())
            break;

        if (param.at(0) == '#')
        {
            grab.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            continue;
        }

        grab >> value;

        if (grab.fail())
        {
            std::cerr  << "Config: loadFile: missing value for " << param << std::endl;
            break;
        }

        if (existsId(param))
        {
            std::cout  << "Config: loadFile: overwriting value of parameter " << param << std::endl;
        }

        config_[param] = value;
        //std::cout << "Config: got id '" << param << "' with value '" << config_[param] << "'" << endl;
    }

    opened_ = true;
}

bool Config::getBool (const std::string &id)
{
    if (!opened_)
        throw std::runtime_error ("Config: getBool: config file was not opened");

    if (!existsId(id))
        throw std::runtime_error ("Config: getBool: config id '"+id+"' not present in configuration");

    unsigned int tmp = getUnsignedInt(id);
    return tmp > 0;
}

int Config::getInt (const std::string &id)
{
    if (!opened_)
        throw std::runtime_error ("Config: getInt: config file was not opened");

    if (!existsId(id))
        throw std::runtime_error ("Config: getInt: config id '"+id+"' not present in configuration");

    return String::intFromString(config_.at(id));
}

unsigned int Config::getUnsignedInt (const std::string &id)
{
    if (!opened_)
        throw std::runtime_error ("Config: getUnsignedInt: config file was not opened");

    if (!existsId(id))
        throw std::runtime_error ("Config: getUnsignedInt: config id '"+id+"' not present in configuration");

    return String::uIntFromString(config_.at(id));
}

double Config::getDouble (const std::string &id)
{
    if (!opened_)
        throw std::runtime_error ("Config: getDouble: config file was not opened");

    if (!existsId(id))
        throw std::runtime_error ("Config: getDouble: config id '"+id+"' not present in configuration");

    return String::doubleFromString(config_.at(id));
}

const std::string &Config::getString (const std::string &id)
{
    if (!opened_)
        throw std::runtime_error ("Config: getValue: config file was not opened");

    if (!existsId(id))
        throw std::runtime_error ("Config: getBool: config id '"+id+"' not present in configuration");

    if (config_[id].empty())
    {
        logerr  << "Config: getValue: unknown id " << id;
        throw std::runtime_error("Config: getValue: unknown id string "+id);
    }
    return config_.at(id);
}

bool Config::existsId(const std::string &id)
{
    assert (id.size() > 0);
    return (config_.count(id) > 0);
}

