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

#include "eval/results/evaluationdetail.h"

#include <functional>

#include <boost/optional.hpp>

namespace EvaluationRequirementResult
{

enum class DetailValuePositionMode
{
    EventPosition = 0, // event position only
    EventRefPosition,  // event reference position only
    AllPositions       // all event positions
};

/**
 * Describes how to obtain a certain value from an evaluation detail.
*/
class DetailValueSource
{
public:
    typedef std::function<boost::optional<double>(const EvaluationDetail&)> DetailValueFunc;

    DetailValueSource() {}
    DetailValueSource(int value_id) : value_id_(value_id) {}
    DetailValueSource(const DetailValueFunc& value_func) : value_func_(value_func) {}

    /**
    */
    bool isValid() const
    {
        return (value_func_ || value_id_ >= 0);
    }

    /**
    */
    void setSource(int value_id)
    {
        value_id_   = value_id;
        value_func_ = {};
    }
    /**
    */
    void setSource(const DetailValueFunc& value_func)
    {
        value_func_ = value_func;
        value_id_   = -1;
    }

    /**
    */
    boost::optional<double> getValue(const EvaluationDetail& detail) const
    {
        assert(isValid());

        //get value via functional?
        if (value_func_)
            return value_func_(detail);

        //get value stored in detail
        auto value = detail.getValue(value_id_);

        //value might not be set
        if (!value.isValid())
            return {};

        //value must be convertable to double
        bool ok;
        double v = value.toDouble(&ok);
        assert(ok);

        return v;
    }

private:
    DetailValueFunc value_func_;
    int             value_id_ = -1;
};

}
