
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
 * ViewPresetEditDialog
 ********************************************************************************************/

/**
*/
ViewPresetEditDialog::ViewPresetEditDialog(View* view, 
                                           ViewPresets::Preset* preset, 
                                           QWidget* parent)
:   QDialog(parent)
,   view_  (view  )
,   preset_(preset)
,   mode_  (preset == nullptr ? Mode::Create : Mode::Edit)
{
    assert(view_);

    createUI();
    configureUI();
}

/**
*/
void ViewPresetEditDialog::createUI()
{
    QVBoxLayout* main_layout = new QVBoxLayout;
    setLayout(main_layout);

    QGridLayout* layout = new QGridLayout;
    main_layout->addLayout(layout);

    QHBoxLayout* preview_layout = new QHBoxLayout;

    preview_label_ = new QLabel;
    preview_label_->setFixedSize(ViewPresets::PreviewMaxSize, ViewPresets::PreviewMaxSize);
    preview_layout->addWidget(preview_label_);
    preview_layout->addStretch(1);

    auto name_label  = new QLabel("Name: ");
    name_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto cat_label   = new QLabel("Category: ");
    cat_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto descr_label = new QLabel("Description: ");
    descr_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    descr_label->setAlignment(Qt::AlignTop);

    auto preview_label = new QLabel("Preview: ");
    preview_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    preview_label->setAlignment(Qt::AlignTop);

    
    layout->addWidget(name_label   , 0, 0);
    layout->addWidget(cat_label    , 1, 0);
    layout->addWidget(descr_label  , 2, 0);
    layout->addWidget(preview_label, 3, 0);

    name_edit_        = new QLineEdit;
    category_edit_    = new QLineEdit;
    description_edit_ = new QTextEdit;

    description_edit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    description_edit_->setAcceptRichText(false);

    update_button_ = new QPushButton("Update view config");
    update_button_->setIcon(QIcon(Utils::Files::getIconFilepath("refresh.png").c_str()));

    config_label_ = new QLabel("");

    layout->addWidget(name_edit_       , 0, 1);
    layout->addWidget(category_edit_   , 1, 1);
    layout->addWidget(description_edit_, 2, 1);
    layout->addLayout(preview_layout   , 3, 1);
    layout->addWidget(update_button_   , 4, 1);
    layout->addWidget(config_label_    , 5, 1);

    main_layout->addStretch(1);

    QHBoxLayout* button_layout = new QHBoxLayout;
    main_layout->addLayout(button_layout);

    create_button_ = new QPushButton("Create");
    copy_button_   = new QPushButton("Copy");
    save_button_   = new QPushButton("Save");
    
    auto cancel_button = new QPushButton("Cancel");

    button_layout->addWidget(cancel_button );
    button_layout->addStretch(1);
    button_layout->addWidget(create_button_);
    button_layout->addWidget(copy_button_  );
    button_layout->addWidget(save_button_  );
    
    connect(create_button_, &QPushButton::pressed, this, &ViewPresetEditDialog::createPreset);
    connect(copy_button_  , &QPushButton::pressed, this, &ViewPresetEditDialog::copyPreset  );
    connect(save_button_  , &QPushButton::pressed, this, &ViewPresetEditDialog::savePreset  );
    connect(cancel_button , &QPushButton::pressed, this, &ViewPresetEditDialog::reject      );

    connect(update_button_, &QPushButton::pressed, this, &ViewPresetEditDialog::updateConfigPressed);
}

/**
*/
void ViewPresetEditDialog::configureUI()
{
    bool create_mode = mode_ == Mode::Create;

    setWindowTitle(create_mode ? "Create New Preset" : "Edit Preset");

    create_button_->setVisible(create_mode);
    copy_button_->setVisible(!create_mode);
    save_button_->setVisible(!create_mode);
    update_button_->setVisible(!create_mode);

    if (create_mode)
    {
        updateConfig();
    }
    else
    {
        preset_backup_ = *preset_;
        preset_update_ = *preset_;

        name_edit_->setText(QString::fromStdString(preset_->name));
        category_edit_->setText(QString::fromStdString(preset_->category));
        description_edit_->setText(QString::fromStdString(preset_->description));
    }

    updatePreview();
}

