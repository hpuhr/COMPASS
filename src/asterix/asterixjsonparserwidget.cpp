#include "asterixjsonparserwidget.h"
#include "asterixjsonparserdetailwidget.h"
#include "asterixjsonparser.h"
#include "logger.h"

#include <QSplitter>
#include <QSettings>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QShortcut>

ASTERIXJSONParserWidget::ASTERIXJSONParserWidget(ASTERIXJSONParser& parser, QWidget* parent)
    : QWidget(parent), parser_(parser)
{
    QVBoxLayout* main_layout = new QVBoxLayout();

    //QHBoxLayout* v_layout = new QHBoxLayout();

    splitter_ = new QSplitter();
    splitter_->setOrientation(Qt::Horizontal);

    QSettings settings("COMPASS", ("ASTERIXJSONParserWidget"+parser_.name()).c_str());

    proxy_model_ = new QSortFilterProxyModel();
    proxy_model_->setSourceModel(&parser_);

    table_view_ = new QTableView();
    table_view_->setModel(proxy_model_);
    table_view_->setSortingEnabled(true);
    table_view_->sortByColumn(0, Qt::AscendingOrder);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    table_view_->setIconSize(QSize(24, 24));
    table_view_->setWordWrap(true);
    table_view_->reset();

    connect(table_view_->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ASTERIXJSONParserWidget::currentRowChanged);

    connect(table_view_->horizontalHeader(), SIGNAL(sectionResized(int, int, int)),
            table_view_, SLOT(resizeRowsToContents()));

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
    splitter_->addWidget(table_view_);

    detail_widget_ = new ASTERIXJSONParserDetailWidget(parser_, this);

    splitter_->addWidget(detail_widget_);
    splitter_->restoreState(settings.value("mainSplitterSizes").toByteArray());

    main_layout->addWidget(splitter_);

    setLayout(main_layout);
}

ASTERIXJSONParserWidget::~ASTERIXJSONParserWidget()
{
    QSettings settings("COMPASS", ("ASTERIXJSONParserWidget"+parser_.name()).c_str());
    settings.setValue("mainSplitterSizes", splitter_->saveState());
}

void ASTERIXJSONParserWidget::resizeColumnsToContents()
{
    loginf << "ASTERIXJSONParserWidget: resizeColumnsToContents";
    //table_model_->update();
    table_view_->resizeColumnsToContents();
}

void ASTERIXJSONParserWidget::currentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
    if (!current.isValid())
    {
        loginf << "ASTERIXJSONParserWidget: currentRowChanged: invalid index";
        return;
    }

    auto const source_index = proxy_model_->mapToSource(current);
    assert (source_index.isValid());

    unsigned int index = source_index.row();

    loginf << "ASTERIXJSONParserWidget: currentRowChanged: current index " << index;

    assert (detail_widget_);
    detail_widget_->currentIndexChangedSlot(index);
}
