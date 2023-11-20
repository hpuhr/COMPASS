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

#include <QDialog>
#include <QImage>

class View;
class QRadioButton;

/**
*/
class ViewScreenshotDialog : public QDialog
{
public:
    ViewScreenshotDialog(View* view, 
                         QWidget* parent = nullptr, 
                         Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~ViewScreenshotDialog() = default;

    static const int PreviewSize = 150;

private:
    void createUI();
    void generateScreenshots();
    void copyToClipboard();
    void save();

    const QImage& getScreenshot() const;

    View* view_ = nullptr;

    QRadioButton* button_data_ = nullptr;
    QRadioButton* button_view_ = nullptr;

    QImage screener_data_;
    QImage screener_view_;
    QImage preview_data_;
    QImage preview_view_;
    QImage preview_data_sel_;
    QImage preview_view_sel_;
};