/**
*/
void ViewPresetEditDialog::revert()
{
    assert(preset_);
    *preset_ = preset_backup_;
}

/**
*/
bool ViewPresetEditDialog::checkName()
{
    auto& presets = COMPASS::instance().viewManager().viewPresets();

    if (name_edit_->text().isEmpty())
    {
        QMessageBox::critical(this, "Error", "Please set a valid preset name.");
        return false;
    }

    if (presets.hasPreset(view_, name_edit_->text().toStdString()))
    {
        QMessageBox::critical(this, "Error", "A preset of this name already exists, please select a unique name.");
        return false;
    }

    return true;
}

/**
*/
void ViewPresetEditDialog::createPreset()
{
    if (!checkName())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    updateMetaData();
    bool ok = COMPASS::instance().viewManager().viewPresets().createPreset(preset_update_, view_);

    QApplication::restoreOverrideCursor();

    if (!ok)
    {
        QMessageBox::critical(this, "Error", "Creating new preset failed.");
        return;
    }

    accept();
}

/**
*/
void ViewPresetEditDialog::copyPreset()
{
    assert(preset_);

    if (!checkName())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    updateMetaData();
    bool ok = COMPASS::instance().viewManager().viewPresets().createPreset(preset_update_, nullptr);

    QApplication::restoreOverrideCursor();

    if (!ok)
    {
        QMessageBox::critical(this, "Error", "Saving as new preset failed.");
        return;
    }

    accept();
}

/**
*/
void ViewPresetEditDialog::savePreset()
{
    assert(preset_);

    //if the name changed we need to check its validity
    if (name_edit_->text().toStdString() != preset_backup_.name && !checkName())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    updateMetaData();
    bool ok = COMPASS::instance().viewManager().viewPresets().updatePreset(preset_->key(), preset_update_);

    QApplication::restoreOverrideCursor();

    if (!ok)
    {
        QMessageBox::critical(this, "Error", "Saving preset failed.");
        return;
    }

    accept();
}

/**
*/
void ViewPresetEditDialog::updateConfig()
{
    ViewPresets::updatePresetConfig(preset_update_, view_, true);
}

/**
*/
void ViewPresetEditDialog::updateMetaData()
{
    preset_update_.name        = name_edit_->text().toStdString();
    preset_update_.category    = category_edit_->text().toStdString();
    preset_update_.description = description_edit_->toPlainText().toStdString();
}

/**
*/
void ViewPresetEditDialog::updatePreview()
{
    preview_label_->setPixmap(QPixmap::fromImage(preset_update_.preview));
}

/**
*/
void ViewPresetEditDialog::updateConfigPressed()
{
    updateConfig();
    updatePreview();
    config_label_->setText("Configuration updated!");
}

/********************************************************************************************
 * ViewPresetItemWidget
 ********************************************************************************************/

/**
*/
ViewPresetItemWidget::ViewPresetItemWidget(const ViewPresets::Key& key, 
                                           View* view,
                                           QWidget* parent)
:   QWidget(parent)
,   view_  (view  )
{
    assert(view_);
    
    createUI();
    updateContents(key);
}

/**
*/
void ViewPresetItemWidget::createUI()
{
    setCursor(Qt::CursorShape::PointingHandCursor);

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

            layout_descr->addWidget(description_label_);
        }

        content_widget_layout->addWidget(info_widget);
    }

    //decorator buttons
    remove_button_ = new QToolButton;
    remove_button_->setIcon(QIcon(Utils::Files::getIconFilepath("delete.png").c_str()));
    remove_button_->setToolTip("Remove preset");
    remove_button_->setAutoRaise(true);

    edit_button_ = new QToolButton;
    edit_button_->setIcon(QIcon(Utils::Files::getIconFilepath("edit.png").c_str()));
    edit_button_->setToolTip("Edit preset");
    edit_button_->setAutoRaise(true);

    decoration_widget_layout->addWidget(remove_button_);
    decoration_widget_layout->addWidget(edit_button_);
    decoration_widget_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

    connect(remove_button_, &QToolButton::pressed, this, &ViewPresetItemWidget::removeButtonPressed);
    connect(edit_button_  , &QToolButton::pressed, this, &ViewPresetItemWidget::editButtonPressed  );

    remove_button_->setCursor(Qt::CursorShape::ArrowCursor);
    edit_button_->setCursor(Qt::CursorShape::ArrowCursor);
}

