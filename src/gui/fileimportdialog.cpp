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

#include "fileimportdialog.h"

#include <QPushButton>
#include <QCheckBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QHeaderView>

/***************************************************************************
 * FileImportDialog
 ***************************************************************************/

/**
*/
FileImportDialog::FileImportDialog(const std::vector<std::string>& files,
                                   QWidget* parent, 
                                   Qt::WindowFlags f)
:   QDialog(parent, f)
{
    for (const auto& f : files)
        import_flags_[ f ] = true;

    setWindowTitle("Import Files");

    auto layout = new QVBoxLayout;
    setLayout(layout);

    auto tree_widget = new QTreeWidget(this);
    tree_widget->setColumnCount(2);

    QStringList headers;
    headers << "Import";
    headers << "Filename";

    tree_widget->setHeaderLabels(headers);

    tree_widget->header()->setSectionResizeMode(0, QHeaderView::ResizeMode::Fixed);
    tree_widget->header()->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);

    tree_widget->resizeColumnToContents(0);

    layout->addWidget(tree_widget);

    customs_layout_ = new QVBoxLayout;
    layout->addLayout(customs_layout_);

    layout->addStretch(1);

    auto button_layout = new QHBoxLayout;
    layout->addLayout(button_layout);

    ok_button_         = new QPushButton("Import");
    auto cancel_button = new QPushButton("Cancel");

    button_layout->addStretch(1);
    button_layout->addWidget(ok_button_);
    button_layout->addWidget(cancel_button);

    for (const auto& f : files)
    {
        auto item = new QTreeWidgetItem;
        item->setCheckState(0, Qt::Checked);
        item->setText(1, QString::fromStdString(f));

        tree_widget->addTopLevelItem(item);
    }

    auto itemChangedCB = [ = ] (QTreeWidgetItem* item, int column)
    {
        if (column == 0)
        {
            bool import = item->checkState(0) == Qt::Checked;
            auto name   = item->text(1).toStdString();

            import_flags_[ name ] = import;

            this->checkImportOk();
        }
    };

    connect(tree_widget, &QTreeWidget::itemChanged, itemChangedCB);
    connect(ok_button_, &QPushButton::pressed, this, &QDialog::accept);
    connect(cancel_button, &QPushButton::pressed, this, &QDialog::reject);
}

/**
*/
bool FileImportDialog::importFile(const std::string& fn) const
{
    auto it = import_flags_.find(fn);
    if (it == import_flags_.end())
        return false;

    return it->second;
}

/**
*/
bool FileImportDialog::importOk() const
{
    bool ok = false;
    for (const auto& f : import_flags_)
    {
        if (f.second)
        {
            ok = true;
            break;
        }
    }

    return ok;
}

/**
*/
void FileImportDialog::checkImportOk()
{
    bool ok = this->importOk();
    ok_button_->setEnabled(ok);
}

/***************************************************************************
 * GeoTIFFImportDialog
 ***************************************************************************/

/**
*/
GeoTIFFImportDialog::GeoTIFFImportDialog(const std::vector<std::string>& files,
                                         QWidget* parent, 
                                         Qt::WindowFlags f)
:   FileImportDialog(files, parent, f)
{
    subsample_box_ = new QCheckBox("Enable subsampling");
    subsample_box_->setChecked(true);

    customs_layout_->addWidget(subsample_box_);
}

/**
*/
bool GeoTIFFImportDialog::subsamplingEnabled() const
{
    return subsample_box_->isChecked();
}
