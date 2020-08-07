#include "managesectorstaskwidget.h"
#include "managesectorstask.h"
#include "logger.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "sector.h"
#include "sectorlayer.h"

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
#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>

using namespace std;

ManageSectorsTaskWidget::ManageSectorsTaskWidget(ManageSectorsTask& task, QWidget* parent)
    : TaskWidget(parent), task_(task)
{
    main_layout_ = new QVBoxLayout();

    tab_widget_ = new QTabWidget();

    main_layout_->addWidget(tab_widget_);

    addImportTab();
    addManageTab();

    updateSectorTable();

    setLayout(main_layout_);
}

void ManageSectorsTaskWidget::expertModeChangedSlot() {}

void ManageSectorsTaskWidget::addImportTab()
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

        parse_msg_edit_ = new QTextEdit ();
        parse_msg_edit_->setReadOnly(true);
        main_tab_layout->addWidget(parse_msg_edit_);

        import_button_ = new QPushButton("Import");
        connect (import_button_, &QPushButton::clicked, this, &ManageSectorsTaskWidget::importSlot);
        import_button_->setDisabled(true);
        main_tab_layout->addWidget(import_button_); // is enabled in updateContext
    }

    if (task_.currentFilename().size())
        updateParseMessage();

    QWidget* main_tab_widget = new QWidget();
    main_tab_widget->setContentsMargins(0, 0, 0, 0);
    main_tab_widget->setLayout(main_tab_layout);
    tab_widget_->addTab(main_tab_widget, "Import");
}

void ManageSectorsTaskWidget::addManageTab()
{
    QVBoxLayout* manage_tab_layout = new QVBoxLayout();

    sector_table_ = new QTableWidget();
    sector_table_->setEditTriggers(QAbstractItemView::AllEditTriggers);
    sector_table_->setColumnCount(table_columns_.size());
    sector_table_->setHorizontalHeaderLabels(table_columns_);
    sector_table_->verticalHeader()->setVisible(false);
    connect(sector_table_, &QTableWidget::itemChanged, this,
            &ManageSectorsTaskWidget::sectorItemChangedSlot);

    manage_tab_layout->addWidget(sector_table_);

    QWidget* manage_tab_widget = new QWidget();
    manage_tab_widget->setContentsMargins(0, 0, 0, 0);
    manage_tab_widget->setLayout(manage_tab_layout);
    tab_widget_->addTab(manage_tab_widget, "Manage");
}

void ManageSectorsTaskWidget::updateSectorTable()
{
    logdbg << "ManageSectorsTaskWidget: updateSectorTable";

    assert(sector_table_);

    DBInterface& db_interface = ATSDB::instance().interface();
    assert (db_interface.ready());

    vector<std::shared_ptr<SectorLayer>>& sector_layers = db_interface.sectorsLayers();

    unsigned int num_layers=0;
    for (auto& sec_lay_it : sector_layers)
        num_layers += sec_lay_it->sectors().size();

    sector_table_->clearContents();
    sector_table_->setRowCount(num_layers);

    int row = 0;
    int col = 0;

    for (auto& sec_lay_it : sector_layers)
    {
        string layer_name = sec_lay_it->name();

        for (auto& sec_it : sec_lay_it->sectors())
        {
            string sector_name = sec_it->name();
            shared_ptr<Sector> sector = sec_it;

            col = 0;
            {  // Sector Name
                QTableWidgetItem* item = new QTableWidgetItem(sector_name.c_str());
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setData(Qt::UserRole, QVariant(sector_name.c_str()));
                sector_table_->setItem(row, col, item);
            }

            {  // Layer Name
                ++col;
                QTableWidgetItem* item = new QTableWidgetItem(layer_name.c_str());
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setData(Qt::UserRole, QVariant(sector_name.c_str()));
                sector_table_->setItem(row, col, item);
            }

            {  // Num Points
                ++col;
                QTableWidgetItem* item = new QTableWidgetItem(QString::number(sector->points().size()));
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setData(Qt::UserRole, QVariant(sector_name.c_str()));
                sector_table_->setItem(row, col, item);
            }

//            {  // Altitude Minimum
//                ++col;
//                if (ds_it.second.hasSac())
//                {
//                    QTableWidgetItem* item =
//                            new QTableWidgetItem(QString::number((uint)ds_it.second.sac()));
//                    item->setData(Qt::UserRole, QVariant(sector_name.c_str()));
//                    sector_table_->setItem(row, col, item);
//                }
//                else
//                {
//                    QTableWidgetItem* item = new QTableWidgetItem();
//                    item->setData(Qt::UserRole, QVariant(sector_name.c_str()));
//                    item->setBackground(Qt::darkGray);
//                    sector_table_->setItem(row, col, item);
//                }
//            }

//            {  // Altitude Maximum
//                ++col;
//                if (ds_it.second.hasSic())
//                {
//                    QTableWidgetItem* item =
//                            new QTableWidgetItem(QString::number((uint)ds_it.second.sic()));
//                    item->setData(Qt::UserRole, QVariant(sector_name.c_str()));
//                    sector_table_->setItem(row, col, item);
//                }
//                else
//                {
//                    QTableWidgetItem* item = new QTableWidgetItem();
//                    item->setData(Qt::UserRole, QVariant(sector_name.c_str()));
//                    item->setBackground(Qt::darkGray);
//                    sector_table_->setItem(row, col, item);
//                }
//            }

//            {  // Color
//                ++col;
//                if (ds_it.second.hasLatitude())
//                {
//                    QTableWidgetItem* item =
//                            new QTableWidgetItem(QString::number(ds_it.second.latitude(), 'g', 10));
//                    item->setData(Qt::UserRole, QVariant(sector_name.c_str()));
//                    sector_table_->setItem(row, col, item);
//                }
//                else
//                {
//                    QTableWidgetItem* item = new QTableWidgetItem();
//                    item->setData(Qt::UserRole, QVariant(sector_name.c_str()));
//                    item->setBackground(Qt::darkGray);
//                    sector_table_->setItem(row, col, item);
//                }
//            }

//            {  // Delete
//                ++col;
//                if (ds_it.second.hasLongitude())
//                {
//                    QTableWidgetItem* item =
//                            new QTableWidgetItem(QString::number(ds_it.second.longitude(), 'g', 10));
//                    item->setData(Qt::UserRole, QVariant(sector_name.c_str()));
//                    sector_table_->setItem(row, col, item);
//                }
//                else
//                {
//                    QTableWidgetItem* item = new QTableWidgetItem();
//                    item->setData(Qt::UserRole, QVariant(sector_name.c_str()));
//                    item->setBackground(Qt::darkGray);
//                    sector_table_->setItem(row, col, item);
//                }
//            }

            ++row;
        }
    }

    sector_table_->resizeColumnsToContents();
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

void ManageSectorsTaskWidget::updateParseMessage ()
{
    loginf << "ViewPointsImportTaskWidget: updateParseMessage";

    assert (parse_msg_edit_);
    parse_msg_edit_->setText(task_.parseMessage().c_str());

    import_button_->setEnabled(task_.canImportFile());
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

void ManageSectorsTaskWidget::importSlot()
{
    loginf << "ManageSectorsTaskWidget: importSlot";

    assert (task_.canImportFile());
    task_.importFile();

    updateSectorTable();
}

void ManageSectorsTaskWidget::sectorItemChangedSlot(QTableWidgetItem* item)
{
    loginf << "ManageSectorsTaskWidget: sectorItemChangedSlot";
}
