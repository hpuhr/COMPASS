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
DBFilterCondition::DBFilterCondition(const std::string& class_id, const std::string& instance_id,
                                     DBFilter* filter_parent)
: Configurable (class_id, instance_id, filter_parent), filter_parent_(filter_parent)
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
            throw std::runtime_error ("DBFilterCondition: constructor: meta dbo variable '" + variable_name_
                                      + "' does not exist");
        meta_variable_ = &ATSDB::instance().objectManager().metaVariable(variable_name_);
        assert (meta_variable_);

        if (!meta_variable_->existsInDB())
            usable_ = false;
    }
    else
    {
        if (!ATSDB::instance().objectManager().existsObject(variable_dbo_name_)
                || !ATSDB::instance().objectManager().object(variable_dbo_name_).hasVariable(variable_name_))
            throw std::runtime_error ("DBFilterCondition: constructor: dbo variable '" + variable_name_
                                      + "' does not exist");

        variable_ = &ATSDB::instance().objectManager().object(variable_dbo_name_).variable(variable_name_);

        assert (variable_);

        if (!variable_->existsInDB())
            usable_ = false;
    }

    registerParameter ("reset_value", &reset_value_, std::string("value"));
    registerParameter ("value", &value_, "");

    if (usable_)
        value_invalid_ = checkValueInvalid (value_);

    loginf << "DBFilterCondition: DBFilterCondition: " << instance_id << " value " << value_
           << " usable " << usable_ << " invalid " << value_invalid_;

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

    if (!usable_)
        widget_->setDisabled(true);
}

DBFilterCondition::~DBFilterCondition()
{
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
    assert (usable_);

    if (meta_variable_)
    {
        if (meta_variable_->existsIn(dbo_name))
            return meta_variable_->getFor(dbo_name).existsInDB();
        else
            return false;
    }
    else
        return variable_dbo_name_ == dbo_name;
}

std::string DBFilterCondition::getConditionString (const std::string &dbo_name, bool &first,
                                                   std::vector <DBOVariable*>& filtered_variables)
{
    logdbg << "DBFilterCondition: getConditionString: object " << dbo_name << " first " << first;
    assert (usable_);

    std::stringstream ss;

    std::string variable_prefix;
    std::string variable_suffix;

    if (absolute_value_)
    {
        variable_prefix="ABS("+variable_prefix;
        variable_suffix=variable_suffix+")";
    }

    assert (variable_ || meta_variable_);

    DBOVariable* variable=nullptr;

    if (meta_variable_)
    {
        assert (meta_variable_->existsIn(dbo_name));

        if (!meta_variable_->existsInDB())
        {
            logwrn << "DBFilterCondition: getConditionString: object " << dbo_name << " meta variable "
                   << meta_variable_->name() << " does not exist in db";
            return "";
        }

        variable = &meta_variable_->getFor(dbo_name);
    }
    else
        variable = variable_;

    if (!variable->existsInDB())
    {
        logwrn << "DBFilterCondition: getConditionString: object " << dbo_name << " variable "
               << variable->name() << " does not exist in db";
        return "";
    }

    const DBTableColumn& column = variable->currentDBColumn();
    const MetaDBTable& meta_table = variable->currentMetaTable();
    std::string table_db_name = meta_table.tableFor(column.identifier()).name();

    if (!first)
    {
        if (op_and_)
            ss << " AND ";
        else
            ss << " OR ";
    }
    first=false;

    ss << variable_prefix << table_db_name << "." << column.name() << variable_suffix << " " << operator_  << " "
       << getTransformedValue (value_, variable);

    if (find (filtered_variables.begin(), filtered_variables.end(), variable) == filtered_variables.end())
        filtered_variables.push_back(variable);


    if (ss.str().size() > 0)
        logdbg  << "DBFilterCondition " << instanceId()<<": getConditionString: '" << ss.str()<<"'";

    return ss.str();
}

/**
 * Checks if value_ is different than edit_ value, if yes sets changed_ and emits possibleFilterChange.
 */
void DBFilterCondition::valueChanged ()
{
    logdbg  << "DBFilterCondition: valueChanged";
    assert (usable_);
    assert  (edit_);

    std::string new_value = edit_->text().toStdString();

    value_invalid_ = checkValueInvalid (new_value);

    if (!value_invalid_ && value_ != new_value)
    {
        value_ = new_value;
        changed_=true;

        emit possibleFilterChange();
    }

    loginf  << "DBFilterCondition: valueChanged: value_ '" << value_ << "' invalid " << value_invalid_;

    if (value_invalid_)
        edit_->setStyleSheet("QLineEdit { background: rgb(255, 100, 100); selection-background-color:"
                             " rgb(255, 200, 200); }");
    else
        edit_->setStyleSheet("QLineEdit { background: rgb(255, 255, 255); selection-background-color:"
                             " rgb(200, 200, 200); }");
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
    assert (variable_ || meta_variable_);

    label_->setText(tr((variable_name_+" "+operator_).c_str()));
    edit_->setText (tr(value_.c_str()));
}

