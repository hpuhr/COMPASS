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
    connect(name_edit_, &QLineEdit::editingFinished,
            this, &MetaDBOVariableDetailWidget::nameEditedSlot);
    name_edit_->setDisabled(true);
    form_layout->addRow("Name", name_edit_);

    description_edit_ = new QTextEdit();
    description_edit_->setReadOnly(true);
    description_edit_->setWordWrapMode(QTextOption::WrapMode::WrapAnywhere);

    form_layout->addRow("Comment", description_edit_);

    for (auto dbcont_it = dbo_man_.begin(); dbcont_it != dbo_man_.end(); ++dbcont_it)
    {
        DBOVariableSelectionWidget* var_sel = new DBOVariableSelectionWidget(true);
        var_sel->showDBOOnly(dbcont_it->first);
        var_sel->setProperty("DBObject", dbcont_it->first.c_str());

        connect(var_sel, &DBOVariableSelectionWidget::selectionChanged,
                this, &MetaDBOVariableDetailWidget::variableChangedSlot);

        var_sel->setDisabled(true);

        selection_widgets_[dbcont_it->first] = var_sel;
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

    has_current_entry_ = true;
    meta_var_ = &meta_var;

    name_edit_->setText(meta_var.name().c_str());
    name_edit_->setDisabled(false);

    description_edit_->document()->setPlainText(meta_var.description().c_str());

    for (auto& sel_it : selection_widgets_)
    {
        loginf << "MetaDBOVariableDetailWidget: show: var '" << meta_var.name() << "' exists in " << sel_it.first
               << " " << meta_var.existsIn(sel_it.first);

        if (meta_var.existsIn(sel_it.first))
            sel_it.second->selectedVariable(meta_var.getFor(sel_it.first));
        else
            sel_it.second->selectEmptyVariable();

        sel_it.second->setDisabled(false);

    }

    delete_button_->setDisabled(false);
}

void MetaDBOVariableDetailWidget::clear()
{
    has_current_entry_ = false;
    meta_var_ = nullptr;

    name_edit_->setText("");
    name_edit_->setDisabled(true);

    description_edit_->document()->setPlainText("");

    for (auto& sel_it : selection_widgets_)
    {
        sel_it.second->selectEmptyVariable();
        sel_it.second->setDisabled(true);
    }

    delete_button_->setDisabled(true);
}

void MetaDBOVariableDetailWidget::nameEditedSlot()
{
    if (!has_current_entry_)
        return;

    assert (name_edit_);

    string new_name = name_edit_->text().toStdString();

    loginf << "MetaDBOVariableDetailWidget: nameEditedSlot: name '" << new_name << "'";

    assert (has_current_entry_);
    assert (meta_var_);

    dbo_man_.renameMetaVariable(meta_var_->name(), new_name);
}


void MetaDBOVariableDetailWidget::variableChangedSlot()
{
    loginf << "MetaDBOVariableDetailWidget: variableChangedSlot";

    if (!has_current_entry_)
        return;

    DBOVariableSelectionWidget* sel_widget = dynamic_cast<DBOVariableSelectionWidget*>(QObject::sender());
    assert (sel_widget);
    assert (sel_widget->hasVariable());

    assert (meta_var_);
    meta_var_->set(sel_widget->selectedVariable());
}

void MetaDBOVariableDetailWidget::deleteVariableSlot()
{
    loginf << "MetaDBOVariableDetailWidget: deleteVariableSlot";

    assert (has_current_entry_);
    assert (meta_var_);

    dbo_man_.deleteMetaVariable(meta_var_->name());
}
