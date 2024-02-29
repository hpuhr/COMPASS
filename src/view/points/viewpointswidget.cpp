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

#include "viewpointswidget.h"
#include "viewmanager.h"
#include "viewpoint.h"
#include "logger.h"
#include "viewpointstablemodel.h"
#include "compass.h"
#include "mainwindow.h"
#include "dbcontent/dbcontentmanager.h"
#include "viewpointstoolwidget.h"
#include "viewpointsreportgenerator.h"
#include "viewpointsreportgeneratordialog.h"
//#include "dbinterface.h"

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
    connect (table_model_, &ViewPointsTableModel::typesChangedSignal,
             this, &ViewPointsWidget::typesChangedSlot);
    connect (table_model_, &ViewPointsTableModel::statusesChangedSignal,
             this, &ViewPointsWidget::statusesChangedSlot);

    proxy_model_ = new QSortFilterProxyModel();
    proxy_model_->setSourceModel(table_model_);

    typesChangedSlot(table_model_->types()); // needs first update
    statusesChangedSlot(table_model_->statuses()); // needs first update

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
    //table_view_->show();
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

        export_pdf_button_ = new QPushButton("Export PDF");
        connect (export_pdf_button_, &QPushButton::clicked, this, &ViewPointsWidget::exportPDFSlot);
        button_layout->addWidget(export_pdf_button_);

        main_layout->addLayout(button_layout);
    }

    setLayout(main_layout);

    DBContentManager& dbo_man = COMPASS::instance().dbContentManager();

    connect (&dbo_man, &DBContentManager::loadingStartedSignal, this, &ViewPointsWidget::loadingStartedSlot);
    connect (&dbo_man, &DBContentManager::loadingDoneSignal, this, &ViewPointsWidget::allLoadingDoneSlot);

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


    QObject::connect(&COMPASS::instance(), &COMPASS::databaseOpenedSignal,
                     this, &ViewPointsWidget::databaseOpenedSlot);
    QObject::connect(&COMPASS::instance(), &COMPASS::databaseClosedSignal,
                     this, &ViewPointsWidget::databaseClosedSlot);
}

ViewPointsWidget::~ViewPointsWidget()
{
}

