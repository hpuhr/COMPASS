/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "viewpointsimporttaskwidget.h"
#include "viewpointsimporttask.h"
#include "atsdb.h"
#include "global.h"
#include "logger.h"
#include "stringconv.h"

#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QListWidget>
#include <QMessageBox>


ViewPointsImportTaskWidget::ViewPointsImportTaskWidget(ViewPointsImportTask& task, QWidget* parent)
    : TaskWidget(parent), task_(task)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout_ = new QVBoxLayout();

    // file stuff
    {
        QVBoxLayout* files_layout = new QVBoxLayout();

        QLabel* files_label = new QLabel("File Selection");
        files_label->setFont(font_bold);
        files_layout->addWidget(files_label);

        file_list_ = new QListWidget();
        file_list_->setWordWrap(true);
        file_list_->setTextElideMode(Qt::ElideNone);
        file_list_->setSelectionBehavior(QAbstractItemView::SelectItems);
        file_list_->setSelectionMode(QAbstractItemView::SingleSelection);
        connect(file_list_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectedFileSlot()));

        updateFileListSlot();
        files_layout->addWidget(file_list_);

        QHBoxLayout* button_layout = new QHBoxLayout();

        add_file_button_ = new QPushButton("Add");
        connect(add_file_button_, &QPushButton::clicked, this, &ViewPointsImportTaskWidget::addFileSlot);
        button_layout->addWidget(add_file_button_);

        delete_file_button_ = new QPushButton("Remove");
        connect(delete_file_button_, &QPushButton::clicked, this, &ViewPointsImportTaskWidget::deleteFileSlot);
        button_layout->addWidget(delete_file_button_);

        files_layout->addLayout(button_layout);

        main_layout_->addLayout(files_layout);
    }

    context_edit_ = new QTextEdit ();
    context_edit_->setReadOnly(true);
    main_layout_->addWidget(context_edit_);

    import_button_ = new QPushButton("Import");
    connect (import_button_, &QPushButton::clicked, this, &ViewPointsImportTaskWidget::importSlot);
    main_layout_->addWidget(import_button_);

    if (task_.currentFilename().size())
        updateContext();

    setLayout(main_layout_);
}

ViewPointsImportTaskWidget::~ViewPointsImportTaskWidget()
{
    logdbg << "ViewPointsImportTaskWidget: destructor";

}

void ViewPointsImportTaskWidget::addFileSlot()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle("Add View Points File(s)");
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(trUtf8("JSON (*.json)"));

    QStringList fileNames;
    if (dialog.exec())
    {
        for (auto& filename : dialog.selectedFiles())
            addFile(filename.toStdString());
    }
}

void ViewPointsImportTaskWidget::addFile(const std::string& filename)
{
    if (!task_.hasFile(filename))
        task_.addFile(filename);
}

void ViewPointsImportTaskWidget::selectFile(const std::string& filename)
{
    QList<QListWidgetItem*> items = file_list_->findItems(filename.c_str(), Qt::MatchExactly);
    assert (items.size() > 0);

    assert(task_.hasFile(filename));
    task_.currentFilename(filename);

    for (auto item_it : items)
    {
        assert (item_it);
        file_list_->setCurrentItem(item_it);
    }

}

void ViewPointsImportTaskWidget::deleteFileSlot()
{
    loginf << "ViewPointsImportTaskWidget: deleteFileSlot";

    if (!file_list_->currentItem() || !task_.currentFilename().size())
    {
        QMessageBox m_warning(QMessageBox::Warning, "JSON File Deletion Failed",
                              "Please select a file in the list.", QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    assert(task_.currentFilename().size());
    assert(task_.hasFile(task_.currentFilename()));
    task_.removeCurrentFilename();
}

void ViewPointsImportTaskWidget::selectedFileSlot()
{
    loginf << "ViewPointsImportTaskWidget: selectedFileSlot";
    assert(file_list_->currentItem());

    QString filename = file_list_->currentItem()->text();
    assert(task_.hasFile(filename.toStdString()));
    task_.currentFilename(filename.toStdString());
}

void ViewPointsImportTaskWidget::updateFileListSlot()
{
    assert(file_list_);

    file_list_->clear();

    for (auto it : task_.fileList())
    {
        QListWidgetItem* item = new QListWidgetItem(tr(it.first.c_str()), file_list_);
        if (it.first == task_.currentFilename())
            file_list_->setCurrentItem(item);
    }
}

void ViewPointsImportTaskWidget::expertModeChangedSlot() {}

void ViewPointsImportTaskWidget::updateContext ()
{
    assert (context_edit_);
    context_edit_->setText("");

    if (task_.currentError().size())
    {
        context_edit_->setText(QString("Error: ")+task_.currentError().c_str());
    }
    else
    {
        const nlohmann::json& data = task_.currentData();

        if (data.contains("view_point_context"))
            context_edit_->setText(data.at("view_point_context").dump(4).c_str());
        else
            context_edit_->setText("No view point context defined");
    }
}

void ViewPointsImportTaskWidget::importSlot()
{
    if (task_.currentError().size())
    {
        loginf << "ViewPointsImportTaskWidget: importSlot: error '" << task_.currentError() << "'";
        return;
    }

    if (!task_.canImport())
    {
        loginf << "ViewPointsImportTaskWidget: importSlot: task can not import";
        return;
    }

    task_.import();
}
