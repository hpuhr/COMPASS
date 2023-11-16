
#include "viewpresetwidget.h"
#include "compass.h"
#include "viewmanager.h"
#include "view.h"
#include "files.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include <QToolButton>
#include <QLabel>
#include <QTextEdit>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QApplication>
#include <QCheckBox>
#include <QFrame>
#include <QScrollArea>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QWidgetAction>
#include <QResizeEvent>
#include <QGuiApplication>
#include <QScreen>

/********************************************************************************************
 * ViewPresetItemWidget
 ********************************************************************************************/

/**
*/
ViewPresetItemWidget::ViewPresetItemWidget(const ViewPresets::Preset* preset, QWidget* parent)
:   QFrame (parent)
,   preset_(preset)
{
    assert(preset_);

    createUI();
    updateContents();
}

/**
*/
void ViewPresetItemWidget::createUI()
{
    QHBoxLayout* main_layout = new QHBoxLayout;
    main_layout->setMargin(0);
    main_layout->setSpacing(0);
    setLayout(main_layout);

    //preview
    {
        QFrame* preview_frame = new QFrame;
        preview_frame->setFrameShape(QFrame::NoFrame);
        preview_frame->setFrameShadow(QFrame::Plain);
        preview_frame->setBackgroundRole(QPalette::Dark);

        QVBoxLayout* frame_layout = new QVBoxLayout;
        frame_layout->setMargin(5);
        frame_layout->setSpacing(0);
        preview_frame->setLayout(frame_layout);

        preview_label_ = new QLabel;
        preview_label_->setFixedSize(ViewPresets::PreviewMaxSize, ViewPresets::PreviewMaxSize);

        QFont f = preview_label_->font();
        f.setItalic(true);
        f.setPointSize(f.pointSize() - 1);
        preview_label_->setFont(f);

        frame_layout->addWidget(preview_label_);
        frame_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

        main_layout->addWidget(preview_frame);
    }

    //info widget
    {
        QWidget* info_widget = new QWidget;
        QVBoxLayout* info_layout = new QVBoxLayout;
        info_layout->setSpacing(2);
        info_widget->setLayout(info_layout);

        QHBoxLayout* layout_category = new QHBoxLayout;
        layout_category->setMargin(1);
        layout_category->setSpacing(1);
        info_layout->addLayout(layout_category);

        //category
        {
            category_label_ = new QLabel;
            category_label_->setWordWrap(true);
            category_label_->setAlignment(Qt::AlignTop);

            QFont f = category_label_->font();
            f.setBold(true);
            f.setPointSize(f.pointSize() - 2);
            category_label_->setFont(f);

            layout_category->addWidget(category_label_);
        }

        remove_button_ = new QToolButton;
        remove_button_->setIcon(QIcon(Utils::Files::getIconFilepath("delete.png").c_str()));
        remove_button_->setToolTip("Remove preset");

        edit_button_ = new QToolButton;
        edit_button_->setIcon(QIcon(Utils::Files::getIconFilepath("edit.png").c_str()));
        edit_button_->setToolTip("Modify preset");

        apply_button_ = new QToolButton;
        apply_button_->setIcon(QIcon(Utils::Files::getIconFilepath("play.png").c_str()));
        apply_button_->setToolTip("Apply preset");

        layout_category->addWidget(remove_button_);
        layout_category->addWidget(edit_button_);
        layout_category->addWidget(apply_button_);

        QHBoxLayout* layout_header = new QHBoxLayout;
        layout_header->setMargin(1);
        layout_header->setSpacing(1);
        info_layout->addLayout(layout_header);

        //name
        {
            name_label_ = new QLabel;
            name_label_->setAlignment(Qt::AlignTop);
            name_label_->setWordWrap(true);

            QFont f = name_label_->font();
            f.setBold(true);
            name_label_->setFont(f);

            layout_header->addWidget(name_label_);
        }

        QHBoxLayout* layout_descr = new QHBoxLayout;
        layout_descr->setMargin(1);
        layout_descr->setSpacing(1);
        info_layout->addLayout(layout_descr);

        //description
        {
            description_label_ = new QLabel;
            description_label_->setWordWrap(true);
            description_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            description_label_->setAlignment(Qt::AlignTop);

            layout_descr->addWidget(description_label_);
        }

        main_layout->addWidget(info_widget);
    }

    connect(remove_button_, &QToolButton::pressed, [ this ] () { emit removePresetRequested(this->preset_->key()); });
    connect(edit_button_  , &QToolButton::pressed, [ this ] () { emit modifyPresetRequested(this->preset_->key()); });
    connect(apply_button_ , &QToolButton::pressed, [ this ] () { emit  applyPresetRequested(this->preset_->key()); });
}

