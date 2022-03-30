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

#include "asterixoverridewidget.h"
#include "asteriximporttask.h"
#include "textfielddoublevalidator.h"

#include <QCheckBox>
#include <QDoubleValidator>
#include <QGridLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

ASTERIXOverrideWidget::ASTERIXOverrideWidget(ASTERIXImportTask& task, QWidget* parent)
    : QWidget(parent), task_(task)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    active_check_ = new QCheckBox("Override Time of Day Active");
    connect(active_check_, &QCheckBox::clicked, this, &ASTERIXOverrideWidget::activeCheckedSlot);
    main_layout->addWidget(active_check_);

    QGridLayout* grid = new QGridLayout();

    unsigned int row = 0;

    // tod offset
    grid->addWidget(new QLabel("Time of Day Offset"), row, 0);

    tod_offset_edit_ = new QLineEdit();
    tod_offset_edit_->setValidator(new TextFieldDoubleValidator(-24 * 3600, 24 * 3600, 3));
    connect(tod_offset_edit_, &QLineEdit::textEdited, this, &ASTERIXOverrideWidget::todOffsetEditedSlot);
    grid->addWidget(tod_offset_edit_, row, 1);

    main_layout->addLayout(grid);

    main_layout->addStretch();

    updateSlot();

    setLayout(main_layout);
}

ASTERIXOverrideWidget::~ASTERIXOverrideWidget() {}

void ASTERIXOverrideWidget::updateSlot()
{
    assert(active_check_);
    assert(tod_offset_edit_);

    active_check_->setChecked(task_.overrideTodActive());

    tod_offset_edit_->setText(QString::number(task_.overrideTodOffset()));
}

void ASTERIXOverrideWidget::activeCheckedSlot()
{
    loginf << "ASTERIXOverrideWidget: activeCheckedSlot";
    assert(active_check_);

    task_.overrideTodActive(active_check_->checkState() == Qt::Checked);
}

void ASTERIXOverrideWidget::todOffsetEditedSlot(const QString& value)
{
    loginf << "ASTERIXOverrideWidget: todOffsetEditedSlot: value '" << value.toStdString() << "'";
    TextFieldDoubleValidator::displayValidityAsColor(tod_offset_edit_);

    if (tod_offset_edit_->hasAcceptableInput())
        task_.overrideTodOffset(tod_offset_edit_->text().toFloat());
}
