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
 * StringRepresentationComboBox.h
 *
 *  Created on: Aug 29, 2012
 *      Author: sk
 */

#ifndef STRINGREPRESENTATIONCOMBOBOX_H_
#define STRINGREPRESENTATIONCOMBOBOX_H_

#include <QComboBox>
#include <stdexcept>

#include "global.h"
#include "dbovariable.h"
#include "stringrepresentation.h"

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
    void changed ()
    {
        variable_.representation(representation());
    }


public:
    /// @brief Constructor
    StringRepresentationComboBox(DBOVariable &variable, QWidget * parent = 0)
    : QComboBox(parent), variable_(variable)
    {
        for (auto it : string_2_representation)
            addItem (it.first.c_str());

        setCurrentText (representation_2_string.at(variable_.representation()).c_str());
        connect(this, SIGNAL( activated(const QString &) ), this, SIGNAL( changedRepresentation() ));
        connect(this, SIGNAL( activated(const QString &) ), this, SLOT( changed() ));
    }

    /// @brief Destructor
    virtual ~StringRepresentationComboBox() {}

    /// @brief Returns the currently selected representation
    StringRepresentation representation ()
    {
        std::string text = currentText().toStdString();
        assert (string_2_representation.count(text) == 1);
        return string_2_representation.at(text);
    }

    /// @brief Sets the currently selected representation
    void representation (StringRepresentation type)
    {
        setCurrentText (representation_2_string.at(type).c_str());
    }

protected:
    /// Used variable
    DBOVariable &variable_;
};


#endif /* STRINGREPRESENTATIONCOMBOBOX_H_ */