/**
*/
void ViewPresetItemWidget::updateContents()
{
    category_label_->setVisible(!preset_->category.empty());
    category_label_->setText(QString::fromStdString(preset_->category));

    name_label_->setText(QString::fromStdString(preset_->name));

    description_label_->setText(QString::fromStdString(preset_->description));

    preview_label_->setText(preset_->preview.isNull() ? "No Preview Available" : "");
    preview_label_->setPixmap(preset_->preview.isNull() ? QPixmap() : QPixmap::fromImage(preset_->preview));
}

/**
*/
void ViewPresetItemWidget::enterEvent(QEvent* event)
{
    if (!inside_)
    {
        inside_ = true;
        this->setBackgroundRole(QPalette::ColorRole::Highlight);
        this->setAutoFillBackground(true);
        setFrameShape(QFrame::Shape::Box);
        setFrameShadow(QFrame::Shadow::Plain);
    }
}

/**
*/
void ViewPresetItemWidget::leaveEvent(QEvent* event)
{
    if (inside_)
    {
        inside_ = false;
        this->setBackgroundRole(QPalette::ColorRole::Background);
        setFrameShape(QFrame::Shape::NoFrame);
    }
}

/********************************************************************************************
 * ViewPresetItemListWidget
 ********************************************************************************************/

/**
*/
ViewPresetItemListWidget::ViewPresetItemListWidget(QWidget* parent)
{
    createUI();
}

/**
*/
void ViewPresetItemListWidget::createUI()
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    QWidget* widget = new QWidget;
    QVBoxLayout* widget_layout = new QVBoxLayout;
    widget_layout->setMargin(0);
    widget_layout->setSpacing(0);
    widget->setLayout(widget_layout);

    item_layout_ = new QVBoxLayout;
    item_layout_->setMargin(0);
    item_layout_->setSpacing(2);

    widget_layout->addLayout(item_layout_);
    widget_layout->addStretch(1);

    QScrollArea* scroll_area = new QScrollArea;
    scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scroll_area->setWidgetResizable(true);

    scroll_area->setWidget(widget);

    layout->addWidget(scroll_area);

    scroll_area->show();
    widget->show();
}

/**
*/
void ViewPresetItemListWidget::clear()
{
    for (auto item : items_)
    {
        item_layout_->removeWidget(item);
        delete item;
    }
    items_.clear();
}

/**
*/
void ViewPresetItemListWidget::addItem(ViewPresetItemWidget* item)
{
    item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    items_.push_back(item);
    item_layout_->addWidget(item);

    connect(item, &ViewPresetItemWidget::removePresetRequested, this, &ViewPresetItemListWidget::removeItem);
    connect(item, &ViewPresetItemWidget::modifyPresetRequested, this, &ViewPresetItemListWidget::modifyItem);
    connect(item, &ViewPresetItemWidget::applyPresetRequested , this, &ViewPresetItemListWidget::apply);
}

/**
*/
void ViewPresetItemListWidget::removeItem(ViewPresets::Key key)
{
    auto it = std::find_if(items_.begin(), items_.end(), [ & ] (ViewPresetItemWidget* item) { return item->getPreset()->key() == key; });
    assert(it != items_.end());

    size_t idx = it - items_.begin();

    item_layout_->removeWidget(items_[ idx ]);
    delete items_[ idx ];

    items_.erase(items_.begin() + idx);

    emit removed(key);
}

/**
*/
void ViewPresetItemListWidget::modifyItem(ViewPresets::Key key)
{
    auto it = std::find_if(items_.begin(), items_.end(), [ & ] (ViewPresetItemWidget* item) { return item->getPreset()->key() == key; });
    assert(it != items_.end());

    size_t idx = it - items_.begin();

    emit modify(key);
    QApplication::processEvents();

    items_[ idx ]->updateContents();
}

/********************************************************************************************
 * ViewPresetWidget
 ********************************************************************************************/

/**
*/
ViewPresetWidget::ViewPresetWidget(View* view, QWidget* parent)
:   QWidget(parent)
,   view_  (view  )
{
    assert(view_);

    createUI();
    refill();
}

