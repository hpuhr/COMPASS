#include "managesectorstaskwidget.h"
#include "managesectorstask.h"
#include "logger.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

ManageSectorsTaskWidget::ManageSectorsTaskWidget(ManageSectorsTask& task, QWidget* parent)
    : TaskWidget(parent), task_(task)
{
    main_layout_ = new QHBoxLayout();

    tab_widget_ = new QTabWidget();

    main_layout_->addWidget(tab_widget_);

    addMainTab();

    setLayout(main_layout_);
}

void ManageSectorsTaskWidget::expertModeChangedSlot() {}

void ManageSectorsTaskWidget::addMainTab()
{
    assert(tab_widget_);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_tab_layout = new QVBoxLayout();

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
        connect(add_file_button_, &QPushButton::clicked, this, &ManageSectorsTaskWidget::addFileSlot);
        button_layout->addWidget(add_file_button_);

        delete_file_button_ = new QPushButton("Remove");
        connect(delete_file_button_, &QPushButton::clicked, this, &ManageSectorsTaskWidget::deleteFileSlot);
        button_layout->addWidget(delete_file_button_);

        delete_all_files_button_ = new QPushButton("Remove All");
        connect(delete_all_files_button_, &QPushButton::clicked, this, &ManageSectorsTaskWidget::deleteAllFilesSlot);
        button_layout->addWidget(delete_all_files_button_);

        files_layout->addLayout(button_layout);

        main_tab_layout->addLayout(files_layout);
    }

    // final stuff
    {
//        debug_check_ = new QCheckBox("Debug in Console");
//        debug_check_->setChecked(task_.debug());
//        connect(debug_check_, &QCheckBox::clicked, this,
//                &ManageSectorsTaskWidget::debugChangedSlot);
//        main_tab_layout->addWidget(debug_check_);

//        limit_ram_check_ = new QCheckBox("Limit RAM Usage");
//        limit_ram_check_->setChecked(task_.limitRAM());
//        connect(limit_ram_check_, &QCheckBox::clicked, this,
//                &ManageSectorsTaskWidget::limitRAMChangedSlot);
//        main_tab_layout->addWidget(limit_ram_check_);

//        create_mapping_stubs_button_ = new QPushButton("Create Mapping Stubs");
//        connect(create_mapping_stubs_button_, &QPushButton::clicked, this,
//                &ManageSectorsTaskWidget::createMappingsSlot);
//        main_tab_layout->addWidget(create_mapping_stubs_button_);

//        test_button_ = new QPushButton("Test Import");
//        connect(test_button_, &QPushButton::clicked, this,
//                &ManageSectorsTaskWidget::testImportSlot);
//        main_tab_layout->addWidget(test_button_);
    }

    QWidget* main_tab_widget = new QWidget();
    main_tab_widget->setContentsMargins(0, 0, 0, 0);
    main_tab_widget->setLayout(main_tab_layout);
    tab_widget_->addTab(main_tab_widget, "Main");
}

void ManageSectorsTaskWidget::addFileSlot()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle("Add Sector File(s)");
    // dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFiles);
    // dialog.setNameFilter(trUtf8("Splits (*.000 *.001)"));
    QStringList fileNames;
    if (dialog.exec())
    {
        for (auto& filename : dialog.selectedFiles())
            addFile(filename.toStdString());
    }
}

void ManageSectorsTaskWidget::addFile(const std::string& filename)
{
    if (!task_.hasFile(filename))
        task_.addFile(filename);
}

void ManageSectorsTaskWidget::selectFile(const std::string& filename)
{
    assert(task_.hasFile(filename));
    task_.currentFilename(filename);

    QList<QListWidgetItem*> items = file_list_->findItems(filename.c_str(), Qt::MatchExactly);
    assert (items.size() > 0);

    for (auto item_it : items)
    {
        assert (item_it);
        file_list_->setCurrentItem(item_it);
    }
}

void ManageSectorsTaskWidget::deleteFileSlot()
{
    loginf << "ManageSectorsTaskWidget: deleteFileSlot";

    if (!file_list_->currentItem() || !task_.currentFilename().size())
    {
        QMessageBox m_warning(QMessageBox::Warning, "Sector File Deletion Failed",
                              "Please select a file in the list.", QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    assert(task_.currentFilename().size());
    assert(task_.hasFile(task_.currentFilename()));
    task_.removeCurrentFilename();
}

void ManageSectorsTaskWidget::deleteAllFilesSlot()
{
    loginf << "ManageSectorsTaskWidget: deleteAllFilesSlot";
    task_.removeAllFiles();
}

void ManageSectorsTaskWidget::selectedFileSlot()
{
    loginf << "ManageSectorsTaskWidget: selectedFileSlot";
    assert(file_list_->currentItem());

    QString filename = file_list_->currentItem()->text();
    assert(task_.hasFile(filename.toStdString()));
    task_.currentFilename(filename.toStdString());
}

void ManageSectorsTaskWidget::updateFileListSlot()
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
