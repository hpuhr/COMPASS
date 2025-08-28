/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "evaluationstandardtabwidget.h"
#include "evaluationmanagerwidget.h"
#include "evaluationmanager.h"
#include "evaluationstandardcombobox.h"
#include "evaluationstandard.h"
#include "evaluationstandardwidget.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QStackedWidget>
#include <QFrame>
#include <QTreeView>
#include <QFormLayout>

using namespace std;

/**
 */
EvaluationStandardTabWidget::EvaluationStandardTabWidget(EvaluationCalculator& calculator)
    : QWidget(nullptr), calculator_(calculator)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QFont font_bold;
    font_bold.setBold(true);

    // standard
    {
        QHBoxLayout* std_layout = new QHBoxLayout();

        QLabel* standard_label = new QLabel("Standard");
        standard_label->setFont(font_bold);
        std_layout->addWidget(standard_label);

        standard_box_.reset(new EvaluationStandardComboBox(calculator_));

        std_layout->addWidget(standard_box_.get());

        main_layout->addLayout(std_layout);
    }

    // buttons
    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        add_button_ = new QPushButton("Add");
        connect (add_button_, &QPushButton::clicked,
                 this, &EvaluationStandardTabWidget::addStandardSlot);
        button_layout->addWidget(add_button_);

        rename_button_ = new QPushButton("Rename");
        connect (rename_button_, &QPushButton::clicked,
                 this, &EvaluationStandardTabWidget::renameStandardSlot);
        button_layout->addWidget(rename_button_);

        copy_button_ = new QPushButton("Copy");
        connect (copy_button_, &QPushButton::clicked,
                 this, &EvaluationStandardTabWidget::copyStandardSlot);
        button_layout->addWidget(copy_button_);

        remove_button_ = new QPushButton("Remove");
        connect (remove_button_, &QPushButton::clicked,
                 this, &EvaluationStandardTabWidget::removeStandardSlot);
        button_layout->addWidget(remove_button_);

        updateButtons();

        main_layout->addLayout(button_layout);
    }

    // standards stack
//    {
        // standards_widget_ = new QStackedWidget();
        // main_layout->addWidget(standards_widget_);
//    }

    standards_layout_ = new QHBoxLayout();
    main_layout->addLayout(standards_layout_);

    if (calculator_.hasCurrentStandard())
        updateStandardWidget();

    // some cfg
    {
        QFormLayout* form_layout = new QFormLayout();

        // max ref time diff
        max_ref_time_diff_edit_ = new QLineEdit(QString::number(calculator_.settings().max_ref_time_diff_));
        max_ref_time_diff_edit_->setValidator(new QDoubleValidator(0.0, 30.0, 2, this));
        connect(max_ref_time_diff_edit_, &QLineEdit::textEdited,
                this, &EvaluationStandardTabWidget::maxRefTimeDiffEditSlot);

        form_layout->addRow("Reference Maximum Time Difference [s]", max_ref_time_diff_edit_);

        main_layout->addLayout(form_layout);
    }

    // connections
    connect (&calculator_, &EvaluationCalculator::standardsChanged,
             this, &EvaluationStandardTabWidget::changedStandardsSlot);
    connect (&calculator_, &EvaluationCalculator::currentStandardChanged,
             this, &EvaluationStandardTabWidget::changedCurrentStandardSlot);

    setContentsMargins(0, 0, 0, 0);
    setLayout(main_layout);
}

/**
 */
void EvaluationStandardTabWidget::changedStandardsSlot()
{
    loginf << "start";

    traced_assert(standard_box_);
    standard_box_->updateStandards();
}

/**
 */
void EvaluationStandardTabWidget::changedCurrentStandardSlot()
{
    loginf << "start";

    traced_assert(standard_box_);
    standard_box_->setStandardName(calculator_.currentStandardName());

    updateButtons();
    updateStandardWidget();
}

/**
 */
