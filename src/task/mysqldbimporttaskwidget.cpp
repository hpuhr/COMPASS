#include "mysqldbimporttaskwidget.h"
#include "mysqldbimporttask.h"
#include "logger.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QListWidget>
#include <QLabel>
#include <QMessageBox>

//using namespace Utils;

MySQLDBImportTaskWidget::MySQLDBImportTaskWidget(MySQLDBImportTask& task, QWidget* parent, Qt::WindowFlags f)
    : TaskWidget (parent, f), task_(task)
{
    QFont font_bold;
    font_bold.setBold(true);

    QHBoxLayout* main_layout = new QHBoxLayout ();

    // file stuff
    {
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

        main_layout->addLayout(files_layout);
    }

    expertModeChangedSlot();

    setLayout (main_layout);
}

MySQLDBImportTaskWidget::~MySQLDBImportTaskWidget()
{
}

void MySQLDBImportTaskWidget::addFileSlot ()
{
    QFileDialog dialog(this, "Add Database File");
    dialog.setFileMode(QFileDialog::ExistingFile);

    QStringList filters;
    filters << "DB Files (*.sql *.tar.gz *.gz *.tar *.tgz)";

    dialog.setNameFilters(filters);
    dialog.setViewMode(QFileDialog::Detail);

    QStringList filenames;
    if (dialog.exec())
    {
        filenames = dialog.selectedFiles();

        if (filenames.size() > 1)
        {
            QMessageBox msgBox;
            msgBox.setText("Only one file can be selected.");
            msgBox.exec();
            return;
        }

        std::string filename = filenames.at(0).toStdString();

        loginf << "MySQLDBImportTaskWidget: addFileSlot: file '" << filename << "'";

        if (filename.size() > 0)
        {
            if (!task_.hasFile(filename))
                task_.addFile(filename);
        }
    }
}

void MySQLDBImportTaskWidget::deleteFileSlot ()
{
    loginf << "MySQLDBImportTaskWidget: deleteFileSlot";

    if (!file_list_->currentItem() || !task_.currentFilename().size())
    {
        QMessageBox m_warning (QMessageBox::Warning, "DB File Deletion Failed",
                               "Please select a file in the list.",
                               QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    assert (task_.currentFilename().size());
    assert (task_.hasFile(task_.currentFilename()));
    task_.removeCurrentFilename();
}

void MySQLDBImportTaskWidget::selectedFileSlot ()
{
    loginf << "MySQLDBImportTaskWidget: selectedFileSlot";
    assert (file_list_->currentItem());

    QString filename = file_list_->currentItem()->text();
    assert (task_.hasFile(filename.toStdString()));
    task_.currentFilename (filename.toStdString());
}

void MySQLDBImportTaskWidget::updateFileListSlot ()
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

void MySQLDBImportTaskWidget::expertModeChangedSlot ()
{

}
