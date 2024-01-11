#include "fftsconfigurationdialog.h"
#include "ffttablemodel.h"
//#include "ffteditwidget.h"
#include "fftmanager.h"
//#include "fftcreatedialog.h"
#include "util/number.h"
#include "logger.h"
#include "compass.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>

using namespace std;
using namespace Utils;

FFTsConfigurationDialog::FFTsConfigurationDialog(FFTManager& ds_man)
    : QDialog(), ds_man_(ds_man)
{
    setWindowTitle("Configure FFTs");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(1000, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    QHBoxLayout* top_layout = new QHBoxLayout();

    table_model_ = new FFTTableModel(ds_man_, *this);

    proxy_model_ = new QSortFilterProxyModel();
    proxy_model_->setSourceModel(table_model_);

    table_view_ = new QTableView();
    table_view_->setModel(proxy_model_);
    table_view_->setSortingEnabled(true);
    table_view_->sortByColumn(2, Qt::AscendingOrder);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    //table_view_->setIconSize(QSize(24, 24));
    table_view_->setWordWrap(true);
    table_view_->reset();

    connect(table_view_->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &FFTsConfigurationDialog::currentRowChanged);

    // HACKY adjust row size but works
    connect(table_view_->horizontalHeader(), SIGNAL(sectionResized(int, int, int)),
            table_view_, SLOT(resizeRowsToContents()));

    table_view_->resizeColumnsToContents();
    table_view_->resizeRowsToContents();
    top_layout->addWidget(table_view_);

//    edit_widget_ = new FFTEditWidget (ds_man_, *this);
//    top_layout->addWidget(edit_widget_);

    main_layout->addLayout(top_layout);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    main_layout->addWidget(line);

    QHBoxLayout* button_layout = new QHBoxLayout();

    QPushButton* new_button = new QPushButton ("Create New");
    new_button->setToolTip("Allows creation of new FFT");
    connect(new_button, &QPushButton::clicked, this, &FFTsConfigurationDialog::newFFTClickedSlot);
    button_layout->addWidget(new_button);

    button_layout->addStretch();

    QPushButton* import_button = new QPushButton ("Import");
    import_button->setToolTip("Imports FFTs as JSON file. The FFTs defined in the JSON file"
                              " will override existing ones (with same name) in the configuration and database");
    connect(import_button, &QPushButton::clicked, this, &FFTsConfigurationDialog::importClickedSlot);
    button_layout->addWidget(import_button);

    button_layout->addSpacing(32);

    QPushButton* delete_all_button = new QPushButton("Delete All");
    delete_all_button->setToolTip("Deletes all FFTs existing in the configuration,"
                                  " but not those (also) defined in the database");
    connect(delete_all_button, &QPushButton::clicked, this, &FFTsConfigurationDialog::deleteAllClickedSlot);
    button_layout->addWidget(delete_all_button);

    button_layout->addSpacing(32);

    QPushButton* export_button = new QPushButton ("Export");
    export_button->setToolTip("Exports all FFTs as JSON file");
    connect(export_button, &QPushButton::clicked, this, &FFTsConfigurationDialog::exportClickedSlot);
    button_layout->addWidget(export_button);

    button_layout->addStretch();

    QPushButton* done_button = new QPushButton("Done");
    connect(done_button, &QPushButton::clicked, this, &FFTsConfigurationDialog::doneClickedSlot);
    button_layout->addWidget(done_button);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);
}

void FFTsConfigurationDialog::updateFFT(const std::string& name)
{
    table_model_->updateFFT(name);
}

void FFTsConfigurationDialog::beginResetModel()
{
    table_model_->beginModelReset();
}

void FFTsConfigurationDialog::endResetModel()
{
    table_model_->endModelReset();
}

void FFTsConfigurationDialog::currentRowChanged(const QModelIndex& current, const QModelIndex& previous)
{
//    assert (edit_widget_);

//    if (!current.isValid())
//    {
//        loginf << "FFTsConfigurationDialog: currentRowChanged: invalid index";

//        edit_widget_->clear();

//        return;
//    }

//    auto const source_index = proxy_model_->mapToSource(current);
//    assert (source_index.isValid());

//    unsigned int id = table_model_->getIdOf(source_index);

//    loginf << "FFTsConfigurationDialog: currentRowChanged: current id " << id;

//    edit_widget_->showID(id);
}

void FFTsConfigurationDialog::newFFTClickedSlot()
{
    loginf << "FFTsConfigurationDialog: newFFTClickedSlot";

//    create_dialog_.reset(new FFTCreateDialog(*this, ds_man_));
//    connect(create_dialog_.get(), &FFTCreateDialog::doneSignal,
//            this, &FFTsConfigurationDialog::newDSDoneSlot);

//    create_dialog_->show();
}

void FFTsConfigurationDialog::newFFTDoneSlot()
{
    loginf << "FFTsConfigurationDialog: newFFTDoneSlot";

//    assert (create_dialog_);

//    if (!create_dialog_->cancelled())
//    {
//        string ds_type = create_dialog_->dsType();

//        unsigned int sac = create_dialog_->sac();
//        unsigned int sic = create_dialog_->sic();

//        loginf << "FFTsConfigurationDialog: newDSDoneSlot: ds_type " << ds_type
//               << " sac " << sac << " sic " << sic;

//        unsigned int ds_id = Number::dsIdFrom(sac, sic);

//        assert (!ds_man_.hasConfigFFT(ds_id));

//        beginResetModel();

//        ds_man_.createConfigFFT(ds_id);
//        assert (ds_man_.hasConfigFFT(ds_id));
//        ds_man_.configFFT(ds_id).dsType(ds_type);

//        endResetModel();

//        auto const model_index = table_model_->dataSourceIndex(ds_id);

//        auto const source_index = proxy_model_->mapFromSource(model_index);
//        assert (source_index.isValid());

//        table_view_->selectRow(source_index.row());
//    }

//    create_dialog_->close();
//    create_dialog_ = nullptr;
}

void FFTsConfigurationDialog::importClickedSlot()
{
    loginf << "FFTsConfigurationDialog: importClickedSlot";

    string filename = QFileDialog::getOpenFileName(
                this, "Import FFTs",
                COMPASS::instance().lastUsedPath().c_str(), "*.json").toStdString();

    if (filename.size() > 0)
    {
        table_model_->beginModelReset();

//        ds_man_.importFFTs(filename);

        table_model_->endModelReset();

//        edit_widget_->updateContent();
    }
}

void FFTsConfigurationDialog::deleteAllClickedSlot()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
                nullptr, "Delete All FFTs",
                "This will delete all FFTs existing in the configuration,"
                         " but not those (also) defined in the database. Do you want to continue?",
                QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        loginf << "FFTsConfigurationDialog: deleteAllClickedSlot: deletion confirmed";

        table_model_->beginModelReset();

//        ds_man_.deleteAllConfigFFTs();
//        edit_widget_->clear();

        table_model_->endModelReset();
    }
}

void FFTsConfigurationDialog::exportClickedSlot()
{
    loginf << "FFTsConfigurationDialog: exportClickedSlot";

    string filename = QFileDialog::getSaveFileName(
                this, "Export FFTs as JSON",
                COMPASS::instance().lastUsedPath().c_str(), "*.json").toStdString();

    if (filename.size() > 0)
    {
        loginf << "FFTsConfigurationDialog: exportClickedSlot: file '" << filename << "'";

//        ds_man_.exportFFTs(filename);
    }
}

void FFTsConfigurationDialog::doneClickedSlot()
{
    emit doneSignal();
}
