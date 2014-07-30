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

/*
 * DBFilter.cpp
 *
 *  Created on: Oct 25, 2011
 *      Author: sk
 */

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <QVBoxLayout>
#include "DBFilter.h"
#include "DBFilterWidget.h"
#include "DBFilterCondition.h"
#include "Logger.h"
#include "FilterManager.h"

/**
 * Initializes members, registers parameters and creates sub-configurables if class id is DBFilter.
 */
DBFilter::DBFilter(std::string class_id, std::string instance_id, Configurable *parent, bool is_generic)
: Configurable (class_id, instance_id, parent), widget_ (0), is_generic_(is_generic)
{
    registerParameter ("active", &active_, false);
    registerParameter ("op_and_", &op_and_, true);
    registerParameter ("changed", &changed_, false);
    registerParameter ("visible", &visible_, false);
    registerParameter ("name", &name_, instance_id);

    if (class_id_.compare("DBFilter")==0) // else do it in subclass
        createSubConfigurables();
}

/**
 * Deletes and removes sub-filters, deletes and removes conditions, deletes widget if required.
 */
DBFilter::~DBFilter()
{
    logdbg  << "DBFilter: destructor: instance_id " << instance_id_;
    for (unsigned int cnt=0; cnt < sub_filters_.size(); cnt++)
    {
        delete sub_filters_.at(cnt);
    }
    sub_filters_.clear();

    for (unsigned int cnt=0; cnt < conditions_.size(); cnt++)
    {
        delete conditions_.at(cnt);
    }
    conditions_.clear();

    if (widget_)
    {
        delete widget_;
        widget_=0;
    }
}

void DBFilter::widgetIsDeleted ()
{
    widget_=0;
}

/**
 * Sets changed in FilterManager if required, overwrites active_ and distributes the change to the sub-filters.
 * Updates widget if existing.
 */
void DBFilter::setActive (bool active)
{
    if (active_ && !active)
        FilterManager::getInstance().setChanged();

    active_=active;

    for (unsigned int cnt=0; cnt < sub_filters_.size(); cnt++)
    {
        sub_filters_.at(cnt)->setActive(active_);
    }

    changed_=true;

    if (widget_)
        widget_->update();
}
bool DBFilter::getActive ()
{
    return active_;
}

/**
 * Returns if any change occurred in this filter, a sub-condition or a sub-filter.
 */
bool DBFilter::getChanged ()
{
    bool ret = changed_;

    for (unsigned int cnt=0; cnt < conditions_.size(); cnt++)
    {
        ret  |= conditions_.at(cnt)->getChanged();
    }

    for (unsigned int cnt=0; cnt < sub_filters_.size(); cnt++)
    {
        ret  |= sub_filters_.at(cnt)->getChanged();
    }

    return ret;
}

/**
 * Sets changed_ and propagates function to all sub-conditions and sub-filters.
 *
 */
void DBFilter::setChanged (bool changed)
{
    changed_=changed;

    for (unsigned int cnt=0; cnt < conditions_.size(); cnt++)
    {
        conditions_.at(cnt)->setChanged(changed);
    }

    for (unsigned int cnt=0; cnt < sub_filters_.size(); cnt++)
    {
        sub_filters_.at(cnt)->setChanged(changed);
    }

}

bool DBFilter::getVisible ()
{
    return visible_;
}
void DBFilter::setVisible (bool visible)
{
    visible_=visible;
}

void DBFilter::setName (std::string name)
{
    name_=name;
    if (widget_)
        widget_->update();
}

void DBFilter::addSubFilter (DBFilter *filter)
{
    sub_filters_.push_back (filter);
}

/**
 * Checks if any sub-filter or sub-condition filter the DBO of the supplied type.
 */
bool DBFilter::filters (DB_OBJECT_TYPE dbo_type)
{
    bool ret = false;

    for (unsigned int cnt=0; cnt < conditions_.size(); cnt++)
    {
        ret  |= conditions_.at(cnt)->filters(dbo_type);
    }

    for (unsigned int cnt=0; cnt < sub_filters_.size(); cnt++)
    {
        ret  |= sub_filters_.at(cnt)->filters(dbo_type);
    }

    return ret;
}

/**
 * If active, returns concatenated condition strings from all sub-conditions and sub-filters, else returns empty string.
 */
std::string DBFilter::getConditionString (DB_OBJECT_TYPE dbo_type, bool &first, std::vector<std::string> &variable_names)
{
    std::stringstream ss;

    if (active_)
    {
        for (unsigned int cnt=0; cnt < conditions_.size(); cnt++)
        {
            std::string text = conditions_.at(cnt)->getConditionString(dbo_type, first, variable_names);
            ss << text;

            if (text.size() > 0)
                first=false;
        }


        for (unsigned int cnt=0; cnt < sub_filters_.size(); cnt ++)
        {
            std::string text = sub_filters_.at(cnt)->getConditionString(dbo_type, first, variable_names);
            ss << text;

            if (text.size() > 0)
                first=false;
        }
    }

    logdbg  << "DBFilter " << instance_id_ << ": getConditionString: here '" <<ss.str() << "'";

    return ss.str();
}

