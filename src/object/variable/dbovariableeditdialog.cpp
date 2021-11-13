#include "dbovariableeditdialog.h"
#include "dbovariable.h"
#include "dbovariabledatatypecombobox.h"
#include "stringrepresentationcombobox.h"
#include "unitselectionwidget.h"
#include "stringconv.h"
#include "logger.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTextEdit>

DBOVariableEditDialog::DBOVariableEditDialog(DBOVariable& variable, QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), variable_(variable)
{
    setWindowFlags(Qt::Window | Qt::WindowTitleHint); //  | Qt::CustomizeWindowHint

    setWindowTitle("Import Sector");

    setModal(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QFormLayout* form_layout = new QFormLayout;
    form_layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    //    QLineEdit* name_edit_ {nullptr};

    name_edit_ = new QLineEdit(variable_.name().c_str());
    connect(name_edit_, &QLineEdit::textChanged, this, &DBOVariableEditDialog::nameChangedSlot);
    form_layout->addRow("Name", name_edit_);

    //    QLineEdit* short_name_edit_ {nullptr};
    short_name_edit_ = new QLineEdit(variable_.shortName().c_str());
    connect(short_name_edit_, &QLineEdit::textChanged, this, &DBOVariableEditDialog::shortNameChangedSlot);
    form_layout->addRow("Short Name", short_name_edit_);

    //    QTextEdit* description_edit_ {nullptr};
    description_edit_ = new QTextEdit();
    description_edit_->setWordWrapMode(QTextOption::WrapMode::WrapAnywhere);

    connect(description_edit_, &QTextEdit::textChanged, this,
            &DBOVariableEditDialog::commentChangedSlot);

    form_layout->addRow("Comment", description_edit_);

    //    DBOVariableDataTypeComboBox* type_combo_ {nullptr};
    type_combo_ = new DBOVariableDataTypeComboBox(variable_);
    form_layout->addRow("Data Type", type_combo_);

    //    UnitSelectionWidget* unit_sel_ {nullptr};
    unit_sel_ = new UnitSelectionWidget(variable_.dimension(), variable_.unit());
    form_layout->addRow("Unit", type_combo_);

    //    StringRepresentationComboBox* representation_box_ {nullptr};
    representation_box_ = new StringRepresentationComboBox(variable_);
    form_layout->addRow("Representation", representation_box_);

    //    QLineEdit* db_column_edit_ {nullptr};
    db_column_edit_ = new QLineEdit(variable_.dbColumnName().c_str());
    connect(db_column_edit_, &QLineEdit::textChanged, this, &DBOVariableEditDialog::dbColumnChangedSlot);
    form_layout->addRow("DBColumn", db_column_edit_);

    main_layout->addLayout(form_layout);

    done_button_ = new QPushButton("Done");
    connect(done_button_, &QPushButton::clicked, this, &DBOVariableEditDialog::doneSlot);
    main_layout->addWidget(done_button_);

    setLayout(main_layout);
}

void DBOVariableEditDialog::nameChangedSlot(const QString& name)
{

}

void DBOVariableEditDialog::shortNameChangedSlot(const QString& name)
{

}

void DBOVariableEditDialog::commentChangedSlot()
{

}

void DBOVariableEditDialog::dbColumnChangedSlot(const QString& name)
{

}

void DBOVariableEditDialog::doneSlot()
{
    accept();
}
