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

#include "propertyvalueedit.h"
#include "property_templates.h"

#include <QLineEdit>
#include <QHBoxLayout>
#include <QValidator>

/**
*/
PropertyValueEdit::PropertyValueEdit(PropertyDataType dtype,
                                     int decimals,
                                     QWidget* parent)
:   QWidget  (parent  )
,   decimals_(decimals)
{
    auto layout = new QHBoxLayout;
    setLayout(layout);

    layout->setMargin(0);
    layout->setContentsMargins(0, 0, 0, 0);

    edit_ = new QLineEdit;
    layout->addWidget(edit_);

    connect(edit_, &QLineEdit::textEdited, this, &PropertyValueEdit::onValueChanged);
    connect(edit_, &QLineEdit::editingFinished, this, &PropertyValueEdit::onValueEdited);

    setPropertyDataType(dtype);
}

/**
*/
void PropertyValueEdit::setPropertyDataType(PropertyDataType dtype)
{
    if (dtype_ == dtype)
        return;

    dtype_ = dtype;

    edit_->setText("");
    edit_->setPlaceholderText(QString::fromStdString(Property::asString(dtype_)));

    checkValidity();
}

/**
*/
void PropertyValueEdit::setValue(const std::string& string_value)
{
    edit_->setText(QString::fromStdString(string_value));

    checkValidity();
}

/**
*/
void PropertyValueEdit::setValue(double value)
{
    auto v_str = property_templates::double2String(dtype_, value, decimals_);
    setValue(v_str);
}

bool PropertyValueEdit::isValid() const
{
    return valid_;
}

/**
*/
PropertyDataType PropertyValueEdit::dataType() const
{
    return dtype_;
}

/**
*/
std::string PropertyValueEdit::valueAsString() const
{
    return edit_->text().toStdString();
}

/**
*/
boost::optional<double> PropertyValueEdit::valueAsDouble() const
{
    auto str = edit_->text().toStdString();
    return property_templates::string2Double(dtype_, str);
}

/**
*/
void PropertyValueEdit::checkValidity()
{
    std::string txt = edit_->text().toStdString();

    auto v = property_templates::string2Double(dtype_, txt);
    valid_ = v.has_value();

    bool show_error = (!txt.empty() && !valid_);

    edit_->setStyleSheet(show_error ? QString("QLineEdit {background-color: #FA8072;}") : QString());
}

/**
*/
void PropertyValueEdit::onValueChanged()
{
    checkValidity();

    emit valueChanged();
}

/**
*/
void PropertyValueEdit::onValueEdited()
{
    checkValidity();

    emit valueEdited();
}