DBFilterWidget *DBFilter::getWidget ()
{
    assert (widget_);
    return widget_;
}

void DBFilter::setAnd (bool op_and)
{
    if (op_and_ != op_and)
    {
        op_and_=op_and;

        changed_=true;

        if (widget_)
            widget_->update();
    }
}

void DBFilter::invert ()
{
    op_and_=!op_and_;

    for (unsigned int cnt=0; cnt < conditions_.size(); cnt++)
    {
        conditions_.at(cnt)->invert();
    }

    for (unsigned int cnt=0; cnt < sub_filters_.size(); cnt++)
    {
        sub_filters_.at(cnt)->invert();
    }
    changed_=true;

    if (widget_)
        widget_->update();
}

/**
 * Can generate instances of DBFilterWidget, DBFilterCondition or DBFilter.
 *
 * \exception std::runtime_error if unknown class_id
 */
void DBFilter::generateSubConfigurable (std::string class_id, std::string instance_id)
{
    logdbg  << "DBFilter: generateSubConfigurable: " << class_id_ << " instance " << instance_id_;

    if (class_id.compare("DBFilterWidget") == 0)
    {
        logdbg  << "DBFilter: generateSubConfigurable: generating widget";
        assert (!widget_);
        widget_ = new DBFilterWidget (*this, class_id, instance_id);
    }
    else
        if (class_id.compare("DBFilterCondition") == 0)
        {
            logdbg  << "DBFilter: generateSubConfigurable: generating condition";
            DBFilterCondition *condition = new DBFilterCondition (instance_id, this);
            conditions_.push_back (condition);

            if (widget_) // bit of a hack. think about order of generation.
                widget_->updateChildWidget();
        }
        else if (class_id.compare("DBFilter") == 0)
        {
            DBFilter *filter = new DBFilter (class_id, instance_id, this);
            addSubFilter (filter);
        }
        else
            throw std::runtime_error ("DBFilter: generateSubConfigurable: unknown class_id "+class_id);
}

/**
 * Creates widget_ if required and adds all sub-filter widgets to it.
 */
void DBFilter::checkSubConfigurables ()
{
    logdbg  << "DBFilter: checkSubConfigurables: " << class_id_;

    if (!widget_)
    {
        logdbg  << "DBFilter: checkSubConfigurables: generating generic filter widget";
        widget_ = new DBFilterWidget (*this, "DBFilterWidget",instance_id_+"Widget0");
    }
    assert (widget_);

    //TODO
    for (unsigned int cnt=0; cnt < sub_filters_.size(); cnt++)
    {
        DBFilterWidget *filter = sub_filters_.at(cnt)->getWidget();
        QObject::connect((QWidget*)filter, SIGNAL( possibleFilterChange() ), (QWidget*)widget_, SLOT( possibleSubFilterChange() ));
        widget_->addChildWidget (filter);
    }
}

/**
 * Calls reset on all sub-conditions or sub-filters, sets changed_ flag.
 */
void DBFilter::reset ()
{
    for (unsigned int cnt=0; cnt < conditions_.size(); cnt++)
    {
        conditions_.at(cnt)->reset();
    }

    for (unsigned int cnt=0; cnt < sub_filters_.size(); cnt++)
    {
        sub_filters_.at(cnt)->reset();
    }
    changed_=true;
}

void DBFilter::deleteCondition (DBFilterCondition *condition)
{
    std::vector <DBFilterCondition *>::iterator it = find (conditions_.begin(), conditions_.end(), condition);
    assert (it != conditions_.end());
    conditions_.erase (it);
    delete condition;
}

void DBFilter::destroy ()
{
    FilterManager::getInstance().deleteFilter (this);
}