/**
*/
void ViewPresetWidget::createUI()
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setSpacing(1);
    setLayout(layout);

    QHBoxLayout* layout_h = new QHBoxLayout;
    layout_h->setSpacing(1);
    layout_h->setMargin(0);
    layout->addLayout(layout_h);

    preset_list_ = new ViewPresetItemListWidget(this);

    show_button_ = new QPushButton("Show Presets");
    layout_h->addWidget(show_button_);

    add_button_ = new QToolButton;
    add_button_->setIcon(QIcon(Utils::Files::getIconFilepath("crosshair_fat.png").c_str()));
    add_button_->setToolTip("Add new preset");
    layout_h->addWidget(add_button_);

    layout_h->addStretch(1);

    //layout->addWidget(preset_list_);

    QMenu* menu = new QMenu;
    auto action = new QWidgetAction(this);
    action->setDefaultWidget(preset_list_);
    menu->addAction(action);

    show_button_->setMenu(menu);

    connect(add_button_, &QToolButton::pressed, this, &ViewPresetWidget::addPreset);

    connect(preset_list_, &ViewPresetItemListWidget::removed, this, &ViewPresetWidget::removePreset);
    connect(preset_list_, &ViewPresetItemListWidget::modify, this, &ViewPresetWidget::modifyPreset);
    connect(preset_list_, &ViewPresetItemListWidget::apply, this, &ViewPresetWidget::applyPreset);
}

/**
*/
void ViewPresetWidget::refill()
{
    preset_list_->blockSignals(true);

    preset_list_->clear();

    const auto& presets    = COMPASS::instance().viewManager().viewPresets();
    const auto& preset_map = presets.presets();

    auto keys = presets.keysFor(view_);

    for (const auto& key : keys)
    {
        const auto& preset = preset_map.at(key);

        ViewPresetItemWidget* widget = new ViewPresetItemWidget(&preset);
        widget->updateContents();

        preset_list_->addItem(widget);
    }

    preset_list_->blockSignals(false);
    
    auto screen_size = QGuiApplication::primaryScreen()->size();
    preset_list_->setMinimumSize(screen_size.height() * 0.3, screen_size.height() * 0.6);
}

/**
*/
void ViewPresetWidget::addPreset()
{
    configurePreset(nullptr);
}

/**
*/
void ViewPresetWidget::removePreset(ViewPresets::Key key)
{
    auto& presets = COMPASS::instance().viewManager().viewPresets();
    presets.removePreset(view_, key.second);
}

/**
*/
void ViewPresetWidget::modifyPreset(ViewPresets::Key key)
{
    auto& presets = COMPASS::instance().viewManager().viewPresets().presets();
    assert(presets.count(key) > 0);

    configurePreset(&presets.at(key));
}

/**
*/
void ViewPresetWidget::applyPreset(ViewPresets::Key key)
{
    auto ret = QMessageBox::question(this, "Apply Preset", "Apply preset '" + QString::fromStdString(key.second) + "'?", QMessageBox::Yes, QMessageBox::No);
    if (ret == QMessageBox::No)
        return;

    const auto& preset = COMPASS::instance().viewManager().viewPresets().presets().at(key);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    //reconfigure view using the preset's view config
    view_->reconfigure(preset.view_config);

    QApplication::restoreOverrideCursor();
}

/**
*/
void ViewPresetWidget::configurePreset(const ViewPresets::Preset* preset)
{
    QDialog dlg;
    dlg.setWindowTitle(preset ? "Modify preset" : "Create new preset");

    QVBoxLayout* main_layout = new QVBoxLayout;
    dlg.setLayout(main_layout);

    QGridLayout* layout = new QGridLayout;
    main_layout->addLayout(layout);

    auto name_label  = new QLabel("Name: ");
    name_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto cat_label   = new QLabel("Category: ");
    cat_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto descr_label = new QLabel("Description: ");
    descr_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    descr_label->setAlignment(Qt::AlignTop);

    layout->addWidget( name_label, 0, 0);
    layout->addWidget(  cat_label, 1, 0);
    layout->addWidget(descr_label, 2, 0);

    auto name_edit  = new QLineEdit;
    auto cat_edit   = new QLineEdit;

    auto descr_edit = new QTextEdit;
    descr_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    descr_edit->setAcceptRichText(false);

    if (preset)
    {
        name_edit->setText(QString::fromStdString(preset->name));
        cat_edit->setText(QString::fromStdString(preset->category));
        descr_edit->setText(QString::fromStdString(preset->description));

        name_edit->setEnabled(false);
    }

    layout->addWidget( name_edit, 0, 1);
    layout->addWidget(  cat_edit, 1, 1);
    layout->addWidget(descr_edit, 2, 1);

    QCheckBox* update_box = new QCheckBox("Update view configuration");
    update_box->setChecked(false);
    update_box->setVisible(preset != nullptr);
    layout->addWidget(update_box, 3, 1);

    QHBoxLayout* button_layout = new QHBoxLayout;
    main_layout->addLayout(button_layout);

    QPushButton* ok_button     = new QPushButton("Ok");
    QPushButton* cancel_button = new QPushButton("Cancel");

    button_layout->addStretch(1);
    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    auto& presets = COMPASS::instance().viewManager().viewPresets();

    auto ok_callback = [ & ] ()
    {
        if (name_edit->text().isEmpty())
        {
            QMessageBox::critical(this, "Error", "Please set a valid preset name.");
            return;
        }
        if (!preset && presets.hasPreset(view_, name_edit->text().toStdString()))
        {
            QMessageBox::critical(this, "Error", "A preset of this name already exists, please select a unique name.");
            return;
        }
        dlg.accept();
    };

    auto ok_button_cb = [ & ] ()
    {
        ok_button->setText(!preset ? "Create" : (update_box->isChecked() ? "Update" : "Save"));
    };

    ok_button_cb();

    connect(ok_button, &QPushButton::pressed, ok_callback);
    connect(cancel_button, &QPushButton::pressed, &dlg, &QDialog::reject);
    connect(update_box, &QCheckBox::toggled, ok_button_cb);

    if (dlg.exec() == QDialog::Rejected)
        return;

    std::string name           = name_edit->text().toStdString();
    std::string category       = cat_edit->text().toStdString();
    std::string description    = descr_edit->toPlainText().toStdString();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    bool ok = preset ? presets.updatePreset(view_, name, category, description, update_box->isChecked(), update_box->isChecked()) :
                       presets.createPreset(view_, name, category, description, true);

    QApplication::restoreOverrideCursor();

    if (!ok)
    {
        QMessageBox::critical(this, "Error", preset ? "Modifying preset failed." : "Creating new preset failed.");
        return;
    }

    if (!preset)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);

        refill();

        QApplication::restoreOverrideCursor();
    }
}


