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

#include "compass.h"
#include "dbfilter.h"
#include "dbfiltercondition.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "metadbovariable.h"
#include "stringconv.h"
#include "unit.h"
#include "unitmanager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <boost/algorithm/string/join.hpp>
#include <cassert>
#include <sstream>
//#include <boost/algorithm/string.hpp>

using namespace Utils;
using namespace std;

/**
 * Initializes members, registers parameters, create GUI elements.
 */
DBFilterCondition::DBFilterCondition(const std::string& class_id, const std::string& instance_id,
                                     DBFilter* filter_parent)
    : Configurable(class_id, instance_id, filter_parent), filter_parent_(filter_parent)
{
    registerParameter("operator", &operator_, ">");
    registerParameter("op_and", &op_and_, true);
    registerParameter("absolute_value", &absolute_value_, false);
    registerParameter("variable_dbo_name", &variable_dbo_name_, "");
    registerParameter("variable_name", &variable_name_, "");
    //registerParameter("variable_name", &variable_name_, "");
    registerParameter("display_instance_id", &display_instance_id_, false);

    // DBOVAR LOWERCASE HACK
    // boost::algorithm::to_lower(variable_name_);

    if (variable_dbo_name_ == META_OBJECT_NAME)
    {
        if (!COMPASS::instance().objectManager().existsMetaVariable(variable_name_))
            throw std::runtime_error("DBFilterCondition: constructor: meta dbo variable '" +
                                     variable_name_ + "' does not exist");
        meta_variable_ = &COMPASS::instance().objectManager().metaVariable(variable_name_);
        assert(meta_variable_);
    }
    else
    {
        if (!COMPASS::instance().objectManager().existsObject(variable_dbo_name_) ||
                !COMPASS::instance()
                .objectManager()
                .object(variable_dbo_name_)
                .hasVariable(variable_name_))
            throw std::runtime_error("DBFilterCondition: constructor: dbo variable '" +
                                     variable_name_ + "' does not exist");

        variable_ =
                &COMPASS::instance().objectManager().object(variable_dbo_name_).variable(variable_name_);

        assert(variable_);
    }

    registerParameter("reset_value", &reset_value_, std::string(""));
    registerParameter("value", &value_, "");

    if (usable_)
        value_invalid_ = checkValueInvalid(value_);

    loginf << "DBFilterCondition: DBFilterCondition: " << instance_id << " value " << value_
           << " usable " << usable_ << " invalid " << value_invalid_;

    widget_ = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    label_ = new QLabel();
    if (display_instance_id_)
        label_->setText(tr((instanceId() + " " + operator_).c_str()));
    else
        label_->setText(tr((variable_name_ + " " + operator_).c_str()));
    layout->addWidget(label_);

    edit_ = new QLineEdit(tr(value_.c_str()));
    connect(edit_, SIGNAL(textChanged(QString)), this, SLOT(valueChanged()));
    layout->addWidget(edit_);

    widget_->setLayout(layout);

    if (!usable_)
        widget_->setDisabled(true);
}

DBFilterCondition::~DBFilterCondition() {}

void DBFilterCondition::invert()
{
    // TODO
    // op_and_=!op_and_;
}

/**
 * Returns if variable_ exists in DBObject of type
 */
bool DBFilterCondition::filters(const std::string& dbo_name)
{
    assert(usable_);

    if (meta_variable_)
        return meta_variable_->existsIn(dbo_name);
    else
        return variable_dbo_name_ == dbo_name;
}

