#ifndef DBOADDDATASOURCEDIALOG_H
#define DBOADDDATASOURCEDIALOG_H

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

class DBOAddDataSourceDialog : public QDialog
{
    Q_OBJECT

public:
    DBOAddDataSourceDialog ()
    {
        setMinimumWidth(300);

        QVBoxLayout* main_layout = new QVBoxLayout ();

        QGridLayout* grid_layout = new QGridLayout;

        QLabel *new_meta_schema_label = new QLabel ("Schema");
        grid_layout->addWidget (new_meta_schema_label, 0, 0);

        schema_box_ = new DBSchemaSelectionComboBox ();
        schema_box_->update ();
        grid_layout->addWidget (schema_box_, 0, 1);

        main_layout->addLayout (grid_layout);

        QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok |  QDialogButtonBox::Cancel);
        connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
        connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));

        main_layout->addWidget (button_box);

        setLayout(main_layout);

        setWindowTitle(tr("Add Data Source for a Schema"));
    }

    std::string schemaName ()
    {
        assert (schema_box_);
        return schema_box_->currentText().toStdString();
    }

protected:
    DBSchemaSelectionComboBox* schema_box_ {nullptr};
};

#endif // DBOADDDATASOURCEDIALOG_H
