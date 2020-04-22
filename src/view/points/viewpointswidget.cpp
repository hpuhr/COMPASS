#include "viewpointswidget.h"
#include "viewmanager.h"
#include "viewpoint.h"
#include "logger.h"
#include "viewpointstablemodel.h"
#include "atsdb.h"
#include "dbobjectmanager.h"
#include "viewpointstoolwidget.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QMessageBox>

ViewPointsWidget::ViewPointsWidget(ViewManager& view_manager)
    : QWidget(), view_manager_(view_manager)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    tool_widget_ = new ViewPointsToolWidget(this);
    main_layout->addWidget(tool_widget_);

    table_model_ = new ViewPointsTableModel(view_manager_);

    proxy_model_ = new QSortFilterProxyModel();
    proxy_model_->setSourceModel(table_model_);

    table_view_ = new QTableView();
    table_view_->setModel(proxy_model_);
    table_view_->setSortingEnabled(true);
    table_view_->sortByColumn(0, Qt::AscendingOrder);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->reset();
    table_view_->show();
//    table_->setEditTriggers(QAbstractItemView::AllEditTriggers);
//    table_->setColumnCount(table_columns_.size());
//    table_->setHorizontalHeaderLabels(table_columns_);
//    //table_->verticalHeader()->setVisible(false);
//    connect(table_, &QTableWidget::itemChanged, this,
//            &DBOEditDataSourcesWidget::configItemChangedSlot);
    // update done later

    connect(table_view_->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ViewPointsWidget::currentRowChanged);

    //connect(table_view_, &QTableView::clicked, this, &ViewPointsWidget::onTableClickedSlot);

    table_view_->resizeColumnsToContents();
    main_layout->addWidget(table_view_);

    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        import_button_ = new QPushButton("Import");
        connect (import_button_, &QPushButton::clicked, this, &ViewPointsWidget::importSlot);
        button_layout->addWidget(import_button_);

        delete_all_button_ = new QPushButton("Delete All");
        connect (delete_all_button_, &QPushButton::clicked, this, &ViewPointsWidget::deleteAllSlot);
        button_layout->addWidget(delete_all_button_);

        export_button_ = new QPushButton("Export");
        connect (export_button_, &QPushButton::clicked, this, &ViewPointsWidget::exportSlot);
        button_layout->addWidget(export_button_);


        main_layout->addLayout(button_layout);
    }

    setLayout(main_layout);

    DBObjectManager& dbo_man = ATSDB::instance().objectManager();

    connect (&dbo_man, &DBObjectManager::loadingStartedSignal, this, &ViewPointsWidget::loadingStartedSlot);
    connect (&dbo_man, &DBObjectManager::allLoadingDoneSignal, this, &ViewPointsWidget::allLoadingDoneSlot);
}

ViewPointsWidget::~ViewPointsWidget()
{

}


void ViewPointsWidget::update()
{
    loginf << "ViewPointsWidget: update";
    table_model_->update();
    table_view_->resizeColumnsToContents();
}

void ViewPointsWidget::exportSlot()
{
    loginf << "ViewPointsWidget: exportSlot";

    QFileDialog dialog(nullptr);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("JSON Files (*.json)");
    dialog.setDefaultSuffix("json");
    dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);

//    if (!overwrite)
//        dialog.setOption(QFileDialog::DontConfirmOverwrite);

    if (dialog.exec())
    {
        QStringList file_names = dialog.selectedFiles();
        assert (file_names.size() == 1);
        std::string filename = file_names.at(0).toStdString();

        loginf << "ViewPointsWidget: exportSlot: filename '" << filename << "'";
        view_manager_.exportViewPoints(filename);
    }
}

void ViewPointsWidget::deleteAllSlot()
{
    loginf << "ViewPointsWidget: deleteAllSlot";

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Delete All", "Delete All Viewpoints?",
    QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        view_manager_.deleteAllViewPoints();
        update();
    }
}

void ViewPointsWidget::importSlot()
{
    loginf << "ViewPointsWidget: importSlot";

    QFileDialog dialog(this);
    dialog.setWindowTitle("Import View Points JSON");
    // dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(trUtf8("JSON (*.json)"));

    QStringList fileNames;
    if (dialog.exec())
    {
        for (auto& filename : dialog.selectedFiles())
            view_manager_.importViewPoints(filename.toStdString());
    }
}

void ViewPointsWidget::currentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (!current.isValid())
    {
        loginf << "ViewPointsWidget: currentRowChanged: invalid index";
        return;
    }

    auto const source_index = proxy_model_->mapToSource(current);
    assert (source_index.isValid());

    unsigned int id = table_model_->getIdOf(source_index);

    loginf << "ViewPointsWidget: currentRowChanged: current id " << id;
    view_manager_.setCurrentViewPoint(id);
}

void ViewPointsWidget::loadingStartedSlot()
{
    assert (tool_widget_);
    tool_widget_->setDisabled(true);
    assert (table_view_);
    table_view_->setDisabled(true);
}

void ViewPointsWidget::allLoadingDoneSlot()
{
    assert (tool_widget_);
    tool_widget_->setDisabled(false);
    assert (table_view_);
    table_view_->setDisabled(false);
}

//void ViewPointsWidget::onTableClickedSlot (const QModelIndex& current)
//{
//    if (!current.isValid())
//    {
//        loginf << "ViewPointsWidget: onTableClickedSlot: invalid index";
//        return;
//    }

//    auto const source_index = proxy_model_->mapToSource(current);
//    assert (source_index.isValid());

//    unsigned int id = table_model_->getIdOf(source_index);

//    loginf << "ViewPointsWidget: onTableClickedSlot: current id " << id;
//    view_manager_.setCurrentViewPoint(id);
//}