// /**
// */
// void ViewPresetWidget::refill()
// {
//     preset_combo_->blockSignals(true);
//     preset_combo_->clear();

//     const auto& presets    = COMPASS::instance().viewManager().viewPresets();
//     const auto& preset_map = presets.presets();

//     current_keys_ = presets.keysFor(view_);

//     //std::cout << "obtained " << current_keys_.size() << " preset key(s) for view " << view_->classId() << std::endl;

//     for (const auto& key : current_keys_)
//         preset_combo_->addItem(QString::fromStdString(preset_map.at(key).name));

//     if (preset_combo_->count() > 0)
//         preset_combo_->setCurrentIndex(0);

//     preset_combo_->blockSignals(false);

//     //update display for current preset
//     currentPresetChanged();
// }

// /**
// */
// void ViewPresetWidget::currentPresetChanged()
// {
//     preview_label_->setPixmap(QPixmap());
//     preview_label_->setText("No Preview");

//     descr_edit_->setText("No Description Available");

//     int idx = preset_combo_->currentIndex();

//     update_button_->setEnabled(idx >= 0);
//     rem_button_->setEnabled(idx >= 0);
//     apply_button_->setEnabled(idx >= 0);

//     if (idx < 0)
//         return;

//     auto key = current_keys_.at(idx);

//     const auto& preset = COMPASS::instance().viewManager().viewPresets().presets().at(key);

//     if (!preset.preview.isNull())
//         preview_label_->setPixmap(QPixmap::fromImage(preset.preview));

//     if (!preset.description.empty())
//         descr_edit_->setText(QString::fromStdString(preset.description));
// }

// 

// /**
// */
// void ViewPresetWidget::addPreset()
// {
//     configurePreset(nullptr);
// }

// /**
// */
// void ViewPresetWidget::updatePreset()
// {
//     int idx = preset_combo_->currentIndex();
//     if (idx < 0)
//         return;

//     auto key = current_keys_.at(idx);

//     const auto& preset = COMPASS::instance().viewManager().viewPresets().presets().at(key);

//     configurePreset(&preset);
// }

// /**
// */
// void ViewPresetWidget::removeCurrentPreset()
// {
//     auto name = preset_combo_->currentText().toStdString();
//     if (name.empty())
//         return;

//     auto& presets = COMPASS::instance().viewManager().viewPresets();
//     presets.removePreset(view_, name);

//     QApplication::setOverrideCursor(Qt::WaitCursor);

//     refill();

//     QApplication::restoreOverrideCursor();
// }

// /**
// */
// void ViewPresetWidget::applyCurrentPreset()
// {
//     int idx = preset_combo_->currentIndex();
//     if (idx < 0)
//         return;

//     auto key = current_keys_.at(idx);

//     const auto& preset = COMPASS::instance().viewManager().viewPresets().presets().at(key);

//     QApplication::setOverrideCursor(Qt::WaitCursor);

//     //reconfigure view using the preset's view config
//     view_->reconfigure(preset.view_config);

//     QApplication::restoreOverrideCursor();
// }
