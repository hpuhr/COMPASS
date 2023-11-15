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
#include <QFrame>

class View;

class QListWidget;
class QToolButton;
class QPushButton;
class QLabel;
class QVBoxLayout;

class QDialog;

/**
*/
class ViewPresetItemWidget : public QFrame
{
    Q_OBJECT
public:
    ViewPresetItemWidget(const ViewPresets::Preset* preset, QWidget* parent = nullptr);
    virtual ~ViewPresetItemWidget() = default;

    const ViewPresets::Preset* getPreset() const { return preset_; }

    void updateContents();

signals:
    void modifyPresetRequested(ViewPresets::Key key);
    void removePresetRequested(ViewPresets::Key key);
    void applyPresetRequested(ViewPresets::Key key);

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void createUI();

    const ViewPresets::Preset* preset_ = nullptr;

    QLabel* category_label_    = nullptr;
    QLabel* name_label_        = nullptr;
    QLabel* preview_label_     = nullptr;
    QLabel* description_label_ = nullptr;

    QToolButton* remove_button_ = nullptr;
    QToolButton* edit_button_   = nullptr;
    QToolButton* apply_button_  = nullptr;

    bool inside_ = false;
};

/**
*/
class ViewPresetItemListWidget : public QWidget
{
    Q_OBJECT
public:
    ViewPresetItemListWidget(QWidget* parent = nullptr);
    virtual ~ViewPresetItemListWidget() = default;

    void clear();
    void addItem(ViewPresetItemWidget* item);

signals:
    void removed(ViewPresets::Key key);
    void modify(ViewPresets::Key key);
    void apply(ViewPresets::Key key);

private:
    void createUI();
    void removeItem(ViewPresets::Key key);
    void modifyItem(ViewPresets::Key key);

    QVBoxLayout* item_layout_;
    std::vector<ViewPresetItemWidget*> items_;
};

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

    void removePreset(ViewPresets::Key key);
    void modifyPreset(ViewPresets::Key key);
    void applyPreset(ViewPresets::Key key);

    void configurePreset(const ViewPresets::Preset* preset);

    View* view_ = nullptr;

    ViewPresetItemListWidget* preset_list_   = nullptr;
    QPushButton*              show_button_   = nullptr;
    QToolButton*              add_button_    = nullptr;
};
