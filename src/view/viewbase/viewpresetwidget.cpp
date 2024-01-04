
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
                                           Mode mode, 
                                           QWidget* parent)
:   QDialog(parent)
,   view_  (view  )
,   preset_(preset)
,   mode_  (mode  )
{
    assert(view_);

    createUI();
    configureUI();
}

/**
*/
QString ViewPresetEditDialog::windowTitle(Mode mode) const
{
    if (mode_ == Mode::Copy)
        return "Copy Preset";
    else if (mode_ == Mode::Create)
        return "Create New Preset";
    else if (mode_ == Mode::Edit)
        return "Edit Preset";

    return "";
}

/**
*/
const QImage& ViewPresetEditDialog::previewImage(Mode mode) const
{
    if (mode_ == Mode::Copy)
    {
        assert(preset_);
        return preset_->preview;
    }
    else if (mode_ == Mode::Create)
    {
        return preset_new_.preview;
    }
    else if (mode_ == Mode::Edit)
    {
        assert(preset_);
        return preset_->preview;
    }

    return preset_->preview;
}

/**
*/
void ViewPresetEditDialog::createUI()
{
    QVBoxLayout* main_layout = new QVBoxLayout;
    setLayout(main_layout);

    QHBoxLayout* horz_layout = new QHBoxLayout;
    main_layout->addLayout(horz_layout);

    QVBoxLayout* preview_layout = new QVBoxLayout;
    horz_layout->addLayout(preview_layout);

    QFrame* line = new QFrame; 
    line->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Shadow::Plain);
    line->setFixedWidth(20);
    horz_layout->addWidget(line);

    QGridLayout* content_layout = new QGridLayout;
    horz_layout->addLayout(content_layout);

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

    content_layout->addWidget(name_label   , 0, 0);
    content_layout->addWidget(cat_label    , 1, 0);
    content_layout->addWidget(descr_label  , 2, 0);

    name_edit_        = new QLineEdit;
    category_edit_    = new QLineEdit;
    description_edit_ = new QTextEdit;

    description_edit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    description_edit_->setAcceptRichText(false);

    content_layout->addWidget(name_edit_       , 0, 1);
    content_layout->addWidget(category_edit_   , 1, 1);
    content_layout->addWidget(description_edit_, 2, 1);

    main_layout->addStretch(1);

    QHBoxLayout* button_layout = new QHBoxLayout;
    main_layout->addLayout(button_layout);

    auto ok_button     = new QPushButton("Ok");
    auto cancel_button = new QPushButton("Cancel");

    button_layout->addWidget(cancel_button );
    button_layout->addStretch(1);
    button_layout->addWidget(ok_button);

    ok_button->setDefault(true);
    
    connect(ok_button    , &QPushButton::pressed, this, &ViewPresetEditDialog::apply );
    connect(cancel_button, &QPushButton::pressed, this, &ViewPresetEditDialog::reject);
}

/**
*/
void ViewPresetEditDialog::configureUI()
{
    setWindowTitle(windowTitle(mode_));

    if (mode_ == Mode::Copy)
    {
        assert(preset_);

        //fill in current metadata (skip name as the name has to change anyway)
        category_edit_->setText(QString::fromStdString(preset_->category));
        description_edit_->setText(QString::fromStdString(preset_->description));
    }
    else if (mode_ == Mode::Create)
    {
        //init new preset with current view config (e.g. for preview)
        updateConfig();
    }
    else if (mode_ == Mode::Edit)
    {
        assert(preset_);

        //fill in current metadata
        name_edit_->setText(QString::fromStdString(preset_->name));
        category_edit_->setText(QString::fromStdString(preset_->category));
        description_edit_->setText(QString::fromStdString(preset_->description));

        name_edit_->setFrame(false);
        name_edit_->setReadOnly(true);
    }

    updatePreview();
}

/**
*/
void ViewPresetEditDialog::updatePreview()
{
    preview_label_->setPixmap(QPixmap::fromImage(previewImage(mode_)));
}

/**
*/
void ViewPresetEditDialog::updateConfig()
{
    //update new preset to current view config
    ViewPresets::updatePresetConfig(preset_new_, view_, true);
}

