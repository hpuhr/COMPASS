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

#include "dbfiltercondition.h"
#include "compass.h"
#include "dbfilter.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "stringconv.h"
#include "global.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <QApplication>
#include <QStyle>

#include <boost/algorithm/string/join.hpp>

#include <cassert>
#include <sstream>

using namespace Utils;
using namespace std;

DBFilterCondition::DBFilterCondition(const std::string& class_id, const std::string& instance_id,
                                     DBFilter* filter_parent)
    : Configurable(class_id, instance_id, filter_parent), filter_parent_(filter_parent)
{
    registerParameter("operator", &operator_, std::string(">"));
    registerParameter("op_and", &op_and_, true);
    registerParameter("absolute_value", &absolute_value_, false);

    registerParameter("variable_dbcontent_name", &variable_dbcontent_name_, std::string());
    registerParameter("variable_name", &variable_name_, std::string());

    registerParameter("display_instance_id", &display_instance_id_, false);

    // DBOVAR LOWERCASE HACK
    // boost::algorithm::to_lower(variable_name_);

    registerParameter("reset_value", &reset_value_, std::string(""));
    registerParameter("value", &value_, std::string());

//    if (usable_)
//        value_invalid_ = checkValueInvalid(value_);

    logdbg << "DBFilterCondition: ctor: " << instance_id << " value " << value_
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
 * Returns if variable_ exists in DBContent of type
 */
bool DBFilterCondition::filters(const std::string& dbcontent_name)
{
    assert(usable_);

    return hasVariable(dbcontent_name);
}

std::string DBFilterCondition::getConditionString(const std::string& dbcontent_name, bool& first)
{
    logdbg << "DBFilterCondition: getConditionString: object " << dbcontent_name << " first " << first;
    assert(usable_);

    std::stringstream ss;

    std::string variable_prefix;
    std::string variable_suffix;

    if (absolute_value_)
    {
        variable_prefix = "ABS(" + variable_prefix;
        variable_suffix = variable_suffix + ")";
    }

    assert (hasVariable(dbcontent_name));

    dbContent::Variable& var = variable(dbcontent_name);

    //const DBTableColumn& column = var.currentDBColumn();
    std::string db_column_name = var.dbColumnName();
    std::string db_table_name = var.dbTableName();

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

    tie(val_str, null_contained) = getTransformedValue(value_, &var);

    if (null_contained)
    {
        ss << "(";

        if (val_str.size())
        {
            ss << variable_prefix << db_table_name << "." << db_column_name << variable_suffix;
            ss << " " << operator_ << val_str << " OR ";
        }

        ss << variable_prefix << db_table_name << "." << db_column_name << variable_suffix;
        ss << " " << operator_ << " NULL";

        ss << ")";
    }
    else
    {
        ss << variable_prefix << db_table_name << "." << db_column_name << variable_suffix;
        ss << " " << operator_ << val_str;
    }

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
    {
        edit_->setStyleSheet(COMPASS::instance().lineEditInvalidStyle());
    }
    else
    {
        //edit_->setStyleSheet(QApplication::style()->objectName());
        edit_->setStyleSheet("");

        // edit_->setStyleSheet(
        //             "QLineEdit { background: rgb(255, 255, 255); selection-background-color:"
        //             " rgb(200, 200, 200); }");
    }
}

std::string DBFilterCondition::getVariableName() const
{
    return variable_name_;
}

void DBFilterCondition::setVariableName(const std::string& variable_name)
{
    loginf << "DBFilterCondition: setVariableName: name '" << variable_name << "'";

    if (variable_name != variable_name_)
    {
        variable_name_ = variable_name;

        reset();
    }
}

bool DBFilterCondition::hasVariable (const std::string& dbcontent_name)
{
    DBContentManager& dbo_man = COMPASS::instance().dbContentManager();

    if (variable_dbcontent_name_ == META_OBJECT_NAME)
    {
        if (!dbo_man.existsMetaVariable(variable_name_))
            return false;

        return dbo_man.metaVariable(variable_name_).existsIn(dbcontent_name);
    }
    else
    {
        if (dbcontent_name != variable_dbcontent_name_)
            return false;

        if (!dbo_man.existsDBContent(variable_dbcontent_name_))
            return false;

        return dbo_man.dbContent(variable_dbcontent_name_).hasVariable(variable_name_);
    }
}


dbContent::Variable& DBFilterCondition::variable (const std::string& dbcontent_name)
{
    assert (hasVariable(dbcontent_name));

    DBContentManager& dbo_man = COMPASS::instance().dbContentManager();

    if (variable_dbcontent_name_ == META_OBJECT_NAME)
        return dbo_man.metaVariable(variable_name_).getFor(dbcontent_name);
    else
         return dbo_man.dbContent(variable_dbcontent_name_).variable(variable_name_);
}


void DBFilterCondition::update()
{
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
    assert(usable_);

    std::vector<dbContent::Variable*> variables;

    if (new_value.size() == 0)
    {
        loginf << "DBFilterCondition: checkValueInvalid: no value, returning invalid";
        return true;
    }

    if (variable_dbcontent_name_ == META_OBJECT_NAME)
    {
         DBContentManager& dbo_man = COMPASS::instance().dbContentManager();

        assert (dbo_man.existsMetaVariable(variable_name_));

        for (auto var_it : dbo_man.metaVariable(variable_name_).variables())
            variables.push_back(&var_it.second);
    }
    else
    {
        assert (hasVariable(variable_dbcontent_name_));
        variables.push_back(&variable(variable_dbcontent_name_));
    }

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
                                                                    dbContent::Variable* variable)
{
    assert(variable);

    std::vector<std::string> value_strings;
    std::vector<std::string> transformed_value_strings;

    if (operator_ == "IN" || operator_ == "NOT IN")
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

        if (variable->representation() != dbContent::Variable::Representation::STANDARD)
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
//            logwrn << "DBFilterCondition: getTransformedValue: variable '" << var.name()
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
        if (operator_ != "IN" && operator_ != "NOT IN")
        {
            assert(transformed_value_strings.size() == 1);
            value_str = transformed_value_strings.at(0);
        }
        else
            value_str = "(" + boost::algorithm::join(transformed_value_strings, ",") + ")";
    }

    return {value_str, null_set};
}
