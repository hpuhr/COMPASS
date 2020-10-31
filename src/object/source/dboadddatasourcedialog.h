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

#ifndef DBOADDDATASOURCEDIALOG_H
#define DBOADDDATASOURCEDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <cassert>

#include "compass.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbschemaselectioncombobox.h"
#include "logger.h"
#include "metadbtable.h"

class DBObject;
class QComboBox;
class DBSchemaSelectionComboBox;
class QLabel;

class DBOAddDataSourceDialog : public QDialog
{
    Q_OBJECT

  public:
    DBOAddDataSourceDialog()
    {
        setMinimumWidth(300);

        QVBoxLayout* main_layout = new QVBoxLayout();

        QGridLayout* grid_layout = new QGridLayout;

        QLabel* new_meta_schema_label = new QLabel("Schema");
        grid_layout->addWidget(new_meta_schema_label, 0, 0);

        schema_box_ = new DBSchemaSelectionComboBox();
        schema_box_->update();
        grid_layout->addWidget(schema_box_, 0, 1);

        main_layout->addLayout(grid_layout);

        QDialogButtonBox* button_box =
            new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
        connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));

        main_layout->addWidget(button_box);

        setLayout(main_layout);

        setWindowTitle(tr("Add Data Source for a Schema"));
    }

    std::string schemaName()
    {
        assert(schema_box_);
        return schema_box_->currentText().toStdString();
    }

  protected:
    DBSchemaSelectionComboBox* schema_box_{nullptr};
};

#endif  // DBOADDDATASOURCEDIALOG_H
