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

#ifndef DBOADDSCHEMAMETATABLEDIALOG_H
#define DBOADDSCHEMAMETATABLEDIALOG_H

#include "logger.h"
#include "dbschemamanager.h"
#include "atsdb.h"
#include "dbschema.h"
#include "dbschemaselectioncombobox.h"
#include "metadbtable.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QComboBox>
#include <QLabel>
#include <QDialog>

#include <cassert>

class DBObject;
class QComboBox;
class DBSchemaSelectionComboBox;
class QLabel;

class DBOAddSchemaMetaTableDialog : public QDialog
{
    Q_OBJECT

public:
    DBOAddSchemaMetaTableDialog ()
    {
        setMinimumWidth(300);

        QVBoxLayout* main_layout = new QVBoxLayout ();

        QGridLayout* grid_layout = new QGridLayout;

        QLabel *new_meta_schema_label = new QLabel ("Schema");
        grid_layout->addWidget (new_meta_schema_label, 0, 0);

        schema_box_ = new DBSchemaSelectionComboBox ();
        schema_box_->update ();
        grid_layout->addWidget (schema_box_, 0, 1);

        QLabel *new_meta_meta_label = new QLabel ("Meta table");
        grid_layout->addWidget (new_meta_meta_label, 1, 0);

        meta_table_box_ = new QComboBox ();
        updateMetaTableSelection ();
        grid_layout->addWidget (meta_table_box_, 1, 1);

        main_layout->addLayout (grid_layout);

        QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok |  QDialogButtonBox::Cancel);
        connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
        connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));

        main_layout->addWidget (button_box);

        setLayout(main_layout);

        setWindowTitle(tr("Add Meta Table for a Schema"));
    }

    std::string schemaName ()
    {
        assert (schema_box_);
        return schema_box_->currentText().toStdString();
    }

    std::string metaTableName ()
    {
        assert (meta_table_box_);
        return meta_table_box_->currentText().toStdString();
    }

protected:
    DBSchemaSelectionComboBox* schema_box_ {nullptr};
    QComboBox* meta_table_box_ {nullptr};

    void updateMetaTableSelection ()
    {
        logdbg  << "DBOAddSchemaMetaTableDialog: updateMetaTableSelection";
        assert (meta_table_box_);

        std::string selection;

        if (meta_table_box_->count() > 0)
            selection = meta_table_box_->currentText().toStdString();

        while (meta_table_box_->count() > 0)
            meta_table_box_->removeItem (0);

        auto metas = ATSDB::instance().schemaManager().getCurrentSchema().metaTables ();

        int index_cnt=-1;
        unsigned int cnt=0;
        for (auto it = metas.begin(); it != metas.end(); it++)
        {
            if (selection.size()>0 && selection.compare(it->second->name()) == 0)
                index_cnt=cnt;

            meta_table_box_->addItem (it->second->name().c_str());

            cnt++;
        }

        if (index_cnt != -1)
        {
            meta_table_box_->setCurrentIndex (index_cnt);
        }
    }
};

#endif // DBOADDSCHEMAMETATABLEDIALOG_H