std::string DBFilterCondition::getConditionString(const std::string& dbo_name, bool& first,
                                                  std::vector<DBOVariable*>& filtered_variables)
{
    logdbg << "DBFilterCondition: getConditionString: object " << dbo_name << " first " << first;
    assert(usable_);

    std::stringstream ss;

    std::string variable_prefix;
    std::string variable_suffix;

    if (absolute_value_)
    {
        variable_prefix = "ABS(" + variable_prefix;
        variable_suffix = variable_suffix + ")";
    }

    assert(variable_ || meta_variable_);

    DBOVariable* variable = nullptr;

    if (meta_variable_)
    {
        assert(meta_variable_->existsIn(dbo_name));

        variable = &meta_variable_->getFor(dbo_name);
    }
    else
        variable = variable_;

    //const DBTableColumn& column = variable->currentDBColumn();
    std::string db_column_name = variable->dbColumnName();
    std::string db_table_name = variable->dbTableName();

    if (!first)
    {
        if (op_and_)
            ss << " AND ";
        else
            ss << " OR ";
    }
    first = false;

    string val_str;
    bool null_contained;

    tie(val_str, null_contained) = getTransformedValue(value_, variable);

    if (null_contained)
    {
        ss << "(";

        if (val_str.size())
        {
            ss << variable_prefix << db_table_name << "." << db_column_name << variable_suffix;
            ss << " " << operator_ << val_str << " OR ";
        }

        ss << variable_prefix << db_table_name << "." << db_column_name << variable_suffix;
        ss << " IS NULL";

        ss << ")";
    }
    else
    {
        ss << variable_prefix << db_table_name << "." << db_column_name << variable_suffix;
        ss << " " << operator_ << val_str;
    }

    if (find(filtered_variables.begin(), filtered_variables.end(), variable) ==
            filtered_variables.end())
        filtered_variables.push_back(variable);

    if (ss.str().size())
        loginf << "DBFilterCondition " << instanceId() << ": getConditionString: '" << ss.str()
               << "'";

    return ss.str();
}

/**
 * Checks if value_ is different than edit_ value, if yes sets changed_ and emits
 * possibleFilterChange.
 */
void DBFilterCondition::valueChanged()
{
    logdbg << "DBFilterCondition: valueChanged";
    assert(usable_);
    assert(edit_);

    std::string new_value = edit_->text().toStdString();

    value_invalid_ = checkValueInvalid(new_value);

    if (!value_invalid_ && value_ != new_value)
    {
        value_ = new_value;
        changed_ = true;

        emit possibleFilterChange();
    }

    loginf << "DBFilterCondition: valueChanged: value_ '" << value_ << "' invalid "
           << value_invalid_;

    if (value_invalid_)
        edit_->setStyleSheet(
                    "QLineEdit { background: rgb(255, 100, 100); selection-background-color:"
                    " rgb(255, 200, 200); }");
    else
        edit_->setStyleSheet(
                    "QLineEdit { background: rgb(255, 255, 255); selection-background-color:"
                    " rgb(200, 200, 200); }");
}

/**
 * Sets the variable if required, sets the variable_name_ and calls reset.
 */
void DBFilterCondition::setVariable(DBOVariable* variable)
{
    if (variable != variable_)
    {
        variable_ = variable;
        variable_name_ = variable_->name();

        reset();
    }
}

void DBFilterCondition::update()
{
    assert(variable_ || meta_variable_);

    if (display_instance_id_)
        label_->setText(tr((instanceId() + " " + operator_).c_str()));
    else
        label_->setText(tr((variable_name_ + " " + operator_).c_str()));

    edit_->setText(tr(value_.c_str()));
}

void DBFilterCondition::setValue(std::string value)
{
    value_ = value;

    update();
}

