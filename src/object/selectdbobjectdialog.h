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

#ifndef SELECTDBOBJECTDIALOG_H
#define SELECTDBOBJECTDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>

#include "dbobjectcombobox.h"

class SelectDBObjectDialog : public QDialog
{
    Q_OBJECT

  public:
    SelectDBObjectDialog()
    {
        setMinimumWidth(300);

        QVBoxLayout* main_layout = new QVBoxLayout();

        name_edit_ = new QLineEdit("Name");
        main_layout->addWidget(name_edit_);

        object_box_ = new DBObjectComboBox(false);
        main_layout->addWidget(object_box_);

        QDialogButtonBox* button_box =
            new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
        connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));

        main_layout->addWidget(button_box);

        setLayout(main_layout);

        setWindowTitle(tr("Select Name and DBObject"));
    }

    std::string name()
    {
        assert(name_edit_);
        return name_edit_->text().toStdString();
    }

    std::string selectedObject()
    {
        assert(object_box_);
        return object_box_->currentText().toStdString();
    }

  protected:
    QLineEdit* name_edit_{nullptr};
    DBObjectComboBox* object_box_{nullptr};
};

#endif  // SELECTDBOBJECTDIALOG_H
