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

#ifndef STRINGREPRESENTATIONCOMBOBOX_H_
#define STRINGREPRESENTATIONCOMBOBOX_H_

#include <QComboBox>
#include <stdexcept>

#include "dbovariable.h"
#include "global.h"
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
    void changedSlot()
    {
        loginf << "StringRepresentationComboBox: changed " << currentText().toStdString();
        variable_->representation(representation());
    }

  public:
    /// @brief Constructor
    StringRepresentationComboBox(DBOVariable& variable, QWidget* parent = 0)
        : QComboBox(parent), variable_(&variable)
    {
        for (auto it : DBOVariable::Representations())
            addItem(it.second.c_str());

        update();

        // connect(this, SIGNAL( activated(const QString &) ), this, SIGNAL( changedRepresentation()
        // ));
        connect(this, SIGNAL(currentTextChanged(const QString&)), this, SLOT(changedSlot()));
    }

    /// @brief Destructor
    virtual ~StringRepresentationComboBox() {}

    /// @brief Returns the currently selected representation
    DBOVariable::Representation representation()
    {
        std::string text = currentText().toStdString();
        return DBOVariable::stringToRepresentation(text);
    }

    /// @brief Sets the currently selected representation
    void representation(DBOVariable::Representation type)
    {
        setCurrentText(DBOVariable::representationToString(type).c_str());
    }

    void setVariable(DBOVariable& variable)
    {
        variable_ = &variable;

        update();
    }

  protected:
    /// Used variable
    DBOVariable* variable_{nullptr};

    void update()
    {
        setCurrentText(DBOVariable::representationToString(variable_->representation()).c_str());
    }
};

#endif /* STRINGREPRESENTATIONCOMBOBOX_H_ */
