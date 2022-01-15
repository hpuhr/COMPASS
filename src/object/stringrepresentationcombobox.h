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

        representation_ = representation();
        representation_str_ = DBContentVariable::representationToString(representation_);
    }

  public:
    /// @brief Constructor
    StringRepresentationComboBox(DBContentVariable::Representation& representation, std::string& representation_str,
                                 QWidget* parent = 0)
        : QComboBox(parent), representation_(representation), representation_str_(representation_str)
    {
        for (auto it : DBContentVariable::Representations())
            addItem(it.second.c_str());

        update();

        // connect(this, SIGNAL( activated(const QString &) ), this, SIGNAL( changedRepresentation()
        // ));
        connect(this, SIGNAL(currentTextChanged(const QString&)), this, SLOT(changedSlot()));
    }

    /// @brief Destructor
    virtual ~StringRepresentationComboBox() {}

    /// @brief Returns the currently selected representation
    DBContentVariable::Representation representation()
    {
        std::string text = currentText().toStdString();
        return DBContentVariable::stringToRepresentation(text);
    }

    /// @brief Sets the currently selected representation
//    void representation(DBOVariable::Representation representation)
//    {
//        setCurrentText(DBOVariable::representationToString(representation).c_str());
//    }

    void setRepresentation(DBContentVariable::Representation& representation,
                           std::string& representation_str)
    {
        representation_ = representation;
        representation_str_ = representation_str;

        update();
    }

  protected:
    /// Used variable
    DBContentVariable::Representation& representation_;
    std::string& representation_str_;

    void update()
    {
        setCurrentText(DBContentVariable::representationToString(representation_).c_str());
    }
};

#endif /* STRINGREPRESENTATIONCOMBOBOX_H_ */
