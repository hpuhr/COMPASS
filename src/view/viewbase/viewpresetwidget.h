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
#include "json.h"

#include <QWidget>
#include <QDialog>
#include <QImage>

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
class ViewPresetEditDialog : public QDialog
{
public:
    enum class Mode
    {   
        Create = 0,
        Edit
    };

    ViewPresetEditDialog(View* view, ViewPresets::Preset* preset, QWidget* parent = nullptr);
    virtual ~ViewPresetEditDialog() = default;

private:
    void createUI();
    void configureUI();
    void updatePreview();
    void updateConfig();
    void updateMetaData();

    void revert();

    bool checkName();

    void createPreset();
    void savePreset();
    void copyPreset();
    
    void updateConfigPressed();

    View*                view_   = nullptr;
    ViewPresets::Preset* preset_ = nullptr;

    QLineEdit* name_edit_        = nullptr;
    QLineEdit* category_edit_    = nullptr;
    QTextEdit* description_edit_ = nullptr;
    QLabel*    preview_label_    = nullptr;
    QLabel*    config_label_     = nullptr;

    QPushButton* create_button_ = nullptr;
    QPushButton* save_button_   = nullptr;
    QPushButton* copy_button_   = nullptr;
    QPushButton* update_button_ = nullptr;

    ViewPresets::Preset preset_backup_;
    ViewPresets::Preset preset_update_;

    Mode mode_ = Mode::Create;
};

/**
*/
class ViewPresetItemWidget : public QWidget
{
    Q_OBJECT
public:
    ViewPresetItemWidget(const ViewPresets::Key& key,
                         View* view,
                         QWidget* parent = nullptr);
    virtual ~ViewPresetItemWidget() = default;

    const ViewPresets::Preset* getPreset() const { return preset_; }
    const ViewPresets::Key& key() const { return key_; }

    void updateContents(const ViewPresets::Key& key);
    void updateContents();

signals:
    void removePreset(ViewPresets::Key key);
    void editPreset(ViewPresets::Key key);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void createUI();

    void removeButtonPressed();
    void editButtonPressed();

    void applyPreset();
    
    ViewPresets::Key           key_;
    const ViewPresets::Preset* preset_ = nullptr;
    View*                      view_   = nullptr;

    QLabel*      category_label_    = nullptr;
    QLabel*      name_label_        = nullptr;
    QLabel*      preview_label_     = nullptr;
    QLabel*      description_label_ = nullptr;
    QToolButton* remove_button_     = nullptr;
    QToolButton* edit_button_       = nullptr;

    bool inside_ = false;
};

/**
*/
class ViewPresetItemListWidget : public QWidget
{
    Q_OBJECT
public:
    ViewPresetItemListWidget(View* view,
                             QWidget* parent = nullptr);
    virtual ~ViewPresetItemListWidget() = default;

    void updateContents();
    void addPreset();

private:
    void createUI();
    void clear();
    void refill();
    void updateFilter();

    void editPreset(ViewPresets::Key key);
    void removePreset(ViewPresets::Key key);
    
    void removeItem(ViewPresets::Key key);
    void updateItem(ViewPresets::Key key);

    View* view_ = nullptr;

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
