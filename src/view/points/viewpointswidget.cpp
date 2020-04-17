#include "viewpointswidget.h"
#include "viewmanager.h"
#include "viewpoint.h"
#include "logger.h"
#include "viewpointstablemodel.h"

#include <QTableView>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QFileDialog>


ViewPointsWidget::ViewPointsWidget(ViewManager& view_manager)
    : QWidget(), view_manager_(view_manager)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    table_model_ = new ViewPointsTableModel(view_manager_);

    QSortFilterProxyModel* proxy_model = new QSortFilterProxyModel();
    proxy_model->setSourceModel(table_model_);

    table_view_ = new QTableView();
    table_view_->setModel(proxy_model);
    table_view_->setSortingEnabled(true);
    table_view_->sortByColumn(0, Qt::AscendingOrder);
    table_view_->reset();
    table_view_->show();
//    table_->setEditTriggers(QAbstractItemView::AllEditTriggers);
//    table_->setColumnCount(table_columns_.size());
//    table_->setHorizontalHeaderLabels(table_columns_);
//    //table_->verticalHeader()->setVisible(false);
//    connect(table_, &QTableWidget::itemChanged, this,
//            &DBOEditDataSourcesWidget::configItemChangedSlot);
    // update done later

    table_view_->resizeColumnsToContents();
    main_layout->addWidget(table_view_);

    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        export_button_ = new QPushButton("Export");
        connect (export_button_, &QPushButton::clicked, this, &ViewPointsWidget::exportSlot);
        button_layout->addWidget(export_button_);

        import_button_ = new QPushButton("Import");
        connect (import_button_, &QPushButton::clicked, this, &ViewPointsWidget::importSlot);
        button_layout->addWidget(import_button_);

        main_layout->addLayout(button_layout);
    }

    setLayout(main_layout);
}

ViewPointsWidget::~ViewPointsWidget()
{

}


void ViewPointsWidget::update()
{
    loginf << "ViewPointsWidget: update";
    table_model_->update();
}

void ViewPointsWidget::exportSlot()
{
    loginf << "ViewPointsWidget: exportSlot";

    QFileDialog dialog(nullptr);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter("CSV Files (*.csv)");
    dialog.setDefaultSuffix("csv");
    dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);

    if (!overwrite)
        dialog.setOption(QFileDialog::DontConfirmOverwrite);

    QStringList file_names;
    if (dialog.exec())
        file_names = dialog.selectedFiles();

    QString filename;


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
            addFile(filename.toStdString());
    }
}


void ViewPointsWidget::importFile (const std::string& filename)
{

}



