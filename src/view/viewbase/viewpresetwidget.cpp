
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

    QHBoxLayout* hlayout1 = new QHBoxLayout;
    hlayout1->setSpacing(1);
    layout->addLayout(hlayout1);

    preset_combo_ = new QComboBox;
    preset_combo_->setEditable(false);
    hlayout1->addWidget(preset_combo_);

    add_button_ = new QToolButton;
    add_button_->setIcon(QIcon(Utils::Files::getIconFilepath("crosshair_fat.png").c_str()));
    add_button_->setCheckable(false);
    add_button_->setToolTip("Add new preset");
    hlayout1->addWidget(add_button_);

    update_button_ = new QToolButton;
    update_button_->setIcon(QIcon(Utils::Files::getIconFilepath("edit_old.png").c_str()));
    update_button_->setCheckable(false);
    update_button_->setToolTip("Update current preset");
    hlayout1->addWidget(update_button_);

    rem_button_ = new QToolButton;
    rem_button_->setIcon(QIcon(Utils::Files::getIconFilepath("delete.png").c_str()));
    rem_button_->setCheckable(false);
    rem_button_->setToolTip("Delete current preset");
    hlayout1->addWidget(rem_button_);

    apply_button_ = new QToolButton;
    apply_button_->setIcon(QIcon(Utils::Files::getIconFilepath("play.png").c_str()));
    apply_button_->setCheckable(false);
    apply_button_->setToolTip("Apply current preset");
    hlayout1->addWidget(apply_button_);

    QHBoxLayout* hlayout2 = new QHBoxLayout;
    hlayout2->setSpacing(1);
    layout->addLayout(hlayout2);

    preview_label_ = new QLabel;
    preview_label_->setFixedSize(ViewPresets::PreviewMaxSize, ViewPresets::PreviewMaxSize);

    QFont f = preview_label_->font();
    f.setItalic(true);
    f.setPointSize(f.pointSize() - 1);

    preview_label_->setFont(f);
    preview_label_->setAlignment(Qt::AlignCenter);
    hlayout2->addWidget(preview_label_);

    descr_edit_ = new QTextEdit("-");
    descr_edit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    descr_edit_->setFixedHeight(ViewPresets::PreviewMaxSize);
    descr_edit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    descr_edit_->setReadOnly(true);
    hlayout2->addWidget(descr_edit_);

    connect(preset_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ViewPresetWidget::currentPresetChanged);
    connect(add_button_, &QToolButton::pressed, this, &ViewPresetWidget::addPreset);
    connect(update_button_, &QToolButton::pressed, this, &ViewPresetWidget::updatePreset);
    connect(rem_button_, &QToolButton::pressed, this, &ViewPresetWidget::removeCurrentPreset);
    connect(apply_button_, &QToolButton::pressed, this, &ViewPresetWidget::applyCurrentPreset);
}

/**
*/
void ViewPresetWidget::refill()
{
    preset_combo_->blockSignals(true);
    preset_combo_->clear();

    const auto& presets    = COMPASS::instance().viewManager().viewPresets();
    const auto& preset_map = presets.presets();

    current_keys_ = presets.keysFor(view_);

    //std::cout << "obtained " << current_keys_.size() << " preset key(s) for view " << view_->classId() << std::endl;

    for (const auto& key : current_keys_)
        preset_combo_->addItem(QString::fromStdString(preset_map.at(key).name));

    if (preset_combo_->count() > 0)
        preset_combo_->setCurrentIndex(0);

    preset_combo_->blockSignals(false);

    //update display for current preset
    currentPresetChanged();
}

/**
*/
void ViewPresetWidget::currentPresetChanged()
{
    preview_label_->setPixmap(QPixmap());
    preview_label_->setText("No Preview");

    descr_edit_->setText("No Description Available");

    int idx = preset_combo_->currentIndex();

    update_button_->setEnabled(idx >= 0);
    rem_button_->setEnabled(idx >= 0);
    apply_button_->setEnabled(idx >= 0);

    if (idx < 0)
        return;

    auto key = current_keys_.at(idx);

    const auto& preset = COMPASS::instance().viewManager().viewPresets().presets().at(key);

    if (!preset.preview.isNull())
        preview_label_->setPixmap(QPixmap::fromImage(preset.preview));

    if (!preset.description.empty())
        descr_edit_->setText(QString::fromStdString(preset.description));
}

/**
*/
void ViewPresetWidget::configurePreset(const ViewPresets::Preset* preset)
{
    QDialog dlg;
    dlg.setWindowTitle(preset ? "Update preset" : "Create new preset");

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

    QCheckBox* preview_box = new QCheckBox("Create preview image");
    preview_box->setChecked(true);
    layout->addWidget(preview_box, 3, 1);

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

    connect(ok_button, &QPushButton::pressed, ok_callback);
    connect(cancel_button, &QPushButton::pressed, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Rejected)
        return;

    std::string name           = name_edit->text().toStdString();
    std::string category       = cat_edit->text().toStdString();
    std::string description    = descr_edit->toPlainText().toStdString();
    bool        create_preview = preview_box->isChecked();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    bool ok = preset ? presets.updatePreset(view_, name, category, description, create_preview) :
                       presets.createPreset(view_, name, category, description, create_preview);

    QApplication::restoreOverrideCursor();

    if (!ok)
    {
        QMessageBox::critical(this, "Error", preset ? "Updating preset failed." : "Creating new preset failed.");
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    refill();

    QApplication::restoreOverrideCursor();

    //set added preset selected and show
    int idx = preset_combo_->findText(QString::fromStdString(name));
    assert(idx >= 0);

    preset_combo_->setCurrentIndex(idx);
    currentPresetChanged();
}

/**
*/
void ViewPresetWidget::addPreset()
{
    configurePreset(nullptr);
}

/**
*/
void ViewPresetWidget::updatePreset()
{
    int idx = preset_combo_->currentIndex();
    if (idx < 0)
        return;

    auto key = current_keys_.at(idx);

    const auto& preset = COMPASS::instance().viewManager().viewPresets().presets().at(key);

    configurePreset(&preset);
}

/**
*/
void ViewPresetWidget::removeCurrentPreset()
{
    auto name = preset_combo_->currentText().toStdString();
    if (name.empty())
        return;

    auto& presets = COMPASS::instance().viewManager().viewPresets();
    presets.removePreset(view_, name);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    refill();

    QApplication::restoreOverrideCursor();
}

/**
*/
void ViewPresetWidget::applyCurrentPreset()
{
    int idx = preset_combo_->currentIndex();
    if (idx < 0)
        return;

    auto key = current_keys_.at(idx);

    const auto& preset = COMPASS::instance().viewManager().viewPresets().presets().at(key);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    //reconfigure view using the preset's view config
    view_->reconfigure(preset.view_config);

    QApplication::restoreOverrideCursor();
}
