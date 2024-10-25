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

#include <string>
#include <map>

#include <QDialog>

class QVBoxLayout;
class QPushButton;
class QCheckBox;

/**
 */
class FileImportDialog : public QDialog
{
public:
    FileImportDialog(const std::vector<std::string>& files,
                     QWidget* parent = nullptr, 
                     Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~FileImportDialog() = default;

    bool importFile(const std::string& fn) const;

protected:
    virtual bool importOk() const;
    void checkImportOk();

    QVBoxLayout* customs_layout_ = nullptr;

private:
    std::map<std::string, bool> import_flags_;

    QPushButton* ok_button_ = nullptr;
};

/**
*/
class GeoTIFFImportDialog : public FileImportDialog
{
public:
    GeoTIFFImportDialog(const std::vector<std::string>& files,
                        QWidget* parent = nullptr, 
                        Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~GeoTIFFImportDialog() = default;

    bool subsamplingEnabled() const;

private:
    QCheckBox* subsample_box_ = nullptr;
};
