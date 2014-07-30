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

#include "Logger.h"
#include "Config.h"

using namespace std;

Config::Config ()
: opened_ (false)
{
}

Config::~Config()
{
    opened_=false;
}

void Config::init (std::string config_filename)
{
    assert (!opened_);
    config_filename_=config_filename;
    loadFile ();
    opened_=true;
}

void Config::loadFile()
{
    //const char *filename = "../palantir/conf/palantir.conf";
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
}

void Config::getValue (string id, int* value)
{
    if (!opened_)
        throw std::runtime_error ("Config: getValue: config file was not opened");

    assert (value);
    assert (id.size() > 0);

    if (config_[id].empty())
    {
        logerr  << "Config: getValue: unknown id " << id;
        throw std::runtime_error("Config: getValue: unknown id string "+id);
    }
    *value= atoi((config_[id]).c_str());
    //logdbg  << "Config: getValue: int is now " << *value;
}

void Config::getValue (string id, unsigned int* value)
{
    if (!opened_)
        throw std::runtime_error ("Config: getValue: config file was not opened");

    assert (value);
    assert (id.size() > 0);

    if (config_[id].empty())
    {
        logerr  << "Config: getValue: unknown id " << id;
        throw std::runtime_error("Config: getValue: unknown id string "+id);
    }
    *value= atoi((config_[id]).c_str());
    //logdbg  << "Config: getValue: int is now " << *value;
}

void Config::getValue (string id, double* value)
{
    if (!opened_)
        throw std::runtime_error ("Config: getValue: config file was not opened");

    assert (value);
    assert (id.size() > 0);

    if (config_[id].empty())
    {
        logerr  << "Config: getValue: unknown id " << id;
        throw std::runtime_error("Config: getValue: unknown id string "+id);
    }
    *value= atof((config_[id]).c_str());
    //logdbg  << "Config: getValue: double is now " << *value;
}
void Config::getValue (string id, float* value)
{
    if (!opened_)
        throw std::runtime_error ("Config: getValue: config file was not opened");

    assert (value);
    assert (id.size() > 0);

    double tmp;
    getValue (id, &tmp);
    *value=tmp;
}

void Config::getValue (string id, bool* value)
{
    if (!opened_)
        throw std::runtime_error ("Config: getValue: config file was not opened");

    assert (value);
    assert (id.size() > 0);

    int tmp;
    getValue (id, &tmp);
    *value=tmp>0;
}

void Config::getValue (string id, string* value)
{
    if (!opened_)
        throw std::runtime_error ("Config: getValue: config file was not opened");

    assert (value);
    assert (id.size() > 0);

    if (config_[id].empty())
    {
        logerr  << "Config: getValue: unknown id " << id;
        throw std::runtime_error("Config: getValue: unknown id string "+id);
    }
    value->assign(config_[id]);
    //logdbg  << "Config: getValue: string is now " << *value;
}

bool Config::existsId(std::string id)
{
    assert (id.size() > 0);
    return (config_.find(id) != config_.end());
}

