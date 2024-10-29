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

#include "property.h"

#include <QWidget>

#include <boost/optional.hpp>

class QLineEdit;
class QValidator;

/**
 */
class PropertyValueEdit : public QWidget
{
    Q_OBJECT
public:
    PropertyValueEdit(PropertyDataType dtype = PropertyDataType::DOUBLE,
                      int decimals = 6,
                      QWidget* parent = nullptr);
    virtual ~PropertyValueEdit() = default;

    void setPropertyDataType(PropertyDataType dtype);
    void setValue(const std::string& string_value);
    void setValue(double value);

    bool isValid() const;

    PropertyDataType dataType() const;
    std::string valueAsString() const;
    boost::optional<double> valueAsDouble() const;

signals:
    void valueChanged();
    void valueEdited();

private:
    void checkValidity();
    void onValueChanged();
    void onValueEdited();

    QLineEdit* edit_ = nullptr;

    int              decimals_;
    PropertyDataType dtype_;
    bool             valid_ = false;
};