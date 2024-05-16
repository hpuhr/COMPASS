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

#include "viewevaldatawidget.h"

#include <QFormLayout>
#include <QLabel>

/**
*/
ViewEvalDataIDWidget::ViewEvalDataIDWidget(QWidget* parent)
{
    createUI();
    updateContent();
}

/**
*/
ViewEvalDataIDWidget::~ViewEvalDataIDWidget() = default;

/**
*/
void ViewEvalDataIDWidget::setID(const ViewEvalDataID& id,
                                 bool notify_changes)
{
    id_ = id;

    updateContent();

    if (notify_changes)
        emit idChanged();
}

/**
*/
ViewEvalDataID ViewEvalDataIDWidget::getID() const
{
    return id_;
}

/**
*/
void ViewEvalDataIDWidget::createUI()
{
    QFormLayout* layout = new QFormLayout;
    layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);

    eval_results_grpreq_label_ = new QLabel();
    eval_results_grpreq_label_->setWordWrap(true);

    layout->addRow(tr("Requirement"), eval_results_grpreq_label_);

    eval_results_id_label_ = new QLabel();
    eval_results_id_label_->setWordWrap(true);

    layout->addRow(tr("Result"), eval_results_id_label_);

    setLayout(layout);
}

/**
*/
void ViewEvalDataIDWidget::updateContent()
{
    bool valid = id_.valid();

    eval_results_grpreq_label_->setText(valid ? QString::fromStdString(id_.eval_results_grpreq) : "-");
    eval_results_id_label_->setText(valid ? QString::fromStdString(id_.eval_results_id) : "-");
}
