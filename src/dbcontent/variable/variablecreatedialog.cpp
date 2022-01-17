#include "dbcontent/variable/variablecreatedialog.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variabledatatypecombobox.h"
#include "stringrepresentationcombobox.h"
#include "unitselectionwidget.h"
#include "stringconv.h"
#include "dbcontent/dbcontent.h"
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

#include <boost/algorithm/string.hpp>


using namespace std;

namespace dbContent
{

VariableCreateDialog::VariableCreateDialog(DBContent& object, const std::string name,
                                                 const std::string description,
                                                 QWidget* parent, Qt::WindowFlags f)
    : QDialog(parent, f), object_(object)
{
    setWindowFlags(Qt::Window | Qt::WindowTitleHint); //  | Qt::CustomizeWindowHint

    setWindowTitle("Create DBContent Variable");

    setModal(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QFormLayout* form_layout = new QFormLayout;
    form_layout->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);

    name_ = name;

    name_edit_ = new QLineEdit(name_.c_str());
    connect(name_edit_, &QLineEdit::textChanged, this, &VariableCreateDialog::nameChangedSlot);
    form_layout->addRow("Name", name_edit_);

    short_name_edit_ = new QLineEdit();
    connect(short_name_edit_, &QLineEdit::textChanged, this, &VariableCreateDialog::shortNameChangedSlot);
    form_layout->addRow("Short Name", short_name_edit_);

    description_ = description;

    description_edit_ = new QTextEdit();
    description_edit_->document()->setPlainText(description_.c_str());
    //description_edit_->setWordWrapMode(QTextOption::WrapMode::WrapAnywhere);

    connect(description_edit_, &QTextEdit::textChanged, this,
            &VariableCreateDialog::commentChangedSlot);

    form_layout->addRow("Comment", description_edit_);

    type_combo_ = new dbContent::VariableDataTypeComboBox(data_type_, data_type_str_);
    form_layout->addRow("Data Type", type_combo_);

    unit_sel_ = new UnitSelectionWidget(dimension_, unit_);
    form_layout->addRow("Unit", unit_sel_);

    representation_box_ = new StringRepresentationComboBox(representation_, representation_str_);
    form_layout->addRow("Representation", representation_box_);

    if (name_.size())
    {
        db_column_name_ = name;

        std::replace_if(db_column_name_.begin(), db_column_name_.end(), [](char ch) {
                return !(isalnum(ch) || ch == '_');
            }, '_');

        boost::algorithm::to_lower(db_column_name_);
    }

    db_column_edit_ = new QLineEdit(db_column_name_.c_str());

    connect(db_column_edit_, &QLineEdit::textChanged, this, &VariableCreateDialog::dbColumnChangedSlot);
    form_layout->addRow("DBColumn", db_column_edit_);

    main_layout->addLayout(form_layout);

    main_layout->addStretch();

    // buttons

    QHBoxLayout* button_layout = new QHBoxLayout();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &VariableCreateDialog::reject);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    ok_button_ = new QPushButton("OK");
    connect(ok_button_, &QPushButton::clicked, this, &VariableCreateDialog::accept);
    button_layout->addWidget(ok_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    invalid_bg_str_ = "QLineEdit { background: rgb(255, 100, 100); selection-background-color:"
                    " rgb(255, 200, 200); }";

    valid_bg_str_ = "QLineEdit { background: rgb(255, 255, 255); selection-background-color:"
                    " rgb(200, 200, 200); }";

    checkSettings();
}

std::string VariableCreateDialog::name() const
{
    return name_;
}

std::string VariableCreateDialog::shortName() const
{
    return short_name_;
}

std::string VariableCreateDialog::dataTypeStr() const
{
    return Property::asString(data_type_);
}

std::string VariableCreateDialog::dimension() const
{
    return dimension_;
}

std::string VariableCreateDialog::unit() const
{
    return unit_;
}

std::string VariableCreateDialog::representationStr() const
{
    return representation_str_;
}

std::string VariableCreateDialog::description() const
{
    return description_;
}

std::string VariableCreateDialog::dbColumnName() const
{
    return db_column_name_;
}

void VariableCreateDialog::nameChangedSlot(const QString& name)
{
    loginf << "VariableCreateDialog: nameChangedSlot: name '" << name.toStdString() << "'";

    assert (db_column_edit_);

    name_ = name.toStdString();

    if (name_.size())
    {
        db_column_name_ = name.toStdString();

        std::replace_if(db_column_name_.begin(), db_column_name_.end(), [](char ch) {
                return !(isalnum(ch) || ch == '_');
            }, '_');

        boost::algorithm::to_lower(db_column_name_);

        db_column_edit_->setText(db_column_name_.c_str());
    }

    checkSettings();
}

void VariableCreateDialog::shortNameChangedSlot(const QString& name)
{
    short_name_ = name.toStdString();
}

void VariableCreateDialog::commentChangedSlot()
{
    assert (description_edit_);

    description_ = description_edit_->document()->toPlainText().toStdString();
}

void VariableCreateDialog::dbColumnChangedSlot(const QString& name)
{
    loginf << "VariableCreateDialog: dbColumnChangedSlot: name '" << name.toStdString() << "'";

    assert (db_column_edit_);

    db_column_name_ = name.toStdString();

    // check lower case lower case

    string lower = db_column_name_;
    boost::algorithm::to_lower(lower);

    if (db_column_name_ != lower)
    {
        db_column_name_ = lower;
        db_column_edit_->setText(db_column_name_.c_str());
    }

    checkSettings();
}

void VariableCreateDialog::checkSettings()
{
    assert (name_edit_);
    assert (db_column_edit_);
    assert (ok_button_);

    // check name
    string name_quicktip;

    if (!name_.size())
    {
        name_ok_ = false;
        name_quicktip = "Can not be empty";
    }
    else if (object_.hasVariable(name_))
    {
        name_ok_ = false;
        name_quicktip = "Name '"+name_+"' already in use";
    }
    else // ok
        name_ok_ = true;

    if (name_ok_)
        name_edit_->setStyleSheet(valid_bg_str_.c_str());
    else
        name_edit_->setStyleSheet(invalid_bg_str_.c_str());

    name_edit_->setToolTip(name_quicktip.c_str());

    // check db column name
    string db_column_quicktip;

    if (!db_column_name_.size())
    {
        db_column_name_ok_ = false;
        db_column_quicktip = "Can not be empty";
    }
    else if (object_.hasVariableDBColumnName(db_column_name_))
    {
        db_column_name_ok_ = false;
        db_column_quicktip = "Column name '"+db_column_name_+"' already in use";
    }
    else if (find_if(db_column_name_.begin(), db_column_name_.end(), [](char ch) {
                        return !(isalnum(ch) || ch == '_');
                    }) != db_column_name_.end())
    {
        db_column_name_ok_ = false;
        db_column_quicktip = "Column name '"+db_column_name_
                +"' can only contain alphanumeric characters and underscores";
    }
    else // ok
        db_column_name_ok_ = true;


    if (db_column_name_ok_)
        db_column_edit_->setStyleSheet(valid_bg_str_.c_str());
    else
        db_column_edit_->setStyleSheet(invalid_bg_str_.c_str());

    db_column_edit_->setToolTip(db_column_quicktip.c_str());

    ok_button_->setDisabled(!name_ok_ || !db_column_name_ok_);
}

}
