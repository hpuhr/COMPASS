
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
ViewPresetItemWidget::ViewPresetItemWidget(const ViewPresets::Key& key, 
                                           View* view,
                                           QWidget* parent)
:   QWidget(parent)
,   key_   (key   )
,   view_  (view  )
{
    assert(view_  );
    
    auto& presets = COMPASS::instance().viewManager().viewPresets();
    assert(presets.hasPreset(key));

    preset_ = &presets.presets().at(key);

    createUI();
    updateContents();
    
    setItemState(ItemState::Ready);
}

/**
*/
void ViewPresetItemWidget::createUI()
{
    QHBoxLayout* main_layout = new QHBoxLayout;
    main_layout->setMargin(0);
    main_layout->setSpacing(0);
    setLayout(main_layout);

    QWidget* content_widget = new QWidget(this);
    content_widget->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* content_widget_layout = new QHBoxLayout;
    content_widget_layout->setMargin(0);
    content_widget_layout->setSpacing(0);
    content_widget->setLayout(content_widget_layout);

    QWidget* decoration_widget = new QWidget(this);
    decoration_widget->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* decoration_widget_layout = new QVBoxLayout;
    decoration_widget_layout->setMargin(5);
    decoration_widget_layout->setSpacing(0);
    decoration_widget->setLayout(decoration_widget_layout);

    main_layout->addWidget(content_widget);
    main_layout->addWidget(decoration_widget);

    //preview
    {
        QFrame* preview_frame = new QFrame;
        preview_frame->setFrameShape(QFrame::NoFrame);
        preview_frame->setFrameShadow(QFrame::Plain);
        preview_frame->setBackgroundRole(QPalette::Dark);
        preview_frame->setContentsMargins(0, 0, 0, 0);

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

        content_widget_layout->addWidget(preview_frame);
    }

    //info widget
    {
        QWidget* info_widget = new QWidget;
        info_widget->setContentsMargins(0, 0, 0, 0);
        
        QVBoxLayout* info_layout = new QVBoxLayout;
        info_layout->setSpacing(1);
        info_layout->setMargin(5);
        info_widget->setLayout(info_layout);

        QHBoxLayout* layout_category = new QHBoxLayout;
        layout_category->setSpacing(1);
        layout_category->setMargin(1);
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

        QHBoxLayout* layout_header = new QHBoxLayout;
        layout_header->setSpacing(1);
        layout_header->setMargin(1);
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

            description_edit_ = new QTextEdit;
            description_edit_->setAcceptRichText(false);

            layout_descr->addWidget(description_label_);
            layout_descr->addWidget(description_edit_ );

            QVBoxLayout* layout_edit_button = new QVBoxLayout;
            layout_edit_button->setMargin(0);
            layout_edit_button->setSpacing(0);

            edit_button_ = new QToolButton;
            edit_button_->setIcon(QIcon(Utils::Files::getIconFilepath("edit_old.png").c_str()));
            edit_button_->setToolTip("Edit description");
            edit_button_->setIconSize(QSize(14, 14));
            edit_button_->setAutoRaise(true);
            edit_button_->setCheckable(true);

            edit_button_->setVisible(false);

            connect(edit_button_, &QToolButton::toggled, this, &ViewPresetItemWidget::editDescriptionToggled);

            layout_edit_button->addWidget(edit_button_);
            layout_edit_button->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

            layout_descr->addLayout(layout_edit_button);
        }

        content_widget_layout->addWidget(info_widget);
    }

    //decorator buttons
    remove_button_ = new QToolButton;
    remove_button_->setIcon(QIcon(Utils::Files::getIconFilepath("delete.png").c_str()));
    remove_button_->setToolTip("Remove preset");
    remove_button_->setAutoRaise(true);

    update_button_ = new QToolButton;
    update_button_->setIcon(QIcon(Utils::Files::getIconFilepath("refresh.png").c_str()));
    update_button_->setToolTip("Update to current view config");
    update_button_->setAutoRaise(true);

    decoration_widget_layout->addWidget(remove_button_);
    decoration_widget_layout->addWidget(update_button_);
    decoration_widget_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

    connect(remove_button_, &QToolButton::pressed, this, &ViewPresetItemWidget::removeButtonPressed);
    connect(update_button_, &QToolButton::pressed, this, &ViewPresetItemWidget::updateButtonPressed);
}

/**
*/
void ViewPresetItemWidget::setItemState(ItemState item_state)
{
    item_state_ = item_state;

    //update stuff on state change
    updateActivity();
    updateCursors();
    updateDescriptionEdit();
}

/**
*/
void ViewPresetItemWidget::updateCursors()
{
    setCursor(item_state_ == ItemState::Edit ? Qt::CursorShape::ArrowCursor : Qt::CursorShape::PointingHandCursor);

    remove_button_->setCursor(Qt::CursorShape::ArrowCursor);
    edit_button_->setCursor(Qt::CursorShape::ArrowCursor);
}

/**
*/
void ViewPresetItemWidget::updateActivity()
{
    remove_button_->setEnabled(item_state_ == ItemState::Ready);
    update_button_->setEnabled(item_state_ == ItemState::Ready);
}

/**
*/
void ViewPresetItemWidget::updateDescriptionEdit()
{
    bool edit_mode = item_state_ == ItemState::Edit;

    //need to deactivate edit button?
    if (!edit_mode && edit_button_->isChecked())
    {
        edit_button_->blockSignals(true);
        edit_button_->setChecked(false);
        edit_button_->blockSignals(false);
    }

    if (edit_mode)
    {
        description_edit_->setFixedHeight(std::max(100, description_label_->height()));
        description_edit_->setPlainText(description_label_->text());
    }

    description_label_->setVisible(!edit_mode);
    description_edit_->setVisible(edit_mode);
}

