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
#include <QShortcut>

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
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    //table_view_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //table_view_->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //table_view_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_view_->setIconSize(QSize(24, 24));
    table_view_->setWordWrap(true);
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

    // HACKY adjust row size but works
    connect(table_view_->horizontalHeader(), SIGNAL(sectionResized(int, int, int)),
            table_view_, SLOT(resizeRowsToContents()));


    //connect(table_view_, &QTableView::clicked, this, &ViewPointsWidget::onTableClickedSlot);

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
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

    // shortcuts
    {
        QShortcut* n_shortcut = new QShortcut(QKeySequence(tr("Down", "Next")), this);
        connect (n_shortcut, &QShortcut::activated, this, &ViewPointsWidget::selectNextSlot);

        QShortcut* o_shortcut = new QShortcut(QKeySequence(tr("O", "Open")), this);
        connect (o_shortcut, &QShortcut::activated, this, &ViewPointsWidget::setSelectedOpenSlot);

        QShortcut* c_shortcut = new QShortcut(QKeySequence(tr("C", "Closed")), this);
        connect (c_shortcut, &QShortcut::activated, this, &ViewPointsWidget::setSelectedClosedSlot);

        QShortcut* t_shortcut = new QShortcut(QKeySequence(tr("T", "Todo")), this);
        connect (t_shortcut, &QShortcut::activated, this, &ViewPointsWidget::setSelectedTodoSlot);

        QShortcut* e_shortcut = new QShortcut(QKeySequence(tr("E", "Edit")), this);
        connect (e_shortcut, &QShortcut::activated, this, &ViewPointsWidget::editCommentSlot);

        QShortcut* p_shortcut = new QShortcut(QKeySequence(tr("Up", "Previous")), this);
        connect (p_shortcut, &QShortcut::activated, this, &ViewPointsWidget::selectPreviousSlot);
    }
}

ViewPointsWidget::~ViewPointsWidget()
{
}


void ViewPointsWidget::resizeColumnsToContents()
{
    loginf << "ViewPointsWidget: resizeColumnsToContents";
    //table_model_->update();
    table_view_->resizeColumnsToContents();
}

ViewPointsTableModel* ViewPointsWidget::tableModel() const
{
    return table_model_;
}

void ViewPointsWidget::selectPreviousSlot()
{
    logdbg << "ViewPointsWidget: selectPrevious";

    if (load_in_progress_)
        return;

    QModelIndexList list = table_view_->selectionModel()->selectedRows();
    assert (list.size() <= 1);

    if (!list.size()) // none selected yet, start with first
    {
        table_view_->selectRow(table_view_->model()->rowCount()-1);
        return;
    }

    QModelIndex current_index = list.at(0);

    if (current_index.isValid())
    {
        QModelIndex next_index = table_view_->model()->index(current_index.row()-1, 0);

        if (next_index.isValid())
        {
            logdbg << "ViewPointsWidget: selectNext: setting index row " << next_index.row();
            table_view_->selectRow(next_index.row());
        }
        else
        {
            logdbg << "ViewPointsWidget: selectNext: invalid next index, step to last";
            table_view_->selectRow(table_view_->model()->rowCount()-1);
        }
    }
    else
        logwrn << "ViewPointsWidget: selectNext: invalid current index";
}

void ViewPointsWidget::selectNextSlot()
{
    logdbg << "ViewPointsWidget: selectNext";

    if (load_in_progress_)
        return;

    QModelIndexList list = table_view_->selectionModel()->selectedRows();
    assert (list.size() <= 1);

    if (!list.size()) // none selected yet, start with first
    {
        table_view_->selectRow(0);
        return;
    }

    QModelIndex current_index = list.at(0);

    if (current_index.isValid())
    {
        QModelIndex next_index = table_view_->model()->index(current_index.row()+1, 0);

        if (next_index.isValid())
        {
            logdbg << "ViewPointsWidget: selectNext: setting index row " << next_index.row();
            table_view_->selectRow(next_index.row());
        }
        else
        {
            logdbg << "ViewPointsWidget: selectNext: invalid next index, step to first";
            table_view_->selectRow(0);
        }
    }
    else
        logwrn << "ViewPointsWidget: selectNext: invalid current index";
}

