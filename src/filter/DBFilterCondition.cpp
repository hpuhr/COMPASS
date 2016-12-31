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
 * DBCondition.cpp
 *
 *  Created on: Nov 6, 2011
 *      Author: sk
 */

#include <sstream>
#include <cassert>
#include <QHBoxLayout>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>

#include "DBFilterCondition.h"
#include "DBObjectManager.h"
#include "DBSchemaManager.h"
#include "MetaDBTable.h"
#include "DBSchema.h"
#include "ATSDB.h"
#include "DBFilter.h"

#include "String.h"

using namespace Utils;

/**
 * Initializes members, registers parameters, create GUI elements.
 */
DBFilterCondition::DBFilterCondition(std::string instance_id, DBFilter *filter_parent)
: Configurable ("DBFilterCondition", instance_id, filter_parent), filter_parent_(filter_parent), variable_(0), changed_(true)
{
    registerParameter ("operator", &operator_, ">");
    registerParameter ("op_and", &op_and_, true);
    registerParameter ("absolute_value", &absolute_value_, false);
    registerParameter ("variable_type", &variable_dbo_type_, "");
    registerParameter ("variable_name", &variable_name_, "");

    if (!DBObjectManager::getInstance().existsDBOVariable (variable_dbo_type_, variable_name_))
        throw std::runtime_error ("DBFilterCondition: constructor: dbo variable '"+variable_name_+"' does not exist");

    variable_ = DBObjectManager::getInstance().getDBOVariable (variable_dbo_type_, variable_name_);

    // TODO ADD THIS LATER
    //variable_->addMinMaxObserver(this);

    registerParameter ("reset_value", &reset_value_, std::string("value"));
    registerParameter ("value", &value_, "");

    widget_ = new QWidget ();
    QHBoxLayout *layout = new QHBoxLayout ();
    layout->setContentsMargins (0, 0, 0, 0);
    layout->setSpacing (0);

    label_ = new QLabel(tr((variable_name_+" "+operator_).c_str()));
    layout->addWidget(label_);

    edit_ = new QLineEdit(tr(value_.c_str()));
    connect(edit_, SIGNAL( returnPressed() ), this, SLOT( valueChanged() ));
    layout->addWidget(edit_);

    widget_->setLayout (layout);
}

DBFilterCondition::~DBFilterCondition()
{
    //TODO ADD THIS LATER
    //variable_->removeMinMaxObserver(this);
}

void DBFilterCondition::invert ()
{
    //TODO
    //op_and_=!op_and_;
}

/**
 * Returns if variable_ exists in DBObject of type
 */
bool DBFilterCondition::filters (const std::string &dbo_type)
{
    return variable_->existsIn(dbo_type);
}