/**
*/
void ViewPresetItemWidget::updateContents()
{
    //revert any active editing on content update
    setItemState(ItemState::Ready);

    category_label_->setVisible(!preset_->category.empty());
    category_label_->setText(QString::fromStdString(preset_->category));

    name_label_->setText(QString::fromStdString(preset_->name));

    description_label_->setText(QString::fromStdString(preset_->description));

    preview_label_->setText(preset_->preview.isNull() ? "No Preview Available" : "");
    preview_label_->setPixmap(preset_->preview.isNull() ? QPixmap() : QPixmap::fromImage(preset_->preview));
}

/**
*/
void ViewPresetItemWidget::editDescriptionToggled(bool edit)
{
    if (!edit)
    {
        description_label_->setText(description_edit_->toPlainText());

        ViewPresets::Preset p = *preset_;
        p.description = description_label_->text().toStdString();

        updatePreset(p);
    }

    //set item state
    setItemState(edit ? ItemState::Edit : ItemState::Ready);
}

/**
*/
void ViewPresetItemWidget::removeButtonPressed()
{
    auto& presets = COMPASS::instance().viewManager().viewPresets();
    presets.removePreset(preset_->key());

    emit removePresetRequested(preset_->key());
}

/**
*/
void ViewPresetItemWidget::updateButtonPressed()
{
    ViewPresets::Preset p = *preset_;
    ViewPresets::updatePresetConfig(p, view_, true);

    updatePreset(p);
    updateContents();
}
    
/**
*/
void ViewPresetItemWidget::applyPreset()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    //reconfigure view using the preset's view config
    view_->reconfigure(preset_->view_config);

    //update item contents (preview image etc.)
    updateContents();

    QApplication::restoreOverrideCursor();
}

/**
*/
bool ViewPresetItemWidget::updatePreset(const ViewPresets::Preset& preset)
{
    //backup current preset
    ViewPresets::Preset p_backup = *preset_;

    //set new preset
    *preset_ = preset;

    //write preset
    const auto& presets = COMPASS::instance().viewManager().viewPresets();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    bool ok = presets.writePreset(key_);

    QApplication::restoreOverrideCursor();

    if (!ok)
    {
        QMessageBox::critical(this, "Error", "Writing preset '" + QString::fromStdString(key_.second) + "' to disk failed.");

        //revert to old version and update
        *preset_ = p_backup;

        updateContents();
    }

    return ok;
}

/**
*/
void ViewPresetItemWidget::mousePressEvent(QMouseEvent *event)
{
    //allow mouse clicks only when item is ready
    if (item_state_ == ItemState::Ready)
    {
        //handle parents mouse event
        QWidget::mousePressEvent(event);

        if (event->button() == Qt::LeftButton)
        {
            applyPreset();
        }
    }
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
    }
}

/**
*/
void ViewPresetItemWidget::hideEvent(QHideEvent *event)
{
    //revert any active editing on hide
    setItemState(ItemState::Ready);
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

    filter_edit_ = new QLineEdit;
    filter_edit_->setPlaceholderText("Filter for preset name...");
    layout->addWidget(filter_edit_);

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
}

/**
*/
void ViewPresetItemListWidget::removeItem(ViewPresets::Key key)
{
    //get item
    auto it = std::find_if(items_.begin(), items_.end(), [ & ] (ViewPresetItemWidget* item) { return item->getPreset()->key() == key; });
    assert(it != items_.end());

    //remove from layout and delete item
    item_layout_->removeWidget(*it);
    delete *it;

    //remove from items
    items_.erase(it);
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
}

/**
*/
void ViewPresetWidget::refill()
{
    preset_list_->blockSignals(true);

    preset_list_->clear();

    const auto& presets = COMPASS::instance().viewManager().viewPresets();

    auto keys = presets.keysFor(view_);

    for (const auto& key : keys)
    {
        ViewPresetItemWidget* widget = new ViewPresetItemWidget(key, view_, preset_list_);
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
    QDialog dlg;
    dlg.setWindowTitle("Create new preset");

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

    layout->addWidget( name_edit, 0, 1);
    layout->addWidget(  cat_edit, 1, 1);
    layout->addWidget(descr_edit, 2, 1);

    QHBoxLayout* button_layout = new QHBoxLayout;
    main_layout->addLayout(button_layout);

    QPushButton* ok_button     = new QPushButton("Create");
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
        if (presets.hasPreset(view_, name_edit->text().toStdString()))
        {
            QMessageBox::critical(this, "Error", "A preset of this name already exists, please select a unique name.");
            return;
        }
        dlg.accept();
    };

    connect(ok_button, &QPushButton::pressed, ok_callback);
    connect(cancel_button, &QPushButton::pressed, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Rejected)
        return;

    std::string name        = name_edit->text().toStdString();
    std::string category    = cat_edit->text().toStdString();
    std::string description = descr_edit->toPlainText().toStdString();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    bool ok = presets.createPreset(view_, name, category, description, true);

    QApplication::restoreOverrideCursor();

    if (!ok)
    {
        QMessageBox::critical(this, "Error", "Creating new preset failed.");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    refill();

    QApplication::restoreOverrideCursor();
}
