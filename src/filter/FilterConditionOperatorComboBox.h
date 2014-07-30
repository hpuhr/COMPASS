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
 * FilterConditionOperatorComboBox.h
 *
 *  Created on: Nov 28, 2012
 *      Author: sk
 */

#ifndef FILTERCONDITIONOPERATORCOMBOBOX_H_
#define FILTERCONDITIONOPERATORCOMBOBOX_H_

#include <QComboBox>
#include <QList>

class FilterConditionOperatorComboBox : public QComboBox
{
protected:
  static QList<QString> stringsList_;
public:
  FilterConditionOperatorComboBox ()
  {
    if (stringsList_.size() == 0)
    {
      stringsList_.append("=");
      stringsList_.append("!=");
      stringsList_.append(">");
      stringsList_.append(">=");
      stringsList_.append("<");
      stringsList_.append("<=");
      stringsList_.append("|=");
      stringsList_.append("IS");
      stringsList_.append("IS NOT");
    }

    /* Populate the comboBox */
    addItems(stringsList_);
  }
  virtual ~FilterConditionOperatorComboBox () {}

};

QList<QString> FilterConditionOperatorComboBox::stringsList_;

#endif /* FILTERCONDITIONOPERATORCOMBOBOX_H_ */