/**
*/
void ViewPresetItemWidget::updateContents(const ViewPresets::Key& key)
{
    auto& presets = COMPASS::instance().viewManager().viewPresets();
    assert(presets.hasPreset(key));

    key_    = key;
    preset_ = &presets.presets().at(key);

    updateContents();
}

/**
*/
void ViewPresetItemWidget::updateContents()
{
    category_label_->setVisible(!preset_->category.empty());
    category_label_->setText(QString::fromStdString(preset_->category));

    name_label_->setText(QString::fromStdString(preset_->name));

    description_label_->setText(QString::fromStdString(preset_->description));

    preview_label_->setText(preset_->preview.isNull() ? "No preview available" : "");
    preview_label_->setPixmap(preset_->preview.isNull() ? QPixmap() : QPixmap::fromImage(preset_->preview));
}

/**
*/
void ViewPresetItemWidget::removeButtonPressed()
{
    emit removePreset(key_);
}

/**
*/
void ViewPresetItemWidget::editButtonPressed()
{
    emit editPreset(key_);
}
    
/**
*/
void ViewPresetItemWidget::applyPreset()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    //reconfigure view using the preset's view config
    view_->reconfigure(preset_->view_config);

    QApplication::restoreOverrideCursor();
}

/**
*/
void ViewPresetItemWidget::mousePressEvent(QMouseEvent *event)
{
    //handle parents mouse event
    QWidget::mousePressEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        //item clicked => apply preset
        applyPreset();
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

/********************************************************************************************
 * ViewPresetItemListWidget
 ********************************************************************************************/

const double ViewPresetItemListWidget::WidgetWFraction = 0.4;
const double ViewPresetItemListWidget::WidgetHFraction = 0.6;

/**
*/
ViewPresetItemListWidget::ViewPresetItemListWidget(View* view,
                                                   QWidget* parent)
:   QWidget(parent)
,   view_  (view  )
{
    createUI();
    updateContents();

    auto& presets = COMPASS::instance().viewManager().viewPresets();

    //react on preset changes globally
    connect(&presets, &ViewPresets::presetAdded  , this, &ViewPresetItemListWidget::updateItem);
    connect(&presets, &ViewPresets::presetRemoved, this, &ViewPresetItemListWidget::removeItem);
    connect(&presets, &ViewPresets::presetRenamed, this, &ViewPresetItemListWidget::updateItem);
    connect(&presets, &ViewPresets::presetUpdated, this, &ViewPresetItemListWidget::updateItem);
}

/**
*/
void ViewPresetItemListWidget::createUI()
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    //layout->addSpacerItem(new QSpacerItem(1, 9, QSizePolicy::Fixed, QSizePolicy::Fixed));

    QHBoxLayout* add_layout = new QHBoxLayout;
    add_layout->setMargin(10);
    add_layout->setSpacing(10);
    layout->addLayout(add_layout);

    QPushButton* add_button = new QPushButton;
    add_button->setIcon(QIcon(Utils::Files::getIconFilepath("crosshair_fat.png").c_str()));
    add_button->setText("Add preset");
    add_layout->addWidget(add_button);

    //add_layout->addSpacerItem(new QSpacerItem(10, 1, QSizePolicy::Fixed, QSizePolicy::Fixed));

    filter_edit_ = new QLineEdit;
    filter_edit_->setPlaceholderText("Filter for preset name...");
    add_layout->addWidget(filter_edit_);

    //layout->addSpacerItem(new QSpacerItem(1, 9, QSizePolicy::Fixed, QSizePolicy::Fixed));

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

    connect(filter_edit_, &QLineEdit::textChanged, this, &ViewPresetItemListWidget::updateFilter);
    connect(add_button, &QToolButton::pressed, this, &ViewPresetItemListWidget::addPreset);
}