/**
*/
void ViewPresetEditDialog::updateMetaData()
{
    //store current metadata to new preset
    preset_new_.name        = name_edit_->text().toStdString();
    preset_new_.category    = category_edit_->text().toStdString();
    preset_new_.description = description_edit_->toPlainText().toStdString();
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
void ViewPresetEditDialog::apply()
{
    bool ok = false;

    //invoke mode-dependent apply action
    if (mode_ == Mode::Copy)
        ok = applyCopy();
    else if (mode_ == Mode::Create)
        ok = applyCreate();
    else if (mode_ == Mode::Edit)
        ok = applyEdit();

    if (!ok)
        return;

    //everythin ok => accept and close
    accept();
}

/**
*/
bool ViewPresetEditDialog::applyCreate()
{
    //check if name is unique
    if (!checkName())
        return false;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    //update new preset's metadata to form content
    updateMetaData();

    //create new preset
    bool ok = COMPASS::instance().viewManager().viewPresets().createPreset(preset_new_, view_);

    QApplication::restoreOverrideCursor();

    if (!ok)
    {
        QMessageBox::critical(this, "Error", "Creating new preset failed.");
        return false;
    }

    return true;
}

/**
*/
bool ViewPresetEditDialog::applyEdit()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    //update metadata to form content
    updateMetaData();

    //edit existing preset with new metadata
    bool ok = COMPASS::instance().viewManager().viewPresets().updatePreset(preset_->key(), 
                                                                           &preset_new_,
                                                                           nullptr,
                                                                           ViewPresets::UpdateMode::MetaData);
    QApplication::restoreOverrideCursor();

    if (!ok)
    {
        QMessageBox::critical(this, "Error", "Editing preset failed.");
        return false;
    }

    return true;
}