void ViewPointsWidget::setSelectedOpenSlot()
{
    QModelIndexList list = table_view_->selectionModel()->selectedRows();
    assert (list.size() <= 1);

    if (!list.size()) // none selected yet, start with first
    {
        logwrn << "ViewPointsWidget: setSelectedOpenSlot: no row selected";
        return;
    }

    QModelIndex current_index = list.at(0);

    if (current_index.isValid())
    {
        auto const source_index = proxy_model_->mapToSource(current_index);
        assert (source_index.isValid());
        table_model_->setStatus(source_index, "open");
    }
    else
        logwrn << "ViewPointsWidget: setSelectedOpenSlot: invalid current index";
}

void ViewPointsWidget::setSelectedClosedSlot()
{
    QModelIndexList list = table_view_->selectionModel()->selectedRows();
    assert (list.size() <= 1);

    if (!list.size()) // none selected yet, start with first
    {
        logwrn << "ViewPointsWidget: setSelectedClosedSlot: no row selected";
        return;
    }

    QModelIndex current_index = list.at(0);

    if (current_index.isValid())
    {
        auto const source_index = proxy_model_->mapToSource(current_index);
        assert (source_index.isValid());
        table_model_->setStatus(source_index, "closed");
    }
    else
        logwrn << "ViewPointsWidget: setSelectedClosedSlot: invalid current index";
}

void ViewPointsWidget::setSelectedTodoSlot()
{
    QModelIndexList list = table_view_->selectionModel()->selectedRows();
    assert (list.size() <= 1);

    if (!list.size()) // none selected yet, start with first
    {
        logwrn << "ViewPointsWidget: setSelectedTodoSlot: no row selected";
        return;
    }

    QModelIndex current_index = list.at(0);

    if (current_index.isValid())
    {
        auto const source_index = proxy_model_->mapToSource(current_index);
        assert (source_index.isValid());
        table_model_->setStatus(source_index, "todo");
    }
    else
        logwrn << "ViewPointsWidget: setSelectedTodoSlot: invalid current index";
}

void ViewPointsWidget::editCommentSlot()
{
    QModelIndexList list = table_view_->selectionModel()->selectedRows();
    assert (list.size() <= 1);

    if (!list.size()) // none selected yet, start with first
    {
        logwrn << "ViewPointsWidget: editCommentSlot: no row selected";
        return;
    }

    QModelIndex current_index = list.at(0);

    if (current_index.isValid())
    {
        auto const source_index = proxy_model_->mapToSource(current_index);
        assert (source_index.isValid());

        QModelIndex src_comment_index = table_model_->index(source_index.row(), table_model_->commentColumn(),
                                                        QModelIndex());
        assert (src_comment_index.isValid());
        QModelIndex comment_index = proxy_model_->mapFromSource(src_comment_index);
        assert (comment_index.isValid());

        table_view_->edit(comment_index);
    }
    else
        logwrn << "ViewPointsWidget: setSelectedTodoSlot: invalid current index";
}


//void ViewPointsWidget::openCurrentSelectNext()
//{
//    loginf << "ViewPointsWidget: openCurrentSelectNext";
//}

//void ViewPointsWidget::closeCurrentSelectNext()
//{
//    loginf << "ViewPointsWidget: closeCurrentSelectNext";
//}

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
        assert (table_model_);
        table_model_->exportViewPoints(filename);
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
        assert (table_model_);
        table_model_->deleteAllViewPoints();
        resizeColumnsToContents();
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
        assert (table_model_);

        for (auto& filename : dialog.selectedFiles())
        {
            table_model_->importViewPoints(filename.toStdString());

            resizeColumnsToContents();
        }
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
    restore_focus_ = true;

    view_manager_.setCurrentViewPoint(id);
}

void ViewPointsWidget::loadingStartedSlot()
{
    load_in_progress_ = true;
    assert (tool_widget_);
    tool_widget_->setDisabled(true);
    assert (table_view_);
    table_view_->setDisabled(true);
}

void ViewPointsWidget::allLoadingDoneSlot()
{
    load_in_progress_ = false;
    assert (tool_widget_);
    tool_widget_->setDisabled(false);
    assert (table_view_);
    table_view_->setDisabled(false);

    if (restore_focus_)
    {
        setFocus();
        restore_focus_ = false;
    }
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




