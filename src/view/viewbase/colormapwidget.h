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

#pragma once

#include "colormap.h"

#include <QWidget>

class ColorMap;
class RangeEditDouble;

class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class QSpinBox;
class QLineEdit;
class QLabel;

/**
*/
class ColorLegendWidget : public QWidget
{
public:
    ColorLegendWidget(QWidget* parent = nullptr);
    virtual ~ColorLegendWidget();

    void setColorMap(ColorMap* colormap);

    void updateUI();

private:
    void createUI();

    ColorMap* colormap_ = nullptr;

    QHBoxLayout* layout_ = nullptr;
    QWidget*     widget_ = nullptr;
};

/**
*/
class ColorMapWidget : public QWidget
{
    Q_OBJECT
public:
    ColorMapWidget(QWidget* parent = nullptr);
    virtual ~ColorMapWidget();

    void setColorMap(ColorMap* colormap);
    void setValueRange(double vmin, double vmax);

    void updateUI();

    static const int SliderSteps = 1000;
    static const int Precision   = 6; 

signals:
    void colorMapChanged();

private:
    void createUI();
    void updateColormap();

    ColorMap*               colormap_ = nullptr;
    boost::optional<double> vmin_;
    boost::optional<double> vmax_;

    QComboBox*       scale_combo_   = nullptr;
    QSpinBox*        steps_box_     = nullptr;

    QLabel*          range_label_   = nullptr;
    QWidget*         range_widget_  = nullptr;
    QLineEdit*       range_min_box_ = nullptr;
    QLineEdit*       range_max_box_ = nullptr;
    RangeEditDouble* range_edit_    = nullptr;
};
