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

#include "Global.h"
#include "DBOVariable.h"

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
        variable_->setStringRepresentation(getRepresentation());
    }


public:
    /// @brief Constructor
    StringRepresentationComboBox(DBOVariable *variable, QWidget * parent = 0)
    : QComboBox(parent), variable_(variable)
    {
        for (unsigned int cnt = 0; cnt <= R_HEX; cnt++)
        {
            addItem (STRING_REPRESENTATION_STRINGS.at((STRING_REPRESENTATION) cnt).c_str());
        }
        setCurrentIndex (variable_->getRepresentation());
        connect(this, SIGNAL( activated(const QString &) ), this, SIGNAL( changedRepresentation() ));
        connect(this, SIGNAL( activated(const QString &) ), this, SLOT( changed() ));
    }

    /// @brief Destructor
    virtual ~StringRepresentationComboBox() {}
    /// @brief Returns the currently selected representation
    STRING_REPRESENTATION getRepresentation ()
    {
        std::string text = currentText().toStdString();
        for (unsigned int cnt = 0; cnt <= R_HEX; cnt++)
        {
            if (text.compare (STRING_REPRESENTATION_STRINGS.at((STRING_REPRESENTATION) cnt)) == 0)
                return (STRING_REPRESENTATION) cnt;
        }
        throw std::runtime_error ("StringRepresentationComboBox: getType: unknown type");
    }
    /// @brief Sets the currently selected representation
    void setRepresentation (STRING_REPRESENTATION type)
    {
        setCurrentIndex (type);
    }

protected:
    /// Used variable
    DBOVariable *variable_;
};


#endif /* STRINGREPRESENTATIONCOMBOBOX_H_ */
