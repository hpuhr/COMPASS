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
#include "colorlegend.h"

#include <QWidget>

class ColorMap;

class QHBoxLayout;

/**
*/
class ColorLegendWidget : public QWidget
{
public:
    ColorLegendWidget(QWidget* parent = nullptr);
    virtual ~ColorLegendWidget();

    void setColorMap(const ColorMap& colormap);
    void setDecorator(const ColorMap::ValueDecorator& d);
    void showSelectionColor(bool ok);
    void showNullColor(bool ok);

    void updateUI();

    const ColorLegend& currentLegend() const { return legend_; }

private:
    void createUI();

    ColorMap                 colormap_;
    ColorLegend              legend_;
    ColorMap::ValueDecorator decorator_;
    bool                     show_selection_col_ = false;
    bool                     show_null_col_ = false;

    QHBoxLayout* layout_ = nullptr;
    QWidget*     widget_ = nullptr;
};
