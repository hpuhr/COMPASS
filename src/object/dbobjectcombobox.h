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

#ifndef DBOOBJECTCOMBOBOX_H
#define DBOOBJECTCOMBOBOX_H

#include <QComboBox>

#include "compass.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "global.h"

/**
 *  @brief Property data type selection for a DBOVariable
 */
class DBObjectComboBox : public QComboBox
{
    Q_OBJECT

  signals:
    /// @brief Emitted if type was changed
    void changedObject();

  public:
    /// @brief Constructor
    DBObjectComboBox(bool allow_meta, QWidget* parent = 0)
        : QComboBox(parent), allow_meta_(allow_meta)
    {
        assert(COMPASS::instance().objectManager().size());
        if (allow_meta_)
            addItem(META_OBJECT_NAME.c_str());

        for (auto& obj_it : COMPASS::instance().objectManager())
        {
            addItem(obj_it.first.c_str());
        }

        setCurrentIndex(0);
        connect(this, SIGNAL(activated(const QString&)), this, SIGNAL(changedObject()));
    }
    /// @brief Destructor
    virtual ~DBObjectComboBox() {}

    /// @brief Returns the currently selected data type
    std::string getObjectName() { return currentText().toStdString(); }

    /// @brief Sets the currently selected data type
    void setObjectName(const std::string& object_name)
    {
        int index = findText(QString(object_name.c_str()));
        assert(index >= 0);
        setCurrentIndex(index);
    }

  protected:
    bool allow_meta_ {false};
};

#endif  // DBOOBJECTCOMBOBOX_H