void DBFilterCondition::reset ()
{
    assert (usable_);

    std::string value;

    if (reset_value_.compare ("MIN") == 0 || reset_value_.compare ("MAX") == 0)
    {

        if (reset_value_.compare ("MIN") == 0)
        {
            if (variable_)
            {
                value = variable_->getMinStringRepresentation();
                logdbg << "DBFilterCondition: reset: value " << value << " repr " << value;
            }
            else
            {
                assert (meta_variable_);
                value = meta_variable_->getMinStringRepresentation();
                logdbg << "DBFilterCondition: reset: value " << value << " repr " << value;
            }
        }
        else if (reset_value_.compare ("MAX") == 0)
        {
            if (variable_)
            {
                value = variable_->getMaxStringRepresentation();
                logdbg << "DBFilterCondition: reset: value " << value << " repr " << value;
            }
            else
            {
                assert (meta_variable_);
                value = meta_variable_->getMaxStringRepresentation();
                logdbg << "DBFilterCondition: reset: value " << value << " repr " << value;
            }
        }
    }
    else
        value=reset_value_;

    value_=value;
    value_invalid_ = checkValueInvalid(value_);

    loginf  << "DBFilterCondition: reset: value '" << value_ << " invalid " << value_invalid_;

    update();
}

bool DBFilterCondition::checkValueInvalid (const std::string& new_value)
{
    assert (variable_ || meta_variable_);
    assert (usable_);

    std::vector <DBOVariable*> variables;

    if (new_value.size() == 0)
    {
        loginf << "DBFilterCondition: checkValueInvalid: no value, returning invalid";
        return true;
    }

    if (meta_variable_)
    {
        for (auto var_it : meta_variable_->variables())
        {
            if (var_it.second.existsInDB())
                variables.push_back(&var_it.second);
        }
    }
    else
        variables.push_back(variable_);

    bool invalid = true;

    try
    {
        for (auto var_it : variables)
        {
            std::string transformed_value = getTransformedValue (new_value, var_it);
            logdbg  << "DBFilterCondition: valueChanged: transformed value " << transformed_value;
        }
        invalid = false;
    }
    catch(std::exception& e)
    {
        logdbg  << "DBFilterCondition: checkValueInvalid: exception thrown: " << e.what();
    }
    catch(...)
    {
        logdbg  << "DBFilterCondition: checkValueInvalid: exception thrown";
    }
    return invalid;
}

std::string DBFilterCondition::getTransformedValue (const std::string& untransformed_value, DBOVariable* variable)
{
    assert (variable);
    const DBTableColumn &column = variable->currentDBColumn();
    assert (column.existsInDB());

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

    logdbg << "DBFilterCondition: getTransformedValue: in value strings '"
           << boost::algorithm::join(value_strings, ",") << "'";

    for (auto value_it : value_strings)
    {
        std::string value_str = value_it;

        if (variable->representation() != DBOVariable::Representation::STANDARD)
            value_str = variable->getValueStringFromRepresentation(value_str); // fix representation

        logdbg << "DBFilterCondition: getTransformedValue: value string " << value_str;

        if (column.unit() != variable->unit()) // do unit conversion stuff
        {
            logdbg << "DBFilterCondition: getTransformedValue: variable " << variable->name()
                   << " of same dimension has different units " << column.unit() << " " << variable->unit();

            if (!UnitManager::instance().hasDimension (variable->dimension()))
            {
                logerr  <<  "DBFilterCondition: getTransformedValue: unknown dimension '"
                         << variable->dimension() << "'";
                throw std::runtime_error ("DBFilterCondition: getTransformedValue: unknown dimension '"
                                          +variable->dimension()+"'");
            }

            const Dimension &dimension = UnitManager::instance().dimension (variable->dimension());

            if (!dimension.hasUnit(column.unit()))
            {
                logerr << "DBFilterCondition: getTransformedValue: variable " << variable->name()
                       << " has unknown column unit '" << column.unit() << "' in dimension " << variable->dimension();
                //  no transformation possible
            }
            else if (!dimension.hasUnit(variable->unit()))
            {
                logerr << "DBFilterCondition: getTransformedValue: variable " << variable->name()
                       << " has unknown variable unit '" << variable->unit() << "' in dimension "
                       << variable->dimension();
                //  no transformation possible
            }
            else
            {
                double factor = dimension.getFactor (column.unit(), variable->unit());
                logdbg  << "DBFilterCondition: getTransformedValue: correct unit transformation with factor " << factor;

                value_str = variable->multiplyString(value_str, 1.0/factor);
            }
        }

        logdbg << "DBFilterCondition: getTransformedValue: transformed value string " << value_str;

        if (column.dataFormat() == "")
            ;
        else if (column.dataFormat() == "hexadecimal")
            value_str = String::hexStringFromInt(std::stoi(value_str));
        else if (column.dataFormat() == "octal")
            value_str = String::octStringFromInt(std::stoi(value_str));
        else
            logwrn << "DBFilterCondition: getTransformedValue: variable '" << variable->name() << "' unknown format '"
                   << column.dataFormat() << "'";

        logdbg << "DBFilterCondition: getTransformedValue: data format transformed value string " << value_str;
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