void ViewPointsWidget::loadViewPoints()
{
    loginf << "ViewPointsWidget: loadViewPoints";

    assert (table_model_);
    table_model_->clearViewPoints();
    table_model_->loadViewPoints();

    assert (table_view_);
    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
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

QStringList ViewPointsWidget::types() const
{
    return types_;
}

QStringList ViewPointsWidget::filteredTypes() const
{
    return filtered_types_;
}

void ViewPointsWidget::filterType (QString type)
{
    assert (types_.contains(type));

    if (filtered_types_.contains(type))
        filtered_types_.removeAll(type);
    else
        filtered_types_.append(type);

    updateFilteredTypes();
}

void ViewPointsWidget::showAllTypes ()
{
    filtered_types_.clear();

    updateFilteredTypes();
}

void ViewPointsWidget::showNoTypes ()
{
    filtered_types_ = types_;

    updateFilteredTypes();
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

void ViewPointsWidget::databaseOpenedSlot()
{
    loginf << "ViewPointsWidget: databaseOpenedSlot";

    loadViewPoints();
}

void ViewPointsWidget::databaseClosedSlot()
{
    loginf << "ViewPointsWidget: databaseClosedSlot";

    assert (table_model_);
    table_model_->clearViewPoints();
}


void ViewPointsWidget::exportSlot()
{
    loginf << "ViewPointsWidget: exportSlot";

    QFileDialog dialog(nullptr);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDirectory(COMPASS::instance().lastUsedPath().c_str());
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

void ViewPointsWidget::exportPDFSlot()
{
    loginf << "ViewPointsWidget: exportPDFSlot";

    ViewPointsReportGeneratorDialog& dialog = view_manager_.viewPointsGenerator().dialog();
    dialog.exec();
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

    COMPASS::instance().mainWindow().importViewPointsSlot();

    resizeColumnsToContents();
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

    view_manager_.setCurrentViewPoint(&table_model_->viewPoint(id));
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

void ViewPointsWidget::typesChangedSlot(QStringList types)
{
    loginf << "ViewPointsWidget: typesChangedSlot";

    types_ = types;
}

void ViewPointsWidget::statusesChangedSlot(QStringList statuses)
{
    loginf << "ViewPointsWidget: statusesChangedSlot";

    statuses_ = statuses;
}

void ViewPointsWidget::updateFilteredTypes ()
{
    QStringList regex_list;

    for (auto& type : types_)
        if (!filtered_types_.contains(type))
            regex_list.append(type);

    //"^(Correlation|banana)$"
    QString regex = "^(" + regex_list.join("|") + ")$";
    loginf << "ViewPointsWidget: updateFilteredTypes: '" << regex.toStdString() << "'";

    assert (proxy_model_);
    proxy_model_->setFilterRegExp(QRegExp(regex, Qt::CaseSensitive, QRegExp::RegExp));
    proxy_model_->setFilterKeyColumn(table_model_->typeColumn()); // type
}

void ViewPointsWidget::updateFilteredStatuses ()
{
    QStringList regex_list;

    for (auto& status : statuses_)
        if (!filtered_statuses_.contains(status))
            regex_list.append(status);

    //"^(Correlation|banana)$"
    QString regex = "^(" + regex_list.join("|") + ")$";
    loginf << "ViewPointsWidget: updateFilteredStatuses: '" << regex.toStdString() << "'";

    assert (proxy_model_);
    proxy_model_->setFilterRegExp(QRegExp(regex, Qt::CaseSensitive, QRegExp::RegExp));
    proxy_model_->setFilterKeyColumn(table_model_->statusColumn()); // status
}

QStringList ViewPointsWidget::columns() const
{
    assert (table_model_);

    return table_model_->tableColumns();
}

QStringList ViewPointsWidget::filteredColumns() const
{
    return filtered_columns_;
}


void ViewPointsWidget::filterColumn(QString name)
{
    assert (columns().contains(name));

    if (filtered_columns_.contains(name))
        filtered_columns_.removeAll(name);
    else
        filtered_columns_.append(name);

    updateFilteredColumns();
}

void ViewPointsWidget::showOnlyMainColumns ()
{
    filtered_columns_.clear();

    QStringList default_cols = table_model_->defaultTableColumns();

    for (auto& col : columns())
        if (!default_cols.contains(col))
            filtered_columns_.append(col);

    updateFilteredColumns();
}

void ViewPointsWidget::showAllColumns()
{
    filtered_columns_.clear();

    updateFilteredColumns();
}

void ViewPointsWidget::showNoColumns()
{
    filtered_columns_ = columns();

    updateFilteredColumns();
}

void ViewPointsWidget::updateFilteredColumns()
{
    assert (table_view_);

    unsigned int cnt=0;

    for (auto& col : columns())
    {
        table_view_->setColumnHidden(cnt, filtered_columns_.contains(col));
        ++cnt;
    }
}

QStringList ViewPointsWidget::statuses() const
{
    return statuses_;
}

QStringList ViewPointsWidget::filteredStatuses() const
{
    return filtered_statuses_;
}

void ViewPointsWidget::filterStatus (QString status)
{
    assert (statuses_.contains(status));

    if (filtered_statuses_.contains(status))
        filtered_statuses_.removeAll(status);
    else
        filtered_statuses_.append(status);

    updateFilteredStatuses();
}

void ViewPointsWidget::showAllStatuses ()
{
    filtered_statuses_.clear();

    updateFilteredStatuses();
}

void ViewPointsWidget::showNoStatuses ()
{
    filtered_statuses_ = statuses_;

    updateFilteredStatuses();
}

std::vector<unsigned int> ViewPointsWidget::viewPoints()
{
    std::vector<unsigned int> data;

    for (auto& vp_it : table_model_->viewPoints())
        data.push_back(vp_it.id());

    return data;
}

std::vector<unsigned int> ViewPointsWidget::viewedViewPoints()
{
    std::vector<unsigned int> data;

    QModelIndex index = table_view_->model()->index(0, 0);

    while (index.isValid())
    {
        loginf << "ViewPointsWidget: viewedViewPoints: row " << index.row();

        auto const source_index = proxy_model_->mapToSource(index);
        assert (source_index.isValid());

        unsigned int id = table_model_->getIdOf(source_index);

        loginf << "ViewPointsWidget: viewedViewPoints: id " << id;

        data.push_back(id);

        index = table_view_->model()->index(index.row()+1, 0);
    }

    return data;
}

