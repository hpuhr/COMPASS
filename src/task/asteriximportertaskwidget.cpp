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

#include "asteriximportertaskwidget.h"
#include "asteriximportertask.h"
#include "asterixconfigwidget.h"
#include "logger.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidget>

ASTERIXImporterTaskWidget::ASTERIXImporterTaskWidget(ASTERIXImporterTask& task, QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f), task_(task)
{
    setWindowTitle ("Import ASTERIX");
    setMinimumSize(QSize(800, 600));

    jasterix_ = task_.jASTERIX();

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    int frame_width_small = 1;

    main_layout_ = new QHBoxLayout ();

    QVBoxLayout* left_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Import ASTERIX data");
    main_label->setFont (font_big);
    left_layout->addWidget (main_label);

    // file stuff
    {
        QFrame *file_frame = new QFrame ();
        file_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        file_frame->setLineWidth(frame_width_small);

        QVBoxLayout* files_layout = new QVBoxLayout();

        QLabel *files_label = new QLabel ("File Selection");
        files_label->setFont(font_bold);
        files_layout->addWidget(files_label);

        file_list_ = new QListWidget ();
        file_list_->setWordWrap(true);
        file_list_->setTextElideMode (Qt::ElideNone);
        file_list_->setSelectionBehavior( QAbstractItemView::SelectItems );
        file_list_->setSelectionMode( QAbstractItemView::SingleSelection );
        connect (file_list_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectedFileSlot()));

        updateFileListSlot ();
        files_layout->addWidget(file_list_);

        QHBoxLayout* button_layout = new QHBoxLayout();

        add_file_button_ = new QPushButton ("Add");
        connect (add_file_button_, SIGNAL(clicked()), this, SLOT(addFileSlot()));
        button_layout->addWidget(add_file_button_);

        delete_file_button_ = new QPushButton ("Remove");
        connect (delete_file_button_, SIGNAL(clicked()), this, SLOT(deleteFileSlot()));
        button_layout->addWidget(delete_file_button_);

        files_layout->addLayout(button_layout);

        file_frame->setLayout (files_layout);

        left_layout->addWidget (file_frame, 1);
    }

    {
        config_widget_ = new ASTERIXConfigWidget (task_, this);
        left_layout->addWidget(config_widget_);
        //config_widget_->show();
    }

    main_layout_->addLayout(left_layout);

    setLayout (main_layout_);

    show();
}

ASTERIXImporterTaskWidget::~ASTERIXImporterTaskWidget()
{
    config_widget_ = nullptr;
}

void ASTERIXImporterTaskWidget::addFileSlot ()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Add ASTERIX File"));

    if (filename.size() > 0)
    {
        if (!task_.hasFile(filename.toStdString()))
            task_.addFile(filename.toStdString());
    }
}

void ASTERIXImporterTaskWidget::deleteFileSlot ()
{
    loginf << "ASTERIXImporterTaskWidget: deleteFileSlot";

    if (!file_list_->currentItem() || !task_.currentFilename().size())
    {
        QMessageBox m_warning (QMessageBox::Warning, "ASTERIX File Deletion Failed",
                               "Please select a file in the list.",
                               QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    assert (task_.currentFilename().size());
    assert (task_.hasFile(task_.currentFilename()));
    task_.removeCurrentFilename();
}

void ASTERIXImporterTaskWidget::selectedFileSlot ()
{
    loginf << "ASTERIXImporterTaskWidget: selectedFileSlot";
    assert (file_list_->currentItem());

    QString filename = file_list_->currentItem()->text();
    assert (task_.hasFile(filename.toStdString()));
    task_.currentFilename (filename.toStdString());
}

void ASTERIXImporterTaskWidget::updateFileListSlot ()
{
    assert (file_list_);

    file_list_->clear();

    for (auto it : task_.fileList())
    {
        QListWidgetItem *item = new QListWidgetItem(tr(it.first.c_str()), file_list_);
        if (it.first == task_.currentFilename())
            file_list_->setCurrentItem(item);
    }
}