//void DBFilter::parseFilterDefinition (std::string def)
//{
//	boost::erase_all(def, " ");
//
//	logdbg << "DBFilter: parseFilterDefinition: start:" << def << std::endl;
//
//	assert (!condition_);
//
//	// remove start and end brackets if exists
//	boost::regex const start_end_brackets ("\\(.*\\)");
//	while (boost::regex_match(def, start_end_brackets))
//	{
//		logdbg << "DBFilter: parseFilterDefinition: removing start and end bracked" << std::endl;
//		def = def.substr(1, def.size()-2);
//		brackets_=true;
//	}
//
//	logdbg << "DBFilter: parseFilterDefinition: no brackets:" <<  def << std::endl;
//	//find positions of matching brackets
//	std::vector<std::pair<int,int> > bracket_pair_indexes;
//	std::vector<std::pair<int, char> > bracket_stack;
//
//	for (unsigned int cnt=0; cnt < def.size(); cnt ++)
//	{
//		if (def.at(cnt) == '(')
//		{
//			bracket_stack.push_back(std::pair <int, char> (cnt, '('));
//		}
//		else if (def.at(cnt) == ')')
//		{
//			std::pair <int, char> last_open = bracket_stack.back();
//			if (last_open.second != '(')
//			{
//				logerr << "DBFilter: parseFilterDefinition: bracket error, close without open" << std::endl;
//				return;
//			}
//			else
//			{
//				bracket_pair_indexes.push_back(std::pair<int, int> (last_open.first, cnt));
//				bracket_stack.pop_back();
//			}
//		}
//	}
//
//	if (bracket_stack.size() != 0)
//	{
//		logerr << "DBFilter: parseFilterDefinition: bracket error, open without close" << std::endl;
//		return;
//	}
//
//	for (unsigned int cnt=0; cnt < bracket_pair_indexes.size(); cnt++)
//	{
//		logdbg << "DBFilter: parseFilterDefinition: bracket pair open " << bracket_pair_indexes.at(cnt).first << " close " << bracket_pair_indexes.at(cnt).second  << std::endl;
//	}
//
//	//remove nested brackets
//	if (bracket_pair_indexes.size() >= 2)
//	{
//		for (unsigned int cnt=0; cnt < bracket_pair_indexes.size(); cnt++ )
//		{
//			for (unsigned int cnt2=0; cnt2 < bracket_pair_indexes.size(); cnt2++ )
//			{
//				if (cnt == cnt2)
//					continue;
//
//				if (bracket_pair_indexes.at(cnt).first < bracket_pair_indexes.at(cnt2).first && bracket_pair_indexes.at(cnt).second > bracket_pair_indexes.at(cnt2).second)
//				{
//					logdbg << "DBFilter: parseFilterDefinition: found nested bracket pair " << bracket_pair_indexes.at(cnt2).first << " close " << bracket_pair_indexes.at(cnt2).second  << std::endl;
//					bracket_pair_indexes.erase(bracket_pair_indexes.begin()+cnt2);
//				}
//
//			}
//		}
//	}
//	for (unsigned int cnt=0; cnt < bracket_pair_indexes.size(); cnt++)
//	{
//		logdbg << "DBFilter: parseFilterDefinition: bottom bracket pair open " << bracket_pair_indexes.at(cnt).first << " close " << bracket_pair_indexes.at(cnt).second  << std::endl;
//	}
//
//	boost::regex andor("AND|OR");
//	boost::sregex_iterator i(def.begin (), def.end (),  andor);
//	boost::sregex_iterator j;
//	int start_pos=0;
//	int end_pos=0;
//
//	int pos=0;
//	std::string sep;
//	bool inside_brackets;
//	std::string last_valid_sep;
//
//	std::vector<std::pair<std::string,std::string> > filter_expression_pairs;
//
//	for(; i!=j; ++i)
//	{
//		pos = (*i).position();
//		sep = i->str();
//		logdbg << "DBFilter: parseFilterDefinition: pos " << pos << " : " << sep <<  std::endl;
//	  inside_brackets=false;
//
//		for (unsigned int cnt=0; cnt < bracket_pair_indexes.size(); cnt++)
//		{
//			if (pos-sep.size() > bracket_pair_indexes.at(cnt).first && pos-sep.size() < bracket_pair_indexes.at(cnt).second)
//				inside_brackets=true;
//		}
//
//		if (!inside_brackets)
//		{
//			if (end_pos == 0) //first one
//			{
//				end_pos = pos;
//			}
//			else
//			{
//				start_pos=end_pos;
//				end_pos=pos;
//			}
//			std::string expr = def.substr(start_pos, end_pos-start_pos);
//			logdbg << "DBFilter: parseFilterDefinition: found expression at bottom level '" << last_valid_sep << ":"<< expr << "'" << std::endl;
//			filter_expression_pairs.push_back(std::pair <std::string, std::string > (last_valid_sep,expr));
//			last_valid_sep=sep;
//			end_pos += sep.size();
//		}
//		else
//		{
//			for (unsigned int cnt=0; cnt < bracket_pair_indexes.size(); cnt++)
//			{
//				if (pos-1 == bracket_pair_indexes.at(cnt).second)
//				{
//					std::string expr = def.substr(bracket_pair_indexes.at(cnt).first, bracket_pair_indexes.at(cnt).second-bracket_pair_indexes.at(cnt).first+1);
//					logdbg << "DBFilter: parseFilterDefinition: found bracked expression at bottom level '" << last_valid_sep << ":"<< expr << "'" << std::endl;
//					filter_expression_pairs.push_back(std::pair <std::string, std::string > (last_valid_sep,expr));
//					last_valid_sep=sep;
//				}
//			}
//		}
//	}
//
//	if (pos != 0) // last part
//	{
//		pos += sep.size();
//		inside_brackets=false;
//		logdbg << "DBFilter: parseFilterDefinition: last expr, checking pos " << pos << std::endl;
//
//		for (unsigned int cnt=0; cnt < bracket_pair_indexes.size(); cnt++)
//		{
//			if (pos+1 > bracket_pair_indexes.at(cnt).first && pos < bracket_pair_indexes.at(cnt).second)
//				inside_brackets=true;
//		}
//
//		if (!inside_brackets)
//		{
//			start_pos=pos;
//			end_pos=def.size();
//			std::string expr = def.substr(start_pos, end_pos-start_pos);
//			logdbg << "DBFilter: parseFilterDefinition: found last expression at bottom level '" << sep << ":"<< expr << "'" << std::endl;
//			filter_expression_pairs.push_back(std::pair <std::string, std::string > (last_valid_sep,expr));
//		}
//		else
//		{
//			for (unsigned int cnt=0; cnt < bracket_pair_indexes.size(); cnt++)
//			{
//				if (pos >= bracket_pair_indexes.at(cnt).first || pos <= bracket_pair_indexes.at(cnt).second)
//				{
//					std::string expr = def.substr(bracket_pair_indexes.at(cnt).first, bracket_pair_indexes.at(cnt).second-bracket_pair_indexes.at(cnt).first+1);
//					logdbg << "DBFilter: parseFilterDefinition: found last bracked expression at bottom level '" << last_valid_sep << ":"<< expr << "'" << std::endl;
//					filter_expression_pairs.push_back(std::pair <std::string, std::string > (last_valid_sep,expr));
//					last_valid_sep=sep;
//				}
//			}
//		}
//	}
//
//	if (bracket_pair_indexes.size() == 0 && pos == 0) // only 1 statement and no brackets
//	{
//		filter_expression_pairs.push_back(std::pair<std::string, std::string> ("", def));
//	}
//
//	for (unsigned int cnt=0; cnt < filter_expression_pairs.size(); cnt++)
//	{
//		logdbg << "DBFilter: parseFilterDefinition: filter pair '" << filter_expression_pairs.at(cnt).first << ":" << filter_expression_pairs.at(cnt).second  << "'" <<  std::endl;
//		std::string sep = filter_expression_pairs.at(cnt).first;
//		std::string expr = filter_expression_pairs.at(cnt).second;
//
//		if (sep.compare("") == 0)
//		{
//			// check expression
//			boost::regex const check_expr ("[a-zA-Z0-9\\.]+(>|<|=|>=|<=)[a-zA-Z0-9\\.]+");
//			if (boost::regex_match(expr, check_expr ))
//			{
//				logdbg << "DBFilter: parseFilterDefinition: got valid expression '" << expr << "'" << std::endl;
//				boost::regex operands(">|<|=|>=|<=");
//				boost::sregex_iterator x(expr.begin (), expr.end (),  operands);
//				boost::sregex_iterator y;
//
//				int pos=0;
//				std::string op;
//
//				for(; x!=y; ++x)
//				{
//					pos = (*x).position();
//					op = x->str();
//				}
//
//				// create and assing values to condition
//				std::string parameter = expr.substr (0, pos);
//				std::string value = expr.substr (pos+op.size(), expr.size()-pos-op.size());
//
//				if (!db_->existsDBVariable(parameter))
//				{
//					logerr  << "DBFilter: parseFilterDefinition: unknown parameter " << parameter;
//					return;
//				}
//				DBVariable *var = db_->getDBVariable(parameter);
//
//				condition_ = new DBFilterCondition(var,op, value);
//
//				logdbg << "DBFilter: parseFilterDefinition: set filter values parameter '" << parameter << "' " << " operator '" << op << "' value '" << value << "'"  << std::endl;
//			}
//			else
//			{
//				logerr << "DBFilter: parseFilterDefinition: got invalid expression '" << expr << "'" << std::endl;
//			}
//
//		}
//		else if (sep.compare("OR") == 0)
//		{
//			logdbg << "DBFilter: parseFilterDefinition: or element" << std::endl;
//			DBFilter *tmp = new DBFilter(db_);
//			tmp->parseFilterDefinition(expr);
//			or_filters_.push_back (tmp);
//		}
//		else if (sep.compare("AND") == 0)
//		{
//			logdbg << "DBFilter: parseFilterDefinition: and element" << std::endl;
//			DBFilter *tmp = new DBFilter(db_);
//			tmp->parseFilterDefinition(expr);
//			and_filters_.push_back (tmp);
//		}
//	}
//
//	logdbg << "DBFilter: parseFilterDefinition: done" <<std::endl;
//}