/**
*/
bool ViewPresetEditDialog::applyCopy()
{
    //check if name is unique
    if (!checkName())
        return false;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    //init new preset with data from original preset
    preset_new_ = *preset_;

    //update new preset's metadata to form content
    updateMetaData();

    //create new preset
    bool ok = COMPASS::instance().viewManager().viewPresets().createPreset(preset_new_, view_);

    QApplication::restoreOverrideCursor();

    if (!ok)
    {
        QMessageBox::critical(this, "Error", "Copying preset failed.");
        return false;
    }

    return true;
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
void ViewPresetItemWidget::showSeparatorLine(bool show)
{
    separator_line_->setVisible(show);
}

/**
*/
void ViewPresetItemWidget::createUI()
{
    setCursor(Qt::CursorShape::PointingHandCursor);

    const int DefaultMargin  = 5;
    const int DefaultSpacing = 5;

    //outer layout (contains main layout and a horizontal separation line below)
    QVBoxLayout* outer_layout = new QVBoxLayout;
    outer_layout->setMargin(0);
    outer_layout->setSpacing(0);
    outer_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(outer_layout);

    setContentsMargins(0, 0, 0, 0);

    //main widget
    main_widget_ = new QWidget;

    QHBoxLayout* main_layout = new QHBoxLayout;
    main_layout->setMargin(DefaultMargin);
    main_layout->setSpacing(DefaultSpacing);
    main_widget_->setLayout(main_layout);

    outer_layout->addWidget(main_widget_);

    //bottom separation line
    separator_line_ = new QFrame;
    separator_line_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    separator_line_->setFrameShape(QFrame::HLine);
    separator_line_->setFrameShadow(QFrame::Shadow::Sunken);
    separator_line_->setContentsMargins(0, 0, 0, 0);

    outer_layout->addWidget(separator_line_);
    
    //preview widget (left side, contains preview and modification buttons)
    QWidget* preview_widget = new QWidget(this);
    preview_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    preview_widget->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* preview_widget_layout = new QVBoxLayout;
    preview_widget_layout->setMargin(DefaultMargin / 2); //a little extra margin for preview layout (aligns better with content)
    preview_widget_layout->setSpacing(0);
    preview_widget->setLayout(preview_widget_layout);

    //content widget (center, contains name, description, etc)
    QWidget* content_widget = new QWidget(this);
    content_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    content_widget->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* content_widget_layout = new QHBoxLayout;
    content_widget_layout->setMargin(0);
    content_widget_layout->setSpacing(0);
    content_widget->setLayout(content_widget_layout);

    //add main widgets to main layout
    main_layout->addWidget(preview_widget);
    main_layout->addWidget(content_widget);

    //preview widget contents
    {
        //preview
        {
            QFrame* preview_frame = new QFrame;
            preview_frame->setFrameShape(QFrame::NoFrame);
            preview_frame->setFrameShadow(QFrame::Plain);
            preview_frame->setBackgroundRole(QPalette::Dark);
            preview_frame->setContentsMargins(0, 0, 0, 0);

            QVBoxLayout* frame_layout = new QVBoxLayout;
            frame_layout->setMargin(0);
            frame_layout->setSpacing(0);
            preview_frame->setLayout(frame_layout);

            preview_label_ = new QLabel;
            preview_label_->setFixedSize(ViewPresets::PreviewMaxSize, ViewPresets::PreviewMaxSize);

            QFont f = preview_label_->font();
            f.setItalic(true);
            f.setPointSize(f.pointSize() - 1);
            preview_label_->setFont(f);

            frame_layout->addWidget(preview_label_);

            preview_widget_layout->addWidget(preview_frame);
        }

        preview_widget_layout->addStretch(1);
    }

    //content widget contents
    {
        QWidget* info_widget = new QWidget;
        info_widget->setContentsMargins(0, 0, 0, 0);
        
        QVBoxLayout* info_layout = new QVBoxLayout;
        info_layout->setSpacing(1);
        info_layout->setMargin(0);
        info_widget->setLayout(info_layout);

        QHBoxLayout* layout_header = new QHBoxLayout;
        layout_header->setSpacing(1);
        layout_header->setMargin(1);
        info_layout->addLayout(layout_header);

        //header
        {
            //category
            category_label_ = new QLabel;
            category_label_->setAlignment(Qt::AlignBottom);

            QFont f = category_label_->font();
            f.setBold(true);
            f.setItalic(true);
            f.setPointSize(f.pointSize() - 2);
            category_label_->setFont(f);

            layout_header->addWidget(category_label_);

            layout_header->addStretch(1);

            //buttons
            edit_button_ = new QToolButton;
            edit_button_->setIcon(QIcon(Utils::Files::getIconFilepath("edit_old.png").c_str()));
            edit_button_->setToolTip("Edit preset");
            edit_button_->setAutoRaise(true);
            edit_button_->setCursor(Qt::CursorShape::ArrowCursor);

            copy_button_ = new QToolButton;
            copy_button_->setIcon(QIcon(Utils::Files::getIconFilepath("copy.png").c_str()));
            copy_button_->setToolTip("Copy preset");
            copy_button_->setAutoRaise(true);
            copy_button_->setCursor(Qt::CursorShape::ArrowCursor);

            save_button_ = new QToolButton;
            save_button_->setIcon(QIcon(Utils::Files::getIconFilepath("save.png").c_str()));
            save_button_->setToolTip("Save view changes to preset");
            save_button_->setAutoRaise(true);
            save_button_->setCursor(Qt::CursorShape::ArrowCursor);

            remove_button_ = new QToolButton;
            remove_button_->setIcon(QIcon(Utils::Files::getIconFilepath("delete.png").c_str()));
            remove_button_->setToolTip("Remove preset");
            remove_button_->setAutoRaise(true);
            remove_button_->setCursor(Qt::CursorShape::ArrowCursor);

            //add modification buttons
            layout_header->addWidget(edit_button_);
            layout_header->addWidget(copy_button_);
            layout_header->addWidget(save_button_);

#if 1
            //add a vertical separator
            auto sep = new QFrame;
            sep->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
            sep->setFrameShape(QFrame::VLine);
            sep->setFrameShadow(QFrame::Shadow::Plain);
            sep->setContentsMargins(0, 0, 0, 0);

            auto p = sep->palette();
            p.setColor(QPalette::ColorRole::HighlightedText, p.color(QPalette::ColorRole::Foreground));

            sep->setPalette(p);

            layout_header->addWidget(sep);
#else
            //add some space
            layout_header->addSpacerItem(new QSpacerItem(DefaultSpacing * 4, 1, QSizePolicy::Fixed, QSizePolicy::Fixed));
#endif

            //finally add remove button
            layout_header->addWidget(remove_button_);

            connect(edit_button_  , &QToolButton::pressed, this, &ViewPresetItemWidget::editButtonPressed  );
            connect(copy_button_  , &QToolButton::pressed, this, &ViewPresetItemWidget::copyButtonPressed  );
            connect(save_button_  , &QToolButton::pressed, this, &ViewPresetItemWidget::saveButtonPressed  );
            connect(remove_button_, &QToolButton::pressed, this, &ViewPresetItemWidget::removeButtonPressed);
        }

        QHBoxLayout* layout_name = new QHBoxLayout;
        layout_name->setSpacing(1);
        layout_name->setMargin(1);
        info_layout->addLayout(layout_name);

        //name
        {
            name_label_ = new QLabel;
            name_label_->setAlignment(Qt::AlignTop);
            name_label_->setWordWrap(true);

            QFont f = name_label_->font();
            f.setBold(true);
            name_label_->setFont(f);

            layout_name->addWidget(name_label_);
        }

        //a little extra spacing between "header" and description
        info_layout->addSpacerItem(new QSpacerItem(1, DefaultSpacing, QSizePolicy::Fixed, QSizePolicy::Fixed));

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
void ViewPresetItemWidget::copyButtonPressed()
{
    emit copyPreset(key_);
}

/**
*/
void ViewPresetItemWidget::saveButtonPressed()
{
    auto& presets = COMPASS::instance().viewManager().viewPresets();
    assert(presets.hasPreset(key_));

    QApplication::setOverrideCursor(Qt::WaitCursor);

    //update preset view config
    bool ok = presets.updatePreset(key_, nullptr, view_, ViewPresets::UpdateMode::ViewConfig, true);

    QApplication::restoreOverrideCursor();

    if (ok)
    {
        updateContents();
        QApplication::processEvents();
    }
    else
    {
        QMessageBox::critical(this, "Error", "Preset could not be written.");
    }
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
        emit presetApplied(key_);
    }
}

/**
*/
void ViewPresetItemWidget::enterEvent(QEvent* event)
{
    if (!inside_)
    {
        inside_ = true;
        main_widget_->setBackgroundRole(QPalette::ColorRole::Highlight);
        main_widget_->setAutoFillBackground(true);
    }
}

/**
*/
void ViewPresetItemWidget::leaveEvent(QEvent* event)
{
    if (inside_)
    {
        inside_ = false;
        main_widget_->setBackgroundRole(QPalette::ColorRole::Background);
        main_widget_->setAutoFillBackground(false);
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

    QHBoxLayout* header_layout = new QHBoxLayout;
    header_layout->setMargin(10);
    header_layout->setSpacing(10);
    layout->addLayout(header_layout);

    filter_edit_ = new QLineEdit;
    filter_edit_->setPlaceholderText("Filter for preset name...");
    header_layout->addWidget(filter_edit_);

    //layout->addSpacerItem(new QSpacerItem(1, 9, QSizePolicy::Fixed, QSizePolicy::Fixed));

    QWidget* widget = new QWidget;
    QVBoxLayout* widget_layout = new QVBoxLayout;
    widget_layout->setMargin(0);
    widget_layout->setSpacing(0);
    widget->setLayout(widget_layout);

    item_layout_ = new QVBoxLayout;
    item_layout_->setMargin(0);
    item_layout_->setSpacing(0);
    item_layout_->setContentsMargins(0, 0, 0, 0);

    widget_layout->addLayout(item_layout_);
    widget_layout->addStretch(1);

    QScrollArea* scroll_area = new QScrollArea;
    scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scroll_area->setWidgetResizable(true);

    scroll_area->setWidget(widget);

    layout->addWidget(scroll_area);

    QHBoxLayout* footer_layout = new QHBoxLayout;
    footer_layout->setMargin(2);
    footer_layout->setSpacing(10);
    layout->addLayout(footer_layout);

    QPushButton* add_button = new QPushButton;
    add_button->setIcon(QIcon(Utils::Files::getIconFilepath("crosshair_fat.png").c_str()));
    add_button->setText("Add preset");
    footer_layout->addWidget(add_button);
    footer_layout->addStretch(1);

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
    ViewPresetEditDialog dlg(view_, nullptr, ViewPresetEditDialog::Mode::Create, this);
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
    ViewPresetEditDialog dlg(view_, &presets.presets().at(key), ViewPresetEditDialog::Mode::Edit, this);
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
void ViewPresetItemListWidget::copyPreset(ViewPresets::Key key)
{
    auto& presets = COMPASS::instance().viewManager().viewPresets();
    assert(presets.hasPreset(key));

    //copy preset
    ViewPresetEditDialog dlg(view_, &presets.presets().at(key), ViewPresetEditDialog::Mode::Copy, this);
    if (dlg.exec() != QDialog::Accepted)
        return;
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

        connect(item, &ViewPresetItemWidget::removePreset , this, &ViewPresetItemListWidget::removePreset );
        connect(item, &ViewPresetItemWidget::editPreset   , this, &ViewPresetItemListWidget::editPreset   );
        connect(item, &ViewPresetItemWidget::copyPreset   , this, &ViewPresetItemListWidget::copyPreset   );
        connect(item, &ViewPresetItemWidget::presetApplied, this, &ViewPresetItemListWidget::presetApplied);

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
    updateContents();

    connect(view, &View::presetChangedSignal, this, &ViewPresetWidget::updateContents);
}

/**
*/
QString ViewPresetWidget::generateTooltip() const
{
    QString tt = "Press to open preset selection";

    if (view_->activePreset())
    {
        tt += "<br><br><b>Active preset</b>: " + QString::fromStdString(view_->activePreset()->name);
        tt += "<br><b>Unsaved changes</b>: " + QString(view_->presetChanged() ? "Yes" : "No");
    }
    else
    {
        tt += "<br><br><i>No active preset</i>";
    }

    return tt;
}

/**
*/
QString ViewPresetWidget::generateButtonText() const
{
    QString txt = (view_->activePreset() ? QString::fromStdString(view_->activePreset()->name) : QString("No active preset"));

    if (view_->presetChanged())
        txt += "*";

    return txt;
}

/**
*/
void ViewPresetWidget::createUI()
{
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setMargin(5);
    setLayout(layout);

    preset_list_ = new ViewPresetItemListWidget(view_, this);

    show_button_ = new QToolButton;
    show_button_->setIcon(QIcon(Utils::Files::getIconFilepath("preset.png").c_str()));
    show_button_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    show_button_->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    show_button_->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
    show_button_->setAutoRaise(false);
    show_button_->setCursor(Qt::PointingHandCursor);
    layout->addWidget(show_button_);

    button_menu_ = new QMenu;
    auto action = new QWidgetAction(this);
    action->setDefaultWidget(preset_list_);
    button_menu_->addAction(action);

    show_button_->setMenu(button_menu_);

    connect(preset_list_, &ViewPresetItemListWidget::presetApplied, this, &ViewPresetWidget::presetApplied);
}

/**
*/
void ViewPresetWidget::updateContents()
{
    auto setFontItalic = [ & ] (bool italic)
    {
        auto f = show_button_->font();
        f.setItalic(italic);

        show_button_->setFont(f);
    };

    setFontItalic(!view_->activePreset());

    show_button_->setText(generateButtonText());
    show_button_->setToolTip(generateTooltip());
}

/**
*/
void ViewPresetWidget::presetApplied(ViewPresets::Key key)
{
    //hide the menu in case something unexpected happens (like an assert)
    button_menu_->hide();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    //apply preset
    std::string error;
    std::vector<Configurable::SubConfigKey> missing_keys;

    const auto& presets = COMPASS::instance().viewManager().viewPresets();
    const auto& preset  = presets.presets().at(key);

    bool ok = view_->applyPreset(preset, &missing_keys, &error);

    QApplication::restoreOverrideCursor();

    //any errors?
    if (!ok)
    {
        QMessageBox::critical(this, "Error", "View preset could not be applied: " + QString::fromStdString(error));
        return;
    }

    //any missing config keys found?
    if (!missing_keys.empty())
    {
        QString msg = "The following view preset configuration key(s) could not be applied to the view.\n";
        for (const auto& key : missing_keys)
            msg += "   " + QString::fromStdString(key.first) + "." + QString::fromStdString(key.second) + "\n";

        QMessageBox::warning(this, "Warning", msg);
    }
}
