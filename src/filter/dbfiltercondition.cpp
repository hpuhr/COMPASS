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

#include <boost/algorithm/string/join.hpp>
//#include <boost/algorithm/string.hpp>

#include "dbfiltercondition.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "metadbovariable.h"
#include "dbschemamanager.h"
#include "metadbtable.h"
#include "dbschema.h"
#include "dbtable.h"
#include "dbtablecolumn.h"
#include "atsdb.h"
#include "dbfilter.h"
#include "unitmanager.h"
#include "unit.h"

#include "stringconv.h"

using namespace Utils;

/**
 * Initializes members, registers parameters, create GUI elements.
 */
DBFilterCondition::DBFilterCondition(const std::string &class_id, const std::string &instance_id, DBFilter *filter_parent)
: Configurable (class_id, instance_id, filter_parent), filter_parent_(filter_parent), variable_(nullptr), meta_variable_(nullptr), changed_(true)
{
    registerParameter ("operator", &operator_, ">");
    registerParameter ("op_and", &op_and_, true);
    registerParameter ("absolute_value", &absolute_value_, false);
    registerParameter ("variable_dbo_name", &variable_dbo_name_, "");
    registerParameter ("variable_name", &variable_name_, "");

    // DBOVAR LOWERCASE HACK
    //boost::algorithm::to_lower(variable_name_);

    if (variable_dbo_name_ == META_OBJECT_NAME)
    {
        if (!ATSDB::instance().objectManager().existsMetaVariable(variable_name_))
            throw std::runtime_error ("DBFilterCondition: constructor: meta dbo variable '"+variable_name_+"' does not exist");
        meta_variable_ = &ATSDB::instance().objectManager().metaVariable(variable_name_);
    }
    else
    {
        if (!ATSDB::instance().objectManager().existsObject(variable_dbo_name_) || !ATSDB::instance().objectManager().object(variable_dbo_name_).hasVariable(variable_name_))
            throw std::runtime_error ("DBFilterCondition: constructor: dbo variable '"+variable_name_+"' does not exist");

        variable_ = &ATSDB::instance().objectManager().object(variable_dbo_name_).variable(variable_name_);
    }

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
    connect(edit_, SIGNAL(textChanged(QString)), this, SLOT( valueChanged() ));
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
bool DBFilterCondition::filters (const std::string &dbo_name)
{
    if (meta_variable_)
    {
        return meta_variable_->existsIn(dbo_name);
    }
    else
        return variable_dbo_name_ == dbo_name;
}

std::string DBFilterCondition::getConditionString (const std::string &dbo_name, bool &first, std::vector <DBOVariable*>& filtered_variables)
{
    logdbg << "DBFilterCondition: getConditionString: object " << dbo_name << " first " << first;
    std::stringstream ss;

    std::string variable_prefix;
    std::string variable_suffix;

    if (absolute_value_)
    {
        variable_prefix="ABS("+variable_prefix;
        variable_suffix=variable_suffix+")";
    }

    assert (variable_ || meta_variable_);

    DBOVariable *variable=nullptr;

    if (meta_variable_)
    {
        assert (meta_variable_->existsIn(dbo_name));
        variable = &meta_variable_->getFor(dbo_name);
    }
    else
        variable = variable_;

    const DBTableColumn &column = variable->currentDBColumn();
    const MetaDBTable &meta_table = variable->currentMetaTable();
    std::string table_db_name = meta_table.tableFor(column.name()).name();

    if (!first)
    {
        if (op_and_)
            ss << " AND ";
        else
            ss << " OR ";
    }
    first=false;

    ss << variable_prefix << table_db_name << "." << column.name() << variable_suffix << " " << operator_ << getTransformedValue (value_, variable);

    if (find (filtered_variables.begin(), filtered_variables.end(), variable) == filtered_variables.end())
        filtered_variables.push_back(variable);


    if (ss.str().size() > 0)
        logdbg  << "DBFilterCondition " << instance_id_<<": getConditionString: '" << ss.str()<<"'";

    return ss.str();
}

/**
 * Checks if value_ is different than edit_ value, if yes sets changed_ and emits possibleFilterChange.
 */
void DBFilterCondition::valueChanged ()
{
    logdbg  << "DBFilterCondition: valueChanged";
    assert  (edit_);
    assert (variable_ || meta_variable_);

    std::string new_value = edit_->text().toStdString();

    std::vector <DBOVariable*> variables;

    if (meta_variable_)
    {
        for (auto var_it : meta_variable_->variables())
            variables.push_back(&var_it.second);
    }
    else
        variables.push_back(variable_);

    invalid_ = true;

    try
    {
        for (auto var_it : variables)
        {
            std::string transformed_value = getTransformedValue (new_value, var_it);
            logdbg  << "DBFilterCondition: valueChanged: transformed value " << transformed_value;
        }
        invalid_ = false;
    }
    catch(std::exception& e)
    {
        logdbg  << "DBFilterCondition: valueChanged: exception thrown: " << e.what();
    }
    catch(...)
    {
        logdbg  << "DBFilterCondition: valueChanged: exception thrown";
    }

    if (!invalid_ && value_ != new_value)
    {
        value_ = new_value;

        changed_=true;

        emit possibleFilterChange();
    }

    loginf  << "DBFilterCondition: valueChanged: value_ '" << value_ << "' invalid " << invalid_;

    if (invalid_)
        edit_->setStyleSheet("QLineEdit { background: rgb(255, 100, 100); selection-background-color: rgb(255, 200, 200); }");
    else
        edit_->setStyleSheet("QLineEdit { background: rgb(255, 255, 255); selection-background-color: rgb(200, 200, 200); }");
}

/**
 * Sets the variable if required, sets the variable_name_ and calls reset.
 */
void DBFilterCondition::setVariable (DBOVariable *variable)
{
    if (variable != variable_)
    {
        variable_=variable;
        variable_name_=variable_->name();

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

bool DBFilterCondition::invalid() const
{
    return invalid_;
}

std::string DBFilterCondition::getTransformedValue (const std::string& untransformed_value, DBOVariable *variable)
{
    assert (variable);
    const DBTableColumn &column = variable->currentDBColumn();

    std::vector<std::string> value_strings;
    std::vector<std::string> transformed_value_strings;

    if (operator_ == "IN")
    {
        value_strings = String::split(untransformed_value, ',');
    }
    else
    {
        value_strings.push_back(untransformed_value);
    }

    logdbg << "DBFilterCondition: getTransformedValue: in value strings '" << boost::algorithm::join(value_strings, ",") << "'";

    for (auto value_it : value_strings)
    {
        std::string value_str = value_it;

        if (variable->representation() != String::Representation::STANDARD)
            value_str = String::getValueStringFromRepresentation(value_str, variable->representation()); // fix representation

        logdbg << "DBFilterCondition: getTransformedValue: value string " << value_str;

        if (column.unit() != variable->unit()) // do unit conversion stuff
        {
            logdbg << "DBFilterCondition: getTransformedValue: variable " << variable->name() << " of same dimension has different units " << column.unit() << " " << variable->unit();

            const Dimension &dimension = UnitManager::instance().dimension (variable->dimension());
            double factor = dimension.getFactor (column.unit(), variable->unit());
            logdbg  << "DBFilterCondition: getTransformedValue: correct unit transformation with factor " << factor;

            switch (variable->dataType())
            {
            case PropertyDataType::BOOL:
            case PropertyDataType::UCHAR:
            case PropertyDataType::UINT:
            case PropertyDataType::ULONGINT:
            {
                unsigned long value = std::stoul(value_str);
                value /= factor;
                value_str = std::to_string(value);
                break;
            }
            case PropertyDataType::CHAR:
            case PropertyDataType::INT:
            case PropertyDataType::LONGINT:
            {
                long value = std::stol(value_str);
                value /= factor;
                value_str = std::to_string(value);
                break;
            }
            case PropertyDataType::FLOAT:
            case PropertyDataType::DOUBLE:
            {
                double value = std::stod(value_str);
                value /= factor;
                value_str = Utils::String::getValueString(value);
                break;
            }
            case PropertyDataType::STRING:
                logerr << "DBFilterCondition: getTransformedValue: unit transformation for string variable " << variable->name() << " impossible";
                break;
            default:
                logerr  <<  "DBFilterCondition: getTransformedValue: unknown property type " << Property::asString(variable->dataType());
                throw std::runtime_error ("DBFilterCondition: getTransformedValue: unknown property type "+Property::asString(variable->dataType()));
            }
        }
        logdbg << "DBFilterCondition: getTransformedValue: transformed value string " << value_str;
        transformed_value_strings.push_back(value_str);
    }

    assert (transformed_value_strings.size());

    if (operator_ != "IN")
    {
        assert (transformed_value_strings.size() == 1);
        return transformed_value_strings.at(0);
    }
    else
        return "(" + boost::algorithm::join(transformed_value_strings, ",") + ")";
}

//void DBFilterCondition::notifyMinMax (DBOVariable *variable)
//{
//    loginf << "DBFilterCondition: notifyMinMax";

//    std::string value;

//    //TODO FIX MINMAX
//    assert (false);

//    assert (variable_->hasMinMaxInfo());
//    if (reset_value_.compare ("MIN") == 0)
//        value = variable_->getRepresentationFromValue(variable_->getMinString());
//    else if (reset_value_.compare ("MAX") == 0)
//        value = variable_->getRepresentationFromValue(variable_->getMaxString());

//    value_=value;
//    update();
//}
