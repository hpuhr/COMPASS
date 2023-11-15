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

#include "viewpresets.h"

#include <QWidget>

class View;

class QComboBox;
class QToolButton;
class QLabel;
class QTextEdit;
class QDialog;

/**
 * Widget governing a view's presets.
*/
class ViewPresetWidget : public QWidget
{
public:
    ViewPresetWidget(View* view,
                     QWidget* parent = nullptr);
    virtual ~ViewPresetWidget() = default;

private:
    void createUI();
    void refill();
    void addPreset();
    void updatePreset();
    void removeCurrentPreset();
    void applyCurrentPreset();
    void currentPresetChanged();

    void configurePreset(const ViewPresets::Preset* preset);

    View* view_ = nullptr;

    std::vector<ViewPresets::Key> current_keys_;

    QComboBox*   preset_combo_  = nullptr;
    QToolButton* add_button_    = nullptr;
    QToolButton* update_button_ = nullptr;
    QToolButton* rem_button_    = nullptr;
    QToolButton* apply_button_  = nullptr;
    QLabel*      preview_label_ = nullptr;
    QTextEdit*   descr_edit_    = nullptr;
};
