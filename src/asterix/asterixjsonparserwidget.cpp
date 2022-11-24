#include "asterixjsonparserwidget.h"
#include "asterixjsonparserdetailwidget.h"
#include "asterixjsonparser.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variable.h"
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
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>

#include <sstream>

using namespace std;

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
    table_view_->sortByColumn(1, Qt::AscendingOrder);
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

void ASTERIXJSONParserWidget::selectModelRow (unsigned int row)
{
    assert (table_view_);
    assert (proxy_model_);

    auto const proxy_index = proxy_model_->mapFromSource(parser_.index(row, 0));

    table_view_->selectRow(proxy_index.row());

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


void ASTERIXJSONParserWidget::keyPressEvent(QKeyEvent* event)
{

    if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_C)
    {
        loginf << "ASTERIXJSONParserWidget: keyPressEvent: copy";

        unsigned int num_rows = parser_.rowCount();
        unsigned int num_cols = parser_.columnCount();

        std::ostringstream ss;

        // add header
        //        for (unsigned int col_cnt = 0; col_cnt < num_cols; ++col_cnt)
        //        {
        //            if (col_cnt != 0)
        //                ss << ";";

        //            ss << "\"" << parser_.headerData(col_cnt, Qt::Horizontal).toString().toStdString() << "\"";
        //        }

        ss << "\"Active\"";
        ss << ";\"JSON Key\"";
        ss << ";\"JSON Comment\"";
        ss << ";\"JSON Unit\"";
        ss << ";\"DBContent Variable\"";
        ss << ";\"DBContVar SN\"";
        ss << ";\"DBContVar DataType\"";
        ss << ";\"DBContVar Comment\"";
        ss << ";\"DBContVar Unit\"";
        ss << ";\"DBContVar DBColumn\"";
        ss << "\n";

        // copy content

        DBContent& dbcontent = parser_.dbContent();
        const auto& cat_info = parser_.categoryItemInfo();
        string dbovar_name;
        string json_key;

        unsigned int model_row;
        for (unsigned int row_cnt=0; row_cnt < num_rows; ++row_cnt)
        {
            QModelIndex proxy_index = proxy_model_->index(row_cnt, 0);
            assert (proxy_index.isValid());

            QModelIndex model_index = proxy_model_->mapToSource(proxy_index); // row in model
            assert (model_index.isValid());

            model_row = model_index.row();
            ASTERIXJSONParser::EntryType entry_type = parser_.entryType(model_row);

            if (entry_type == ASTERIXJSONParser::EntryType::ExistingMapping)
            {
                auto& mapping = parser_.mapping(model_row);

                dbovar_name = mapping.dboVariableName();
                json_key = mapping.jsonKey();

                ss << "\"" << (mapping.active() ? "Y" : "N") << "\""; // Active
                ss << ";\"" << json_key << "\""; // JSON Key

                if (cat_info.count(json_key))
                    ss << ";\"" << cat_info.at(json_key).description_ << "\""; // JSON Comment
                else
                    ss << ";"; // JSON Comment

                ss << ";\"" << mapping.dimensionUnitStr() << "\""; // JSON Unit

                ss << ";\"" << dbovar_name << "\""; // DBContVar

                if (dbcontent.hasVariable(dbovar_name))
                {
                    if (dbcontent.variable(dbovar_name).hasShortName())
                        ss << ";\"" << dbcontent.variable(dbovar_name).shortName() << "\""; // DBContVar SN
                    else
                        ss << ";"; // DBContVar SN

                    ss << ";\"" << dbcontent.variable(dbovar_name).dataTypeString() << "\""; // DBContVar DataType

                    ss << ";\"" << dbcontent.variable(dbovar_name).description() << "\""; // DBContVar Comment
                    ss << ";\"" << dbcontent.variable(dbovar_name).dimensionUnitStr() << "\""; // DBContVar Unit
                    ss << ";\"" << dbcontent.variable(dbovar_name).dbColumnName() << "\""; // DBContVar DBColumn
                }
                else
                {
                    ss << ";"; // DBContVar DataType
                    ss << ";"; // DBContVar Comment
                    ss << ";"; // DBContVar Unit
                    ss << ";"; // DBContVar DBColumn
                }

            }
            else if (entry_type == ASTERIXJSONParser::EntryType::UnmappedJSONKey)
            {
                json_key = parser_.unmappedJSONKey(model_row);

                ss << ""; // Active
                ss << ";\"" << json_key << "\""; // JSON Key

                if (cat_info.count(json_key))
                    ss << ";\"" << cat_info.at(json_key).description_ << "\""; // JSON Comment
                else
                    ss << ";"; // JSON Comment

                ss << ";"; // JSON Unit
                ss << ";"; // DBContVar
                ss << ";"; // DBContVar SN
                ss << ";"; // DBContVar DataType
                ss << ";"; // DBContVar Comment
                ss << ";"; // DBContVar Unit
                ss << ";"; // DBContVar DBColumn
            }
            else if (entry_type == ASTERIXJSONParser::EntryType::UnmappedDBContentVariable)
            {
                dbovar_name = parser_.unmappedDBContentVariable(model_row);

                ss << ""; // Active
                ss << ";"; // JSON Key
                ss << ";"; // JSON Comment
                ss << ";"; // JSON Unit
                ss << ";\"" << dbovar_name << "\""; // DBContVar

                if (dbcontent.hasVariable(dbovar_name))
                {
                    if (dbcontent.variable(dbovar_name).hasShortName())
                        ss << ";\"" << dbcontent.variable(dbovar_name).shortName() << "\""; // DBContVar SN
                    else
                        ss << ";"; // DBContVar SN

                    ss << ";\"" << dbcontent.variable(dbovar_name).dataTypeString() << "\""; // DBContVar DataType

                    ss << ";\"" << dbcontent.variable(dbovar_name).description() << "\""; // DBContVar Comment
                    ss << ";\"" << dbcontent.variable(dbovar_name).dimensionUnitStr() << "\""; // DBContVar Unit
                    ss << ";\"" << dbcontent.variable(dbovar_name).dbColumnName() << "\""; // DBContVar DBColumn
                }
                else
                {
                    ss << ";"; // DBContVar SN
                    ss << ";"; // DBContVar DataType
                    ss << ";"; // DBContVar Comment
                    ss << ";"; // DBContVar Unit
                    ss << ";"; // DBContVar DBColumn
                }
            }

            ss << "\n";
        }


        loginf << "'" << ss.str() << "'";

        QApplication::clipboard()->setText(ss.str().c_str());
    }
}
