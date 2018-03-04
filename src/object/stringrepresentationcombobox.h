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

#ifndef STRINGREPRESENTATIONCOMBOBOX_H_
#define STRINGREPRESENTATIONCOMBOBOX_H_

#include <QComboBox>
#include <stdexcept>

#include "global.h"
#include "dbovariable.h"
#include "logger.h"

/**
 * @brief String representation selection for a DBOVariable
 */
class StringRepresentationComboBox : public QComboBox
{
    Q_OBJECT

signals:
    /// @brief Emitted if representation was changed
    void changedRepresentation();

public slots:
    /// @brief Sets the representation
    void changedSlot ()
    {
        loginf << "StringRepresentationComboBox: changed " << currentText().toStdString();
        variable_.representation(representation());
    }


public:
    /// @brief Constructor
    StringRepresentationComboBox(DBOVariable &variable, QWidget * parent = 0)
    : QComboBox(parent), variable_(variable)
    {
        for (auto it : DBOVariable::Representations())
            addItem (it.second.c_str());

        setCurrentText (DBOVariable::representationToString(variable_.representation()).c_str());
        //connect(this, SIGNAL( activated(const QString &) ), this, SIGNAL( changedRepresentation() ));
        connect(this, SIGNAL(currentTextChanged(const QString &)), this, SLOT(changedSlot()));
    }

    /// @brief Destructor
    virtual ~StringRepresentationComboBox() {}

    /// @brief Returns the currently selected representation
    DBOVariable::Representation representation ()
    {
        std::string text = currentText().toStdString();
        return DBOVariable::stringToRepresentation(text);
    }

    /// @brief Sets the currently selected representation
    void representation (DBOVariable::Representation type)
    {
        setCurrentText (DBOVariable::representationToString(type).c_str());
    }

protected:
    /// Used variable
    DBOVariable &variable_;
};


#endif /* STRINGREPRESENTATIONCOMBOBOX_H_ */
