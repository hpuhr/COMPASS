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

#include "evaluationdetail.h"

/*****************************************************************************
 * EvaluationDetailComments
 *****************************************************************************/

/**
 */
size_t EvaluationDetailComments::numComments(const std::string& group_id) const 
{
    if (!comments_.has_value())
        return 0;

    auto it = comments_.value().find(group_id);
    if (it == comments_.value().end())
        return 0;

    return it->second.size();
}

/**
 */
EvaluationDetailComments& EvaluationDetailComments::generalComment(const std::string& c)
{
    comment_ = c;
    return *this;
}

/**
 */
const std::string& EvaluationDetailComments::generalComment() const
{
    return comment_;
}

/**
 */
EvaluationDetailComments& EvaluationDetailComments::comment(const std::string& group_id,
                                                            const std::string& comment_id,
                                                            const std::string& c)
{
    comments()[ group_id ][ comment_id ] = c;
    return (*this);
}

/**
 */
const std::string& EvaluationDetailComments::comment(const std::string& group_id,
                                                     const std::string& comment_id) const
{
    return comments()[ group_id ][ comment_id ];
}

/**
 */
EvaluationDetailComments& EvaluationDetailComments::group(const std::string& group_id,
                                                          const CommentGroup& cg)
{
    comments()[ group_id ] = cg;
    return (*this);
}

/**
 */
const EvaluationDetailComments::CommentGroup& EvaluationDetailComments::group(const std::string& group_id) const
{
    return comments()[ group_id ];
}

/*****************************************************************************
 * EvaluationDetail
 *****************************************************************************/

/**
*/
EvaluationDetail::EvaluationDetail(const Timestamp& ts, 
                                   const Position& pos)
:   timestamp_(ts)
{
    addPosition(pos);
}

/**
*/
EvaluationDetail::EvaluationDetail(const Timestamp& ts, 
                                   const Position& pos0,
                                   const Position& pos1)
:   timestamp_(ts)
{
    positions_.resize(2);
    positions_[ 0 ] = pos0;
    positions_[ 1 ] = pos1;
}

/**
*/
EvaluationDetail::EvaluationDetail(const Timestamp& ts, 
                                   const std::vector<Position>& positions)
:   timestamp_(ts)
,   positions_(positions)
{
}

/**
*/
EvaluationDetail& EvaluationDetail::setValue(const Key& key, const QVariant& value)
{
    values_[ key ] = value;

    return *this;
}

/**
*/
EvaluationDetail& EvaluationDetail::setValue(const Key& key, const boost::posix_time::ptime& value)
{
    QString v = QString::fromStdString(Utils::Time::toString(value));
    values_[ key ] = v;

    return *this;
}

/**
*/
EvaluationDetail& EvaluationDetail::setValue(const Key& key, const boost::posix_time::time_duration& value)
{
    double v = Utils::Time::partialSeconds(value);
    values_[ key ] = v;

    return *this;
}

/**
*/
QVariant EvaluationDetail::getValue(const Key& key) const
{
    auto it = values_.find(key);
    if (it == values_.end())
        return {};

    return it->second;
}

/**
*/
EvaluationDetail& EvaluationDetail::addPosition(const Position& p)
{
    positions_.push_back(p);

    return *this;
}

/**
*/
EvaluationDetail& EvaluationDetail::addPosition(const boost::optional<Position>& p)
{
    if (p.has_value())
        addPosition(p.value());
}

/**
*/
int EvaluationDetail::numPositions() const
{
    return (int)positions_.size();
}

/**
*/
const std::vector<EvaluationDetail::Position>& EvaluationDetail::positions() const
{
    return positions_;
}

/**
*/
const EvaluationDetail::Position& EvaluationDetail::position(int idx) const
{
    return positions_.at(idx);
}

/**
*/
EvaluationDetail::Details& EvaluationDetail::genDetails() const
{
    if (!details_.has_value())
        details_ = Details();

    return details_.value();
}

/**
*/
EvaluationDetail& EvaluationDetail::addDetail(const EvaluationDetail& detail)
{
    genDetails().push_back(detail);
    return *this;
}

/**
*/
bool EvaluationDetail::hasDetails() const
{
    return details_.has_value();
}

/**
*/
const EvaluationDetail::Details& EvaluationDetail::details() const
{
    return genDetails();
}

/**
*/
EvaluationDetail& EvaluationDetail::generalComment(const std::string& c)
{
    comments().generalComment(c);
}
