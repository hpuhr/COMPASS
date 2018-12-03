#ifndef SELECTDBOBJECTDIALOG_H
#define SELECTDBOBJECTDIALOG_H

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QComboBox>
#include <QLabel>
#include <QDialog>

#include "dbobjectcombobox.h"

class SelectDBObjectDialog : public QDialog
{
    Q_OBJECT

public:
    SelectDBObjectDialog ()
    {
        setMinimumWidth(300);

        QVBoxLayout* main_layout = new QVBoxLayout ();

        object_box_ = new DBObjectComboBox (false);
        main_layout->addWidget (object_box_);

        QDialogButtonBox *button_box = new QDialogButtonBox(QDialogButtonBox::Ok |  QDialogButtonBox::Cancel);
        connect(button_box, SIGNAL(accepted()), this, SLOT(accept()));
        connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));

        main_layout->addWidget (button_box);

        setLayout(main_layout);

        setWindowTitle(tr("Select DBObject"));
    }

    std::string selectedObject ()
    {
        assert (object_box_);
        return object_box_->currentText().toStdString();
    }

protected:
    DBObjectComboBox* object_box_ {nullptr};

//    void updateMetaTableSelection ()
//    {
//        logdbg  << "DBOAddSchemaMetaTableDialog: updateMetaTableSelection";
//        assert (meta_table_box_);

//        std::string selection;

//        if (meta_table_box_->count() > 0)
//            selection = meta_table_box_->currentText().toStdString();

//        while (meta_table_box_->count() > 0)
//            meta_table_box_->removeItem (0);

//        auto metas = ATSDB::instance().schemaManager().getCurrentSchema().metaTables ();

//        int index_cnt=-1;
//        unsigned int cnt=0;
//        for (auto it = metas.begin(); it != metas.end(); it++)
//        {
//            if (selection.size()>0 && selection.compare(it->second->name()) == 0)
//                index_cnt=cnt;

//            meta_table_box_->addItem (it->second->name().c_str());

//            cnt++;
//        }

//        if (index_cnt != -1)
//        {
//            meta_table_box_->setCurrentIndex (index_cnt);
//        }
//    }
};

#endif // SELECTDBOBJECTDIALOG_H