std::string DBFilterCondition::getConditionString (const std::string &dbo_type, bool &first, std::vector<std::string> &variable_names)
{
    std::stringstream ss;

    std::string variable_prefix;
    std::string variable_suffix;

    if (absolute_value_)
    {
        variable_prefix="ABS("+variable_prefix;
        variable_suffix=variable_suffix+")";
    }

    assert (variable_);

    if (operator_.compare("|=") == 0)
    {
        std::vector<std::string> chunks = String::split(value_, ',');

        if (chunks.size() > 0)
        {
            //loginf  << "only subs";
            if (!variable_->isMetaVariable())
            {
                std::string meta_table_name = variable_->getCurrentMetaTable();
                assert (DBSchemaManager::getInstance().getCurrentSchema()->hasMetaTable(meta_table_name));
                MetaDBTable *table =DBSchemaManager::getInstance().getCurrentSchema()->getMetaTable(meta_table_name);
                std::string table_db_name = table->getTableDBNameForVariable(variable_->getId());
                //ss << table_db_name << "." << variable_->id_;

                if (variable_->getDBOType() == dbo_type)
                {
                    if (!first)
                    {
                        if (op_and_)
                            ss << " AND ";
                        else
                            ss << " OR ";

                        first=false;
                    }

                    ss  << "(";

                    for (unsigned int cnt2 = 0; cnt2 < chunks.size(); cnt2++)
                    {
                        if (cnt2 != 0)
                            ss << " OR ";

                        assert (false);
                        // FIX representation
//                        ss << variable_prefix << table_db_name << "." << variable_->getId()  << variable_suffix << " = "
//                                << variable_->getValueFromRepresentation(chunks.at(cnt2), true);
                    }
                    ss  << ")";

                    if (chunks.size() > 0) // variable added
                    {
                        if (find (variable_names.begin(), variable_names.end(), variable_->getId()) == variable_names.end())
                            variable_names.push_back(variable_->getId());
                    }
                }
            }
            else // is meta
            {
                const std::map <std::string, std::string> &subvars = variable_->getSubVariables ();
                std::map <std::string, std::string>::const_iterator it;

                for (it =subvars.begin(); it != subvars.end(); it++)
                {
                    DBOVariable *tmpvar = variable_->getFor (it->first);

                    std::string meta_table_name = tmpvar->getCurrentMetaTable();
                    assert (DBSchemaManager::getInstance().getCurrentSchema()->hasMetaTable(meta_table_name));
                    MetaDBTable *table =DBSchemaManager::getInstance().getCurrentSchema()->getMetaTable(meta_table_name);
                    std::string table_db_name = table->getTableDBNameForVariable(tmpvar->getId());
                    //ss << table_db_name << "." << variable_->id_;


                    if (tmpvar->getDBOType() == dbo_type)
                    {
                        if (!first)
                        {
                            if (op_and_)
                                ss << " AND ";
                            else
                                ss << " OR ";

                            first=false;
                        }

                        ss  << "(";

                        for (unsigned int cnt2 = 0; cnt2 < chunks.size(); cnt2++)
                        {
                            if (cnt2 != 0)
                                ss << " OR ";

                            //TODO FIX REPRESENTATION
                            assert (false);
/*                            ss << variable_prefix << table_db_name<<"." <<tmpvar->id_  << variable_suffix << " = "
                                    << variable_->getValueFromRepresentation(chunks.at(cnt2), true)*/;
                        }
                        ss  << ")";

                        if (chunks.size() > 0) // variable added
                        {
                            if (find (variable_names.begin(), variable_names.end(), tmpvar->getId()) == variable_names.end())
                                variable_names.push_back(tmpvar->getId());
                        }

                    }
                }
            }
        }
    }
    else
    {
        if (!variable_->isMetaVariable())
        {
            std::string meta_table_name = variable_->getCurrentMetaTable();
            assert (DBSchemaManager::getInstance().getCurrentSchema()->hasMetaTable(meta_table_name));
            MetaDBTable *table =DBSchemaManager::getInstance().getCurrentSchema()->getMetaTable(meta_table_name);
            std::string table_db_name = table->getTableDBNameForVariable(variable_->getId());
            //ss << table_db_name << "." << variable_->id_;

            if (variable_->getDBOType() == dbo_type)
            {
                if (!first)
                {
                    if (op_and_)
                        ss << " AND ";
                    else
                        ss << " OR ";

                    first=false;
                }

                //TODO FIX REPRESENATION
                assert (false);
//                ss << variable_prefix << table_db_name << "." << variable_->id_  << variable_suffix  << " " << operator_ << " "
//                        << variable_->getValueFromRepresentation(value_, true);

                if (find (variable_names.begin(), variable_names.end(), variable_->getId()) == variable_names.end())
                    variable_names.push_back(variable_->getId());
            }
        }
        else // is meta
        {
            const std::map <std::string, std::string> &subvars = variable_->getSubVariables ();
            std::map <std::string, std::string>::const_iterator it;

            for (it =subvars.begin(); it != subvars.end(); it++)
            {
                DBOVariable *tmpvar = variable_->getFor (it->first);

                std::string meta_table_name = tmpvar->getCurrentMetaTable();
                assert (DBSchemaManager::getInstance().getCurrentSchema()->hasMetaTable(meta_table_name));
                MetaDBTable *table =DBSchemaManager::getInstance().getCurrentSchema()->getMetaTable(meta_table_name);
                std::string table_db_name = table->getTableDBNameForVariable(tmpvar->getId());
                //ss << table_db_name << "." << variable_->id_;

                if (tmpvar->getDBOType() == dbo_type)
                {
                    if (!first)
                    {
                        if (op_and_)
                            ss << " AND ";
                        else
                            ss << " OR ";

                        first=false;
                    }
                    //TODO FIX REPRESENTATION
                    assert (false);
//                    ss << variable_prefix << table_db_name << "." << tmpvar->id_  << variable_suffix  << " " << operator_ << " "
//                            << variable_->getValueFromRepresentation(value_, true);

                    if (find (variable_names.begin(), variable_names.end(), tmpvar->getId()) == variable_names.end())
                        variable_names.push_back(tmpvar->getId());

                }
            }
        }
    }

    if (ss.str().size() > 0)
    loginf  << "DBFilterCondition " << instance_id_<<": getConditionString: '" << ss.str()<<"'";

    return ss.str();
}

/**
 * Checks if value_ is different than edit_ value, if yes sets changed_ and emits possibleFilterChange.
 */
void DBFilterCondition::valueChanged ()
{
    logdbg  << "DBFilterCondition: valueChanged";
    assert  (edit_);

    if (value_.compare(edit_->text().toStdString()) != 0)
    {
        value_ = edit_->text().toStdString();

        changed_=true;

        emit possibleFilterChange();
    }
}

/**
 * Sets the variable if required, sets the variable_name_ and calls reset.
 */
void DBFilterCondition::setVariable (DBOVariable *variable)
{
    if (variable != variable_)
    {
        variable_=variable;

        variable_name_=variable_->getName();

        reset();
    }
}

void DBFilterCondition::update ()
{
    assert (variable_);

    label_->setText(tr((variable_name_+" "+operator_).c_str()));
    edit_->setText (tr(value_.c_str()));
}

void DBFilterCondition::reset ()
{
    std::string value;

    if (reset_value_.compare ("MIN") == 0 || reset_value_.compare ("MAX") == 0)
    {
        // FIX MINMAX
        assert (false);
//        if (!variable_->hasMinMaxInfo())
//        {
//            value = "LOADING";
//            variable_->buildMinMaxInfo();
//        }
//        else
//        {
//            if (reset_value_.compare ("MIN") == 0)
//            {
//                value = variable_->getRepresentationFromValue(variable_->getMinString());
//                loginf << "DBFilterCondition: reset: value " << variable_->getMinString() << " repr " << value;
//            }
//            else if (reset_value_.compare ("MAX") == 0)
//            {
//                value = variable_->getRepresentationFromValue(variable_->getMaxString());
//                loginf << "DBFilterCondition: reset: value " << variable_->getMaxString() << " repr " << value;
//            }
//        }
    }
    else
        value=reset_value_;

    value_=value;

    update();
}

void DBFilterCondition::notifyMinMax (DBOVariable *variable)
{
    loginf << "DBFilterCondition: notifyMinMax";

    std::string value;

    //TODO FIX MINMAX
    assert (false);

//    assert (variable_->hasMinMaxInfo());
//    if (reset_value_.compare ("MIN") == 0)
//        value = variable_->getRepresentationFromValue(variable_->getMinString());
//    else if (reset_value_.compare ("MAX") == 0)
//        value = variable_->getRepresentationFromValue(variable_->getMaxString());

    value_=value;
    update();
}
