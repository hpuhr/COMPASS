#include "dbschemamanager.h"
#include "dbschemamanagerwidget.h"

#include "dbtable.h"
#include "metadbtable.h"
#include "dbschema.h"
#include "dbschemawidget.h"
#include "dbschemamanager.h"
#include "logger.h"

#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QStackedWidget>

DBSchemaManagerWidget::DBSchemaManagerWidget(DBSchemaManager &manager, QWidget* parent, Qt::WindowFlags f)
 : QFrame(parent), manager_(manager), schema_select_(nullptr), add_button_(nullptr), delete_button_(nullptr),
   schema_widgets_(nullptr)
{
    unsigned int frame_width = 2;
    QFont font_bold;
    font_bold.setBold(true);

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(frame_width);

    QVBoxLayout *layout = new QVBoxLayout ();

    QLabel *db_schema_label = new QLabel (tr("Database Schema"));
    db_schema_label->setFont (font_bold);
    layout->addWidget (db_schema_label);

    schema_select_ = new QComboBox ();
    updateSchemas();
    connect (schema_select_, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(schemaSelectedSlot(const QString &)));
    schema_select_->setDisabled(true);
    layout->addWidget(schema_select_);

    QHBoxLayout *button_layout = new QHBoxLayout ();

    add_button_ = new QPushButton(tr("Add Schema"));
    add_button_->setDisabled (true);
    connect(add_button_, SIGNAL( clicked() ), this, SLOT( addSchemaSlot() ));
    button_layout->addWidget (add_button_);

    delete_button_ = new QPushButton(tr("Delete Schema"));
    delete_button_->setDisabled (true);
    connect(delete_button_, SIGNAL( clicked() ), this, SLOT( deleteSchemaSlot() ));
    button_layout->addWidget (delete_button_);

    layout->addLayout(button_layout);

    schema_widgets_ = new QStackedWidget ();
    layout->addWidget(schema_widgets_);

    updateSchemas();

    layout->addStretch();

    setLayout (layout);

}

DBSchemaManagerWidget::~DBSchemaManagerWidget()
{
}

void DBSchemaManagerWidget::addSchemaSlot()
{
    logdbg << "DBSchemaManagerWidget: addSchemaSlot";

    bool ok;
    QString text = QInputDialog::getText(this, tr("Schema Name"),
                                         tr("Specify a (unique) schema name:"), QLineEdit::Normal,
                                         "", &ok);

    if (ok && !text.isEmpty())
    {
        manager_.addEmptySchema(text.toStdString());
        updateSchemas();
    }
}

void DBSchemaManagerWidget::deleteSchemaSlot ()
{
    logdbg << "DBSchemaManagerWidget: deleteSchemaSlot";

    manager_.deleteCurrentSchema();

    updateSchemas ();
}

void DBSchemaManagerWidget::schemaSelectedSlot (const QString &value)
{
    logdbg << "DBSchemaManagerWidget: schemaSelectedSlot: '" << value.toStdString() << "'";

    assert (schema_widgets_);
    while (schema_widgets_->count() > 0)
        schema_widgets_->removeWidget(schema_widgets_->widget(0));

    if (value.size() > 0)
    {
        manager_.setCurrentSchema(value.toStdString());

        DBSchemaWidget *widget = manager_.getCurrentSchema().widget();
//        QObject::connect(widget, SIGNAL(serverConnectedSignal()), this, SLOT(serverConnectedSlot()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));
//        QObject::connect(widget, SIGNAL(databaseOpenedSignal()), this, SLOT(databaseOpenedSlot()), static_cast<Qt::ConnectionType>(Qt::UniqueConnection));

        schema_widgets_->addWidget(widget);
        delete_button_->setDisabled(false);
    }
    else
        delete_button_->setDisabled(true);
}

void DBSchemaManagerWidget::updateSchemas()
{
    assert (schema_select_);

    schema_select_->clear();

    for (auto it : manager_.getSchemas())
    {
        schema_select_->addItem (it.first.c_str());
    }

    if (manager_.hasCurrentSchema())
    {
        int index = schema_select_->findText(manager_.getCurrentSchemaName().c_str());
        if (index != -1) // -1 for not found
        {
           schema_select_->setCurrentIndex(index);
        }
    }

}


void DBSchemaManagerWidget::databaseOpenedSlot ()
{
    assert (schema_select_);
    schema_select_->setDisabled (false);

    assert (add_button_);
    add_button_->setDisabled (false);

    assert (delete_button_);
    delete_button_->setDisabled (false);
}

// and who are you? the proud lord said...
//void DBSchemaWidget::addRDLSchema ()
//{
//    assert (new_schema_name_edit_);
//    manager_.addRDLSchema(new_schema_name_edit_->text().toStdString());
//    updateSchemaCombo();
//}
