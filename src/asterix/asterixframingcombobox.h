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

#ifndef ASTERIXFRAMINGCOMBOBOX_H
#define ASTERIXFRAMINGCOMBOBOX_H

#include "asteriximportertask.h"

#include <jasterix/jasterix.h>

#include <QComboBox>

class ASTERIXFramingComboBox: public QComboBox
{
    Q_OBJECT

signals:
    /// @brief Emitted if type was changed
    void changedFraming();

public:
    /// @brief Constructor
    ASTERIXFramingComboBox(ASTERIXImporterTask& task, QWidget * parent = 0)
    : QComboBox(parent), task_(task)
    {
        loadFramings();
        connect(this, SIGNAL(activated(const QString &)), this, SIGNAL(changedFraming()));

    }
    /// @brief Destructor
    virtual ~ASTERIXFramingComboBox() {}

    void loadFramings ()
    {
        clear();

        for (std::string frame_it : task_.jASTERIX()->framings())
        {
            addItem (frame_it.c_str());
        }

        setCurrentIndex (0);
    }

    /// @brief Returns the currently selected framing
    std::string getFraming ()
    {
        return currentText().toStdString();
    }

    /// @brief Sets the currently selected data type
    void setFraming (const std::string &framing)
    {
        int index = findText(QString(framing.c_str()));
        assert (index >= 0);
        setCurrentIndex (index);
    }

protected:
    ASTERIXImporterTask& task_;
};

#endif // ASTERIXFRAMINGCOMBOBOX_H
