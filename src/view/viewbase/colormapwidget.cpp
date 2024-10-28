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

#include "colormapwidget.h"

//#include "colorscales.h"

#include "rangeedit.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

/*****************************************************************************************
 * ColorLegendWidget
 *****************************************************************************************/

/**
*/
ColorLegendWidget::ColorLegendWidget(QWidget* parent)
:   QWidget(parent)
{
    createUI();
}

/**
*/
ColorLegendWidget::~ColorLegendWidget() = default;

/**
*/
void ColorLegendWidget::setColorMap(const ColorMap& colormap)
{
    colormap_ = colormap;

    updateUI();
}

/**
*/
void ColorLegendWidget::setDescriptionMode(ColorMapDescriptionMode mode)
{
    if (descr_mode_ == mode)
        return;

    descr_mode_ = mode;

    updateUI();
}

/**
*/
void ColorLegendWidget::setDecorator(const ColorMap::ValueDecorator& d)
{
    decorator_ = d;

    updateUI();
}

/**
*/
void ColorLegendWidget::showSelectionColor(bool ok)
{
    if (show_selection_col_ == ok)
        return;

    show_selection_col_ = ok;

    updateUI();
}

/**
*/
void ColorLegendWidget::showNullColor(bool ok)
{
    if (show_null_col_ == ok)
        return;
    
    show_null_col_ = ok;

    updateUI();
}

/**
*/
void ColorLegendWidget::createUI()
{
    layout_ = new QHBoxLayout;
    setLayout(layout_);
}

/**
*/
void ColorLegendWidget::updateUI()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    if (colormap_.numColors() < 1)
        return;

    widget_ = new QWidget;

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    widget_->setLayout(layout);

    QGridLayout* layout_grid = new QGridLayout;
    layout_grid->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(layout_grid);
    layout->addStretch(1);

    auto descr = colormap_.getDescription(descr_mode_, 
                                          show_selection_col_, 
                                          show_null_col_, 
                                          decorator_);
    int row = 0;
    for (const auto& d : descr)
    {
        QImage img(16, 16, QImage::Format_ARGB32);
        img.fill(d.first);

        QLabel* l = new QLabel;
        l->setPixmap(QPixmap::fromImage(img));
        l->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

        QLabel* l2 = new QLabel;
        l2->setText(QString::fromStdString(d.second));
        l2->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

        layout_grid->addWidget(l , row, 0);
        layout_grid->addWidget(l2, row, 1);

        ++row;
    }

    layout_->addWidget(widget_);
}

/*****************************************************************************************
 * ColorMapWidget
 *****************************************************************************************/

/**
*/
ColorMapWidget::ColorMapWidget(QWidget* parent)
:   QWidget(parent)
{
    createUI();
}

/**
*/
ColorMapWidget::~ColorMapWidget() = default;

/**
*/
void ColorMapWidget::setColorMap(ColorMap* colormap)
{
    colormap_ = colormap;

    updateUI();
}

/**
*/
void ColorMapWidget::setValueRange(double vmin, double vmax)
{
    assert(vmin <= vmax);

    vmin_ = vmin;
    vmax_ = vmax;

    updateUI();
    updateColormap();
}

/**
*/
void ColorMapWidget::createUI()
{
    QFormLayout* layout = new QFormLayout;
    setLayout(layout);

    scale_combo_ = ColorMap::generateScaleSelection();
    connect(scale_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColorMapWidget::updateColormap);

    layout->addRow("Color Scale:", scale_combo_);

    steps_box_ = new QSpinBox;
    steps_box_->setMinimum(1);
    steps_box_->setMaximum(20);
    connect(steps_box_, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColorMapWidget::updateColormap);
    
    layout->addRow("Steps:", steps_box_);

    {
        range_widget_ = new QWidget;

        QVBoxLayout* layout = new QVBoxLayout;
        layout->setMargin(0);
        layout->setSpacing(1);
        range_widget_->setLayout(layout);

        QHBoxLayout* hlayout = new QHBoxLayout;
        hlayout->setMargin(0);
        hlayout->setSpacing(1);
        layout->addLayout(hlayout);

        range_min_box_ = new QLineEdit();
        connect(range_min_box_, &QLineEdit::textEdited, this, &ColorMapWidget::updateColormap);

        hlayout->addWidget(range_min_box_);

        hlayout->addStretch(1);

        range_max_box_ = new QLineEdit();
        connect(range_max_box_, &QLineEdit::textEdited, this, &ColorMapWidget::updateColormap);

        hlayout->addWidget(range_max_box_);

        range_edit_ = new RangeEditDouble(SliderSteps, Precision);
        range_edit_->connectToFields(range_min_box_, range_max_box_);

        layout->addWidget(range_edit_);
    }

    range_label_ = new QLabel("Value Range:");

    layout->addRow(range_label_, range_widget_);
}

/**
*/
void ColorMapWidget::updateUI()
{
    if (!colormap_)
        return;

    scale_combo_->blockSignals(true);

    int idx = scale_combo_->findData((int)colormap_->colorScale());
    if (idx >= 0)
        scale_combo_->setCurrentIndex(idx);
    else
        scale_combo_->setEnabled(false);

    scale_combo_->blockSignals(false);

    steps_box_->blockSignals(true);
    steps_box_->setValue(colormap_->colorSteps());
    steps_box_->blockSignals(false);

    if (vmin_.has_value() && vmax_.has_value())
    {
        const double vmin = vmin_.value();
        const double vmax = vmax_.value();

        const QString limit0 = QString::number(vmin, 'f', Precision);
        const QString limit1 = QString::number(vmax, 'f', Precision);

        range_min_box_->setValidator(new TextFieldDoubleValidator(vmin, vmax, Precision));
        range_max_box_->setValidator(new TextFieldDoubleValidator(vmin, vmax, Precision));

        range_edit_->setLimits(limit0, limit1);

        range_min_box_->setText(limit0);
        range_max_box_->setText(limit1);

        range_label_->setVisible(true);
        range_widget_->setVisible(true);
    }
    else
    {
        range_label_->setVisible(false);
        range_widget_->setVisible(false);
    }
}

/**
*/
void ColorMapWidget::updateColormap()
{
    ColorMap::OValueRange vrange;
    
    if (vmin_.has_value() && vmax_.has_value())
    {
        bool ok0, ok1;
        const double vmin = range_min_box_->text().toDouble(&ok0);
        const double vmax = range_max_box_->text().toDouble(&ok1);

        bool ok = (!range_min_box_->text().isEmpty() && ok0 &&
                   !range_max_box_->text().isEmpty() && ok1);
        
        vrange = std::make_pair(ok ? vmin : vmin_.value(), 
                                ok ? vmax : vmax_.value());
    }

    colormap_->create((ColorMap::ColorScale)scale_combo_->currentData().toInt(),
                      (size_t)steps_box_->value(),
                      ColorMap::Type::Linear,
                      vrange);

    loginf << "ColorMapWidget: updateColormap:"
           << " scale " << (int)colormap_->colorScale()
           << " steps " << colormap_->colorSteps()
           << " range_min " << (vrange.has_value() ? std::to_string(vrange.value().first ) : "-")
           << " range_max " << (vrange.has_value() ? std::to_string(vrange.value().second) : "-");

    for (size_t i = 0; i < colormap_->numColors(); ++i)
        loginf << "    " << colormap_->getColor(i).redF() << "," << colormap_->getColor(i).greenF() << "," << colormap_->getColor(i).blueF();

    emit colorMapChanged();
}