/**
*/
void ViewPresetItemListWidget::addPreset()
{
    //show creation dialog
    ViewPresetEditDialog dlg(view_, nullptr, this);
    dlg.exec();
}

/**
*/
void ViewPresetItemListWidget::clear()
{
    for (auto item : items_)
    {
        item_layout_->removeWidget(item);
        item->deleteLater(); //better to delete later
    }
    items_.clear();
}

/**
*/
void ViewPresetItemListWidget::editPreset(ViewPresets::Key key)
{
    auto& presets = COMPASS::instance().viewManager().viewPresets();
    assert(presets.hasPreset(key));

    //edit preset
    ViewPresetEditDialog dlg(view_, &presets.presets().at(key), this);
    if (dlg.exec() != QDialog::Accepted)
        return;
}

/**
*/
void ViewPresetItemListWidget::removePreset(ViewPresets::Key key)
{
    auto& presets = COMPASS::instance().viewManager().viewPresets();
    assert(presets.hasPreset(key));

    //remove preset
    presets.removePreset(key);
}

/**
*/
void ViewPresetItemListWidget::removeItem(ViewPresets::Key key)
{
    //key does not concern me?
    if (!ViewPresets::keyIsView(key, view_))
        return;

    //get item
    auto it = std::find_if(items_.begin(), items_.end(), [ & ] (ViewPresetItemWidget* item) { return item->key() == key; });
    assert(it != items_.end());

    //remove item from layout and delete
    item_layout_->removeWidget(*it);
    delete *it;

    //remove from items
    items_.erase(it);
}

/**
*/
void ViewPresetItemListWidget::updateItem(ViewPresets::Key key)
{
    //key does not concern me?
    if (!ViewPresets::keyIsView(key, view_))
        return;

    //just update complete content on item change
    updateContents();
}

/**
*/
void ViewPresetItemListWidget::updateContents()
{
    //refresh item widgets
    refill();
}

/**
*/
void ViewPresetItemListWidget::refill()
{
    blockSignals(true);

    //clear items
    clear();

    const auto& presets = COMPASS::instance().viewManager().viewPresets();

    //get preset keys for stored view type
    auto keys = presets.keysFor(view_);

    for (const auto& key : keys)
    {
        //add item
        ViewPresetItemWidget* item = new ViewPresetItemWidget(key, view_, this);
        item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        connect(item, &ViewPresetItemWidget::removePreset, this, &ViewPresetItemListWidget::removePreset);
        connect(item, &ViewPresetItemWidget::editPreset  , this, &ViewPresetItemListWidget::editPreset  );

        item_layout_->addWidget(item);
        items_.push_back(item);
    }

    //update visibility of items via filter
    updateFilter();

    blockSignals(false);
    
    updateMinSize();
}

namespace
{
    /**
     * Check if preset name is part of filter clause.
     */
    bool inFilter(const QString& filter, const ViewPresets::Preset* preset)
    {
        if (filter.isEmpty())
            return true;

        QString name = QString::fromStdString(preset->name);

        //check if name starts with filter
        if (name.toLower().startsWith(filter.toLower()))
            return true;

        return false;
    }
}

/**
*/
void ViewPresetItemListWidget::updateFilter()
{
    auto filter = filter_edit_->text();

    for (auto item : items_)
        item->setVisible(inFilter(filter, item->getPreset()));
}

/**
*/
void ViewPresetItemListWidget::updateMinSize()
{
    auto screen_size = QGuiApplication::primaryScreen()->size();
    auto min_size    = std::min(screen_size.width(), screen_size.height());

    //update minimum widget size to a fraction of the screen size
    setMinimumSize(min_size * WidgetWFraction, screen_size.height() * WidgetHFraction);
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

    preset_list_ = new ViewPresetItemListWidget(view_, this);

    show_button_ = new QPushButton("Show Presets");
    layout_h->addWidget(show_button_);

    layout_h->addStretch(1);

    //layout->addWidget(preset_list_);

    QMenu* menu = new QMenu;
    auto action = new QWidgetAction(this);
    action->setDefaultWidget(preset_list_);
    menu->addAction(action);

    show_button_->setMenu(menu);
}
