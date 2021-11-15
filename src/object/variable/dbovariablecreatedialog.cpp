#include "dbovariablecreatedialog.h"
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

DBOVariableCreateDialog::DBOVariableCreateDialog(DBObject& object, const std::string name,
                                                 const std::string description,
                                                 QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), object_(object)
{
    setWindowFlags(Qt::Window | Qt::WindowTitleHint); //  | Qt::CustomizeWindowHint

    setWindowTitle("Create DBOVariable");

    setModal(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QFormLayout* form_layout = new QFormLayout;
    form_layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    //    QLineEdit* name_edit_ {nullptr};

    name_ = name;

    name_edit_ = new QLineEdit(name_.c_str());
    connect(name_edit_, &QLineEdit::textChanged, this, &DBOVariableCreateDialog::nameChangedSlot);
    form_layout->addRow("Name", name_edit_);

    //    QLineEdit* short_name_edit_ {nullptr};
    short_name_edit_ = new QLineEdit();
    connect(short_name_edit_, &QLineEdit::textChanged, this, &DBOVariableCreateDialog::shortNameChangedSlot);
    form_layout->addRow("Short Name", short_name_edit_);

    //    QTextEdit* description_edit_ {nullptr};

    description_ = description;

    description_edit_ = new QTextEdit();
    description_edit_->document()->setPlainText(description_.c_str());
    //description_edit_->setWordWrapMode(QTextOption::WrapMode::WrapAnywhere);

    connect(description_edit_, &QTextEdit::textChanged, this,
            &DBOVariableCreateDialog::commentChangedSlot);

    form_layout->addRow("Comment", description_edit_);

    //    DBOVariableDataTypeComboBox* type_combo_ {nullptr};
    type_combo_ = new DBOVariableDataTypeComboBox(data_type_);
    form_layout->addRow("Data Type", type_combo_);

    //    UnitSelectionWidget* unit_sel_ {nullptr};
    unit_sel_ = new UnitSelectionWidget(dimension_, unit_);
    form_layout->addRow("Unit", type_combo_);

    //    StringRepresentationComboBox* representation_box_ {nullptr};
    representation_box_ = new StringRepresentationComboBox(representation_, representation_str_);
    form_layout->addRow("Representation", representation_box_);

    //    QLineEdit* db_column_edit_ {nullptr};
    db_column_edit_ = new QLineEdit();
    connect(db_column_edit_, &QLineEdit::textChanged, this, &DBOVariableCreateDialog::dbColumnChangedSlot);
    form_layout->addRow("DBColumn", db_column_edit_);

    main_layout->addLayout(form_layout);

    main_layout->addStretch();

    // buttons

    QHBoxLayout* button_layout = new QHBoxLayout();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &DBOVariableCreateDialog::reject);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    ok_button_ = new QPushButton("OK");
    connect(ok_button_, &QPushButton::clicked, this, &DBOVariableCreateDialog::accept);
    button_layout->addWidget(ok_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    invalid_bg_str_ = "QLineEdit { background: rgb(255, 100, 100); selection-background-color:"
                    " rgb(255, 200, 200); }";

    valid_bg_str_ = "QLineEdit { background: rgb(255, 255, 255); selection-background-color:"
                    " rgb(200, 200, 200); }";
}

void DBOVariableCreateDialog::nameChangedSlot(const QString& name)
{
    loginf << "DBOVariableCreateDialog: nameChangedSlot: name '" << name.toStdString() << "'";

    assert (name_edit_);
    string new_name = name.toStdString();

    if (new_name == name_)
        return;

    if (!new_name.size())
    {
        name_edit_->setStyleSheet(invalid_bg_str_.c_str());
        return;
    }

    if (object_.hasVariable(new_name))
    {
        logwrn << "DBOVariableCreateDialog: nameChangedSlot: name '" << new_name << "' already in use";

        name_edit_->setStyleSheet(invalid_bg_str_.c_str());
        name_edit_->setToolTip(("Variable name '"+new_name+"' already in use").c_str());
        return;
    }

    // ok, rename
    name_ = new_name;

    name_edit_->setStyleSheet(valid_bg_str_.c_str());
    name_edit_->setToolTip("");
}

void DBOVariableCreateDialog::shortNameChangedSlot(const QString& name)
{
    short_name_ = name.toStdString();
}

void DBOVariableCreateDialog::commentChangedSlot()
{
    assert (description_edit_);

    description_ = description_edit_->document()->toPlainText().toStdString();
}

void DBOVariableCreateDialog::dbColumnChangedSlot(const QString& name)
{
    assert (db_column_edit_);
    string new_name = name.toStdString();

    if (new_name == name_)
        return;

    if (!new_name.size())
    {
        db_column_edit_->setStyleSheet(invalid_bg_str_.c_str());
        return;
    }

    if (object_.hasVariableDBColumnName(new_name))
    {
        logwrn << "DBOVariableCreateDialog: dbColumnChangedSlot: name '" << new_name << "' already in use";

        db_column_edit_->setStyleSheet(invalid_bg_str_.c_str());
        db_column_edit_->setToolTip(("Variable DB Column name '"+new_name+"' already in use").c_str());
        return;
    }

    // ok, rename
    db_column_name_ = new_name;

    db_column_edit_->setStyleSheet(valid_bg_str_.c_str());
    db_column_edit_->setToolTip("");
}

