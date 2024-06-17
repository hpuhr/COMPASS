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

#include "variableviewannotationwidget.h"
#include "variableview.h"

#include <QVBoxLayout>
#include <QComboBox>

/**
*/
VariableViewAnnotationWidget::VariableViewAnnotationWidget(const VariableView* view, QWidget* parent)
:   QWidget(parent)
,   view_  (view  )
{
    assert(view_);

    createUI();
    updateContent();
}

/**
*/
VariableViewAnnotationWidget::~VariableViewAnnotationWidget() = default;

/**
*/
void VariableViewAnnotationWidget::createUI()
{
    QVBoxLayout* layout = new QVBoxLayout;

    combo_ = new QComboBox;
    layout->addWidget(combo_);

    connect(combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VariableViewAnnotationWidget::idChanged);

    setLayout(layout);
}

/**
*/
void VariableViewAnnotationWidget::updateContent()
{
    combo_->blockSignals(true);

    combo_->clear();
    for (const auto& a : view_->annotations())
        combo_->addItem(QString::fromStdString(a.first));

    int idx = combo_->findText(QString::fromStdString(view_->currentAnnotationID()));
    if (idx >= 0)
        combo_->setCurrentIndex(idx);

    combo_->blockSignals(false);
}

/**
*/
std::string VariableViewAnnotationWidget::currentID() const
{
    return combo_->currentText().toStdString();
}
