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

#include "viewscreenshotdialog.h"
#include "view.h"
#include "viewwidget.h"
#include "timeconv.h"
#include "compass.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QToolButton>
#include <QLineEdit>
#include <QPushButton>

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>

/**
*/
ViewScreenshotDialog::ViewScreenshotDialog(View* view, 
                                           QWidget* parent, 
                                           Qt::WindowFlags f) 
:   QDialog(parent, f)
,   view_  (view     )
{
    assert(view_);

    generateScreenshots();
    createUI();

    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

/**
*/
void ViewScreenshotDialog::createUI()
{
    setWindowTitle("Screenshot");

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);

    QLabel* label_preview_data = new QLabel;
    label_preview_data->setPixmap(QPixmap::fromImage(preview_data_));

    QLabel* label_preview_view = new QLabel;
    label_preview_view->setPixmap(QPixmap::fromImage(preview_view_));

    QGridLayout* layout_previews = new QGridLayout;

    button_data_ = new QRadioButton("Data");
    button_view_ = new QRadioButton("View");

    layout_previews->addWidget(label_preview_data, 0, 1);
    layout_previews->addWidget(label_preview_view, 0, 2);
    layout_previews->addWidget(button_data_      , 1, 1);
    layout_previews->addWidget(button_view_      , 1, 2);

    layout_previews->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 0);
    layout_previews->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed), 0, 3);

    layout->addLayout(layout_previews);

    layout->addStretch(1);

    QHBoxLayout* layout_buttons = new QHBoxLayout;
    layout->addLayout(layout_buttons);

    QPushButton* button_cancel = new QPushButton("Cancel");
    QPushButton* button_clipb  = new QPushButton("Copy to Clipboard");
    QPushButton* button_save   = new QPushButton("Save");

    layout_buttons->addWidget(button_cancel);
    layout_buttons->addStretch(1);
    layout_buttons->addWidget(button_clipb);
    layout_buttons->addWidget(button_save);

    button_data_->setChecked(true);

    auto selection_cb = [ = ] (bool data_selected)
    {
        label_preview_data->setPixmap(QPixmap::fromImage(data_selected ? preview_data_sel_ : preview_data_));
        label_preview_view->setPixmap(QPixmap::fromImage(data_selected ? preview_view_ : preview_view_sel_));
    };

    selection_cb(true);

    connect(button_data_, &QRadioButton::toggled, selection_cb);

    connect(button_cancel, &QPushButton::pressed, this, &ViewScreenshotDialog::reject         );
    connect(button_clipb , &QPushButton::pressed, this, &ViewScreenshotDialog::copyToClipboard);
    connect(button_save  , &QPushButton::pressed, this, &ViewScreenshotDialog::save           );
}

/**
 */
void ViewScreenshotDialog::generateScreenshots()
{
    assert(view_);

    auto createPreview = [ & ] (QImage& preview,
                                QImage& preview_selected,
                                const QImage& img)
    {
        QImage preview_img(PreviewSize, PreviewSize, QImage::Format_RGB32);
        preview_img.fill(Qt::gray);

        auto scaled_img = img.scaled(PreviewSize, PreviewSize, Qt::AspectRatioMode::KeepAspectRatio);

        int offsx = (PreviewSize - scaled_img.width() ) / 2;
        int offsy = (PreviewSize - scaled_img.height()) / 2;

        QPainter painter(&preview_img);
        painter.drawImage(offsx, offsy, scaled_img);

        preview = preview_img;

        painter.drawRect(0, 0, PreviewSize - 1, PreviewSize - 1);

        preview_selected = preview_img;
    };

    screener_data_ = view_->renderData();
    screener_view_ = view_->renderView();

    createPreview(preview_data_, preview_data_sel_, screener_data_);
    createPreview(preview_view_, preview_view_sel_, screener_view_);
}

/**
 */
void ViewScreenshotDialog::copyToClipboard()
{
    QApplication::clipboard()->setImage(getScreenshot(), QClipboard::Clipboard);
    accept();
}

/**
 */
void ViewScreenshotDialog::save()
{
    auto timestamp = Utils::Time::toString(Utils::Time::currentUTCTime());

    std::string fn_init = view_->classId() + "_" + timestamp + ".png";
    std::string path    = COMPASS::instance().lastUsedPath() + "/" + fn_init;

    auto fn = QFileDialog::getSaveFileName(this, "Select screenshot file", QString::fromStdString(path), "*.png");
    if (fn.isEmpty())
        return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    bool ok = getScreenshot().save(fn);

    QApplication::restoreOverrideCursor();

    if (!ok)
        QMessageBox::critical(this, "Error", "Screenshot could not be written.");

    accept();
}

/**
 */
const QImage& ViewScreenshotDialog::getScreenshot() const
{
    return (button_data_->isChecked() ? screener_data_ : screener_view_); 
}
