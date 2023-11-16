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

class QListWidget;
class QToolButton;
class QPushButton;
class QLabel;
class QVBoxLayout;
class QLineEdit;
class QTextEdit;

class QDialog;

/**
*/
class ViewPresetItemWidget : public QWidget
{
    Q_OBJECT
public:
    enum class ItemState
    {
        Ready = 0,
        Edit  = 1
    };

    ViewPresetItemWidget(const ViewPresets::Key& key,
                         View* view,
                         QWidget* parent = nullptr);
    virtual ~ViewPresetItemWidget() = default;

    const ViewPresets::Preset* getPreset() const { return preset_; }

    void updateContents();

signals:
    void removePresetRequested(ViewPresets::Key key);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void createUI();
    void setItemState(ItemState item_state);
    void updateDescriptionEdit();
    void updateCursors();
    void updateActivity();

    void editDescriptionToggled(bool edit);

    void removeButtonPressed();
    void updateButtonPressed();

    void applyPreset();
    bool updatePreset(const ViewPresets::Preset& preset);
    
    ViewPresets::Key     key_;
    ViewPresets::Preset* preset_ = nullptr;
    View*                view_   = nullptr;

    QLabel*    category_label_        = nullptr;
    QLabel*    name_label_            = nullptr;
    QLabel*    preview_label_         = nullptr;
    QLabel*    description_label_     = nullptr;
    QTextEdit* description_edit_      = nullptr;

    QToolButton* remove_button_ = nullptr;
    QToolButton* update_button_ = nullptr;
    QToolButton* edit_button_   = nullptr;

    ItemState item_state_ = ItemState::Ready;
    bool      inside_     = false;
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

private:
    void createUI();
    void removeItem(ViewPresets::Key key);

    QLineEdit*   filter_edit_ = nullptr;
    QVBoxLayout* item_layout_ = nullptr;

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

    View* view_ = nullptr;

    ViewPresetItemListWidget* preset_list_   = nullptr;
    QPushButton*              show_button_   = nullptr;
    QToolButton*              add_button_    = nullptr;
};
