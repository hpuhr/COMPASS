#include "metadbovariabledetailwidget.h"
#include "metadbovariable.h"
#include "dbobjectmanager.h"
#include "dbovariableselectionwidget.h"
#include "logger.h"

#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QFormLayout>
#include <QPushButton>

using namespace std;

MetaDBOVariableDetailWidget::MetaDBOVariableDetailWidget(DBObjectManager& dbo_man, QWidget *parent)
    : QWidget(parent), dbo_man_(dbo_man)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QFormLayout* form_layout = new QFormLayout();

    name_edit_ = new QLineEdit();
    connect(name_edit_, &QLineEdit::textEdited,
            this, &MetaDBOVariableDetailWidget::nameEditedSlot);
    name_edit_->setDisabled(true);
    form_layout->addRow("Name", name_edit_);

    description_edit_ = new QTextEdit();
    description_edit_->setDisabled(true);
    description_edit_->setWordWrapMode(QTextOption::WrapMode::WrapAnywhere);

    connect(description_edit_, &QTextEdit::textChanged, this,
            &MetaDBOVariableDetailWidget::commentEditedSlot);

    form_layout->addRow("Comment", description_edit_);

    for (auto dbcont_it = dbo_man_.begin(); dbcont_it != dbo_man_.end(); ++dbcont_it)
    {
        DBOVariableSelectionWidget* var_sel = new DBOVariableSelectionWidget(true);
        var_sel->showDBOOnly(dbcont_it->first);
        var_sel->setProperty("DBObject", dbcont_it->first.c_str());

        connect(var_sel, &DBOVariableSelectionWidget::selectionChanged,
                this, &MetaDBOVariableDetailWidget::variableChangedSlot);

        var_sel->setDisabled(true);
        form_layout->addRow(dbcont_it->first.c_str(), var_sel);
    }

    main_layout->addLayout(form_layout);


    delete_button_ = new QPushButton("Delete");
    connect(delete_button_, &QPushButton::clicked,
            this, &MetaDBOVariableDetailWidget::deleteVariableSlot);
    delete_button_->setDisabled(true);

    main_layout->addWidget(delete_button_);

    setLayout(main_layout);
}


void MetaDBOVariableDetailWidget::show (MetaDBOVariable& meta_var)
{
    loginf << "MetaDBOVariableDetailWidget: show: var '" << meta_var.name() << "'";

    //    if (variable_.existsIn(obj_it.first))
    //        var_sel->selectedVariable(variable_.getFor(obj_it.first));
}

void MetaDBOVariableDetailWidget::nameEditedSlot(const QString& name)
{
    string new_name = name.toStdString();

    loginf << "MetaDBOVariableDetailWidget: nameEditedSlot: name '" << new_name << "'";
}


void MetaDBOVariableDetailWidget::commentEditedSlot()
{
    loginf << "MetaDBOVariableDetailWidget: commentEditedSlot";
}

void MetaDBOVariableDetailWidget::variableChangedSlot()
{
    loginf << "MetaDBOVariableDetailWidget: variableChangedSlot";
}

void MetaDBOVariableDetailWidget::deleteVariableSlot()
{
    loginf << "MetaDBOVariableDetailWidget: deleteVariableSlot";
}
