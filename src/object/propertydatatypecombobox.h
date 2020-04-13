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

#ifndef PROPERTYDATATYPECOMBOBOX_H_
#define PROPERTYDATATYPECOMBOBOX_H_

#include <QComboBox>
#include <stdexcept>

#include "DBOVariable.h"
#include "Global.h"
#include "Logger.h"

/**
 *  @brief Property data type selection for a DBOVariable
 */
class PropertyDataTypeComboBox : public QComboBox
{
    Q_OBJECT

  signals:
    /// @brief Emitted if type was changed
    void changedType();

  public slots:
    /// @brief Sets the data type
    void changed()
    {
        assert(false);
        // TODO
        //        std::string text = currentText().toStdString();
        //        for (unsigned int cnt = 0; cnt < P_TYPE_SENTINEL; cnt++)
        //        {
        //            if (text.compare (PROPERTY_DATA_TYPE_STRINGS.at((PROPERTY_DATA_TYPE) cnt)) ==
        //            0)
        //            {
        //                data_type_ = (PROPERTY_DATA_TYPE) cnt;
        //                //loginf << "uga " << text << " " << PROPERTY_DATA_TYPE_STRINGS
        //                [(PROPERTY_DATA_TYPE) cnt]; return;
        //            }
        //        }
        //        throw std::runtime_error ("PropertyDataTypeComboBox: getType: unknown type");
    }

  public:
    /// @brief Constructor
    PropertyDataTypeComboBox(QWidget* parent = 0) : QComboBox(parent)
    {
        assert(false);
        // TODO
        //        for (unsigned int cnt = 0; cnt < P_TYPE_SENTINEL; cnt++)
        //        {
        //            addItem (PROPERTY_DATA_TYPE_STRINGS.at((PROPERTY_DATA_TYPE) cnt).c_str(),
        //            cnt);
        //        }
        //        connect(this, SIGNAL( activated(const QString &) ), this, SIGNAL( changedType()
        //        )); connect(this, SIGNAL( activated(const QString &) ), this, SLOT( changed() ));

        //        setType (P_TYPE_BOOL);
    }
    /// @brief Destructor
    virtual ~PropertyDataTypeComboBox() {}
    /// @brief Returns the currently selected data type
    PropertyDataType getType() { return data_type_; }
    /// @brief Sets the currently selected data type
    void setType(PropertyDataType type)
    {
        data_type_ = type;

        assert(false);
        // TODO
        //        int index = findData((unsigned int) type);
        //        assert (index != -1);
        //        setCurrentIndex (index);
    }

  protected:
    PropertyDataType data_type_;
};

#endif /* PROPERTYDATATYPECOMBOBOX_H_ */