void DBFilterCondition::reset()
{
    assert(usable_);

    std::string value;

//    if (reset_value_.compare("MIN") == 0 || reset_value_.compare("MAX") == 0)
//    {
//        if (reset_value_.compare("MIN") == 0)
//        {
//            if (variable_)
//            {
//                value = variable_->getMinStringRepresentation();
//                logdbg << "DBFilterCondition: reset: value " << value << " repr " << value;
//            }
//            else
//            {
//                assert(meta_variable_);
//                value = meta_variable_->getMinStringRepresentation();
//                logdbg << "DBFilterCondition: reset: value " << value << " repr " << value;
//            }
//        }
//        else if (reset_value_.compare("MAX") == 0)
//        {
//            if (variable_)
//            {
//                value = variable_->getMaxStringRepresentation();
//                logdbg << "DBFilterCondition: reset: value " << value << " repr " << value;
//            }
//            else
//            {
//                assert(meta_variable_);
//                value = meta_variable_->getMaxStringRepresentation();
//                logdbg << "DBFilterCondition: reset: value " << value << " repr " << value;
//            }
//        }
//    }
//    else
    value = reset_value_;

    value_ = value;
    value_invalid_ = checkValueInvalid(value_);

    loginf << "DBFilterCondition: reset: value '" << value_ << " invalid " << value_invalid_;

    update();
}

bool DBFilterCondition::getDisplayInstanceId() const
{
    return display_instance_id_;
}

bool DBFilterCondition::checkValueInvalid(const std::string& new_value)
{
    assert(variable_ || meta_variable_);
    assert(usable_);

    std::vector<DBOVariable*> variables;

    if (new_value.size() == 0)
    {
        loginf << "DBFilterCondition: checkValueInvalid: no value, returning invalid";
        return true;
    }

    if (meta_variable_)
    {
        for (auto var_it : meta_variable_->variables())
            variables.push_back(&var_it.second);
    }
    else
        variables.push_back(variable_);

    bool invalid = true;

    try
    {
        for (auto var_it : variables)
        {
            std::string transformed_value;
            bool null_contained;
            tie(transformed_value, null_contained) = getTransformedValue(new_value, var_it);
            logdbg << "DBFilterCondition: valueChanged: transformed value " << transformed_value
                   << " null " << null_contained;
        }
        invalid = false;
    }
    catch (std::exception& e)
    {
        logdbg << "DBFilterCondition: checkValueInvalid: exception thrown: " << e.what();
    }
    catch (...)
    {
        logdbg << "DBFilterCondition: checkValueInvalid: exception thrown";
    }
    return invalid;
}

std::pair<std::string, bool> DBFilterCondition::getTransformedValue(const std::string& untransformed_value,
                                                                    DBOVariable* variable)
{
    assert(variable);

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

    bool null_set = find(value_strings.begin(), value_strings.end(), "NULL") != value_strings.end();

    if (null_set) // remove null value
        value_strings.erase(find(value_strings.begin(), value_strings.end(), "NULL"));

    std::string value_str;

    for (auto value_it : value_strings)
    {
        value_str = value_it;

        if (variable->representation() != DBOVariable::Representation::STANDARD)
            value_str =
                    variable->getValueStringFromRepresentation(value_str);  // fix representation

        logdbg << "DBFilterCondition: getTransformedValue: value string " << value_str;

        logdbg << "DBFilterCondition: getTransformedValue: transformed value string " << value_str;

//        if (column.dataFormat() == "")
//            ;
//        else if (column.dataFormat() == "hexadecimal")
//            value_str = String::hexStringFromInt(std::stoi(value_str));
//        else if (column.dataFormat() == "octal")
//            value_str = String::octStringFromInt(std::stoi(value_str));
//        else
//            logwrn << "DBFilterCondition: getTransformedValue: variable '" << variable->name()
//                   << "' unknown format '" << column.dataFormat() << "'";

//        logdbg << "DBFilterCondition: getTransformedValue: data format transformed value string "
//               << value_str;

        if (variable->dataType() == PropertyDataType::STRING)
            transformed_value_strings.push_back("'" + value_str + "'");
        else
            transformed_value_strings.push_back(value_str);
    }

    if (transformed_value_strings.size()) // can be empty if only NULL
    {
        if (operator_ != "IN")
        {
            assert(transformed_value_strings.size() == 1);
            value_str = transformed_value_strings.at(0);
        }
        else
            value_str = "(" + boost::algorithm::join(transformed_value_strings, ",") + ")";
    }

    return {value_str, null_set};
}
