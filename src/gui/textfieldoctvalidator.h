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

#ifndef TEXTFIELDOCTVALIDATOR_H
#define TEXTFIELDOCTVALIDATOR_H

#include <QValidator>
#include <QLineEdit>

//#include "logger.h"

class TextFieldOctValidator : public QValidator
{
public:
    TextFieldOctValidator(unsigned int max_length, QObject* parent = nullptr)
        : QValidator(parent), max_length_(max_length)
    {
        //setNotation(QDoubleValidator::StandardNotation);
    }

    QValidator::State validate(QString &input, int &pos) const
    {
        if (!input.size())
            return QValidator::Acceptable;

        if (input.size() > max_length_ )
            return QValidator::Invalid;

        // match against needed regexp
        QRegExp rx("[0-7]+");
        if (rx.exactMatch(input)) {
            return QValidator::Acceptable;
        }
        return QValidator::Invalid;
    }

protected:
    unsigned int max_length_;
};


#endif // TEXTFIELDOCTVALIDATOR_H
