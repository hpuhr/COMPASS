#include "dbovariableeditdialog.h"
#include "dbovariable.h"
#include "dbovariabledatatypecombobox.h"
#include "stringrepresentationcombobox.h"
#include "unitselectionwidget.h"
#include "stringconv.h"
#include "dbobject.h"
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

using namespace std;

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

    invalid_bg_str_ = "QLineEdit { background: rgb(255, 100, 100); selection-background-color:"
                    " rgb(255, 200, 200); }";

    valid_bg_str_ = "QLineEdit { background: rgb(255, 255, 255); selection-background-color:"
                    " rgb(200, 200, 200); }";
}

bool DBOVariableEditDialog::variableEdited() const
{
    return variable_edited_;
}

DBOVariable &DBOVariableEditDialog::variable() const
{
    return variable_;
}

void DBOVariableEditDialog::nameChangedSlot(const QString& name)
{
    loginf << "DBOVariableEditDialog: nameChangedSlot: name '" << name.toStdString() << "'";

    assert (name_edit_);
    string new_name = name.toStdString();

    if (new_name == variable_.name())
        return;

    if (!new_name.size())
    {
        name_edit_->setStyleSheet(invalid_bg_str_.c_str());
        return;
    }

    if (variable_.object().hasVariable(new_name))
    {
        logwrn << "DBOVariableEditDialog: nameChangedSlot: name '" << new_name << "' already in use";

        name_edit_->setStyleSheet(invalid_bg_str_.c_str());
        name_edit_->setToolTip(("Variable name '"+new_name+"' already in use").c_str());
        return;
    }

    // ok, rename

    name_edit_->setStyleSheet(valid_bg_str_.c_str());
    name_edit_->setToolTip("");

    loginf << "DBOVariableEditDialog: nameChangedSlot: renaming '" << variable_.name() << "' to '" << new_name << "'";
    variable_.object().renameVariable(variable_.name(), new_name);

    variable_edited_ = true;
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
