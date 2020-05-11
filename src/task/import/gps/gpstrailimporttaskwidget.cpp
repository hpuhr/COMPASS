#include "gpstrailimporttaskwidget.h"
#include "gpstrailimporttask.h"
#include "logger.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>

#include <iostream>

using namespace std;

GPSTrailImportTaskWidget::GPSTrailImportTaskWidget(GPSTrailImportTask& task, QWidget* parent, Qt::WindowFlags f)
    : TaskWidget(parent, f), task_(task)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    QFont font_bold;
    font_bold.setBold(true);

    // file stuff
    {
        QLabel* files_label = new QLabel("File Selection");
        files_label->setFont(font_bold);
        main_layout->addWidget(files_label);

        file_list_ = new QListWidget();
        file_list_->setWordWrap(true);
        file_list_->setTextElideMode(Qt::ElideNone);
        file_list_->setSelectionBehavior(QAbstractItemView::SelectItems);
        file_list_->setSelectionMode(QAbstractItemView::SingleSelection);
        connect(file_list_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectedFileSlot()));

        updateFileListSlot();
        main_layout->addWidget(file_list_);
    }

    // file button stuff
    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        add_file_button_ = new QPushButton("Add");
        connect(add_file_button_, &QPushButton::clicked, this, &GPSTrailImportTaskWidget::addFileSlot);
        button_layout->addWidget(add_file_button_);

        delete_file_button_ = new QPushButton("Remove");
        connect(delete_file_button_, &QPushButton::clicked, this, &GPSTrailImportTaskWidget::deleteFileSlot);
        button_layout->addWidget(delete_file_button_);

        delete_all_files_button_ = new QPushButton("Remove All");
        connect(delete_all_files_button_, &QPushButton::clicked, this, &GPSTrailImportTaskWidget::deleteAllFilesSlot);
        button_layout->addWidget(delete_all_files_button_);

        main_layout->addLayout(button_layout);
    }

    text_edit_ = new QTextEdit ();
    text_edit_->setReadOnly(true);
    main_layout->addWidget(text_edit_);

    updateText();

    setLayout(main_layout);
}

GPSTrailImportTaskWidget::~GPSTrailImportTaskWidget() {}

void GPSTrailImportTaskWidget::addFile(const std::string& filename)
{
    if (!task_.hasFile(filename))
        task_.addFile(filename);
}

void GPSTrailImportTaskWidget::selectFile(const std::string& filename)
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

    updateText();
}

void GPSTrailImportTaskWidget::addFileSlot()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Add NMEA File"));

    if (filename.size() > 0)
        addFile(filename.toStdString());
}

void GPSTrailImportTaskWidget::deleteFileSlot()
{
    loginf << "JSONImporterTaskWidget: deleteFileSlot";

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

void GPSTrailImportTaskWidget::deleteAllFilesSlot()
{
    loginf << "GPSTrailImportTaskWidget: deleteAllFilesSlot";
    task_.removeAllFiles();
}


void GPSTrailImportTaskWidget::selectedFileSlot()
{
    logdbg << "JSONImporterTaskWidget: selectedFileSlot";
    assert(file_list_->currentItem());

    QString filename = file_list_->currentItem()->text();
    assert(task_.hasFile(filename.toStdString()));

    task_.currentFilename(filename.toStdString());

    updateText();
}

void GPSTrailImportTaskWidget::updateFileListSlot()
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

void GPSTrailImportTaskWidget::expertModeChangedSlot() {}

//void GPSTrailImportTaskWidget::runStarted()
//{
//    loginf << "GPSTrailImportTaskWidget: runStarted";

//    //test_button_->setDisabled(true);
//}

//void GPSTrailImportTaskWidget::runDone()
//{
//    loginf << "GPSTrailImportTaskWidget: runDone";

//    //test_button_->setDisabled(false);
//}

void GPSTrailImportTaskWidget::updateText ()
{
    loginf << "ViewPointsImportTaskWidget: updateText";

    assert (text_edit_);

    stringstream ss;

    if (task_.currentError().size())
        ss << "Errors:\n" << task_.currentError();
    else
        ss << "Errors: None\n";

    if (task_.currentText().size())
        ss << "Info:\n" << task_.currentText();
    else
        ss << "Info: None\n";

    text_edit_->setText(ss.str().c_str());

//    if (task_.currentError().size())
//    {
//        context_edit_->setText(QString("Error: ")+task_.currentError().c_str());
//        import_button_->setDisabled(true);
//    }
//    else
//    {
//        const nlohmann::json& data = task_.currentData();

//        assert (data.contains("view_point_context"));

//        context_edit_->setText(data.at("view_point_context").dump(4).c_str());

//        import_button_->setDisabled(false);
//    }
}