void EvaluationStandardTabWidget::addStandardSlot ()
{
    loginf << "start";

    bool ok;
    QString text =
            QInputDialog::getText(this, tr("Standard Name"),
                                  tr("Specify a (unique) standard name:"), QLineEdit::Normal, "", &ok);

    if (ok)
    {
        std::string name = text.toStdString();

        if (!name.size())
        {
            QMessageBox m_warning(QMessageBox::Warning, "Adding Standard Failed",
                                  "Standard has to have a non-empty name.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        if (calculator_.hasStandard(name))
        {
            QMessageBox m_warning(QMessageBox::Warning, "Adding Standard Failed",
                                  "Standard with this name already exists.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        calculator_.addStandard(name);
    }
}

/**
 */
void EvaluationStandardTabWidget::renameStandardSlot ()
{
    loginf << "start";

    bool ok;
    QString text =
            QInputDialog::getText(this, tr("Standard Name"),
                                  tr("Specify a (unique) standard name:"), QLineEdit::Normal,
                                  calculator_.currentStandardName().c_str(), &ok);

    if (ok)
    {
        string new_name = text.toStdString();

        if (!new_name.size())
        {
            QMessageBox m_warning(QMessageBox::Warning, "Renaming Standard Failed",
                                  "Standard with empty name not possible.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        if (calculator_.hasStandard(new_name))
        {
            QMessageBox m_warning(QMessageBox::Warning, "Renaming Standard Failed",
                                  "Standard with this name already exists.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        calculator_.renameCurrentStandard(new_name);
    }
}

/**
 */
void EvaluationStandardTabWidget::copyStandardSlot ()
{
    loginf << "start";

    bool ok;
    QString text =
            QInputDialog::getText(this, tr("New Standard Name"),
                                  tr("Specify a (unique) standard name:"), QLineEdit::Normal, "", &ok);

    if (ok)
    {
        string new_name = text.toStdString();

        if (!new_name.size())
        {
            QMessageBox m_warning(QMessageBox::Warning, "Copying Standard Failed",
                                  "Standard with empty name not possible.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        if (calculator_.hasStandard(new_name))
        {
            QMessageBox m_warning(QMessageBox::Warning, "Copying Standard Failed",
                                  "Standard with this name already exists.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        calculator_.copyCurrentStandard(new_name);
    }
}

/**
 */
void EvaluationStandardTabWidget::removeStandardSlot ()
{
    loginf << "start";

    traced_assert(calculator_.hasCurrentStandard());
    calculator_.deleteCurrentStandard();
}

/**
 */
void EvaluationStandardTabWidget::updateButtons()
{
    traced_assert(add_button_);
    add_button_->setDisabled(false);
    traced_assert(rename_button_);
    rename_button_->setEnabled(calculator_.hasCurrentStandard());
    traced_assert(copy_button_);
    copy_button_->setEnabled(calculator_.hasCurrentStandard());
    traced_assert(remove_button_);
    remove_button_->setEnabled(calculator_.hasCurrentStandard());
}

/**
 */
void EvaluationStandardTabWidget::updateStandardWidget()
{
    // traced_assert(standards_widget_);

    // string standard_name = calculator_.currentStandardName();

    // if (!standard_name.size())
    // {
    //     while (standards_widget_->count() > 0)  // remove all widgets
    //     {
    //         auto std_widget = standards_widget_->widget(0);
    //         standards_widget_->removeWidget(std_widget);
    //         delete std_widget;
    //     }
    //     return;
    // }

    // EvaluationStandard& standard = calculator_.currentStandard();

    // if (standards_widget_->indexOf(standard.widget()) < 0)
    //     standards_widget_->addWidget(standard.widget());

    // standards_widget_->setCurrentWidget(standard.widget());

    traced_assert(standards_layout_);
    QLayoutItem* item;
    while ((item = standards_layout_->takeAt(0)) != nullptr)
    {
        if (QWidget* widget = item->widget())
        {
            widget->setParent(nullptr); // Optional: disconnect from layout
            delete widget;
        }
        delete item;
    }

    if (calculator_.hasCurrentStandard())
    {
        auto* widget = calculator_.currentStandard().widget();
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);;

        standards_layout_->addWidget(widget);
    }
}

/**
 */
void EvaluationStandardTabWidget::maxRefTimeDiffEditSlot(QString value)
{
    loginf << "value " << value.toStdString();

    bool ok;
    float val = value.toFloat(&ok);

    if (ok)
        calculator_.settings().max_ref_time_diff_ = val;
    else
        loginf << "invalid value";
}
