#include "jsonimportertaskwidget.h"
#include "jsonimportertask.h"
#include "dbobjectcombobox.h"
#include "dbovariable.h"
#include "dbovariableselectionwidget.h"
#include "logger.h"
#include "atsdb.h"
#include "stringconv.h"
#include "dbobjectmanager.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidget>
#include <QCheckBox>
#include <QLineEdit>

using namespace Utils;

JSONImporterTaskWidget::JSONImporterTaskWidget(JSONImporterTask& task, QWidget* parent, Qt::WindowFlags f)
: QWidget (parent, f), task_(task)
{
    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Import ADSBexchange JSON data");
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    QLabel *files_label = new QLabel ("File Selection");
    files_label->setFont(font_bold);
    main_layout->addWidget(files_label);

    file_list_ = new QListWidget ();
    file_list_->setWordWrap(true);
    file_list_->setTextElideMode (Qt::ElideNone);
    file_list_->setSelectionBehavior( QAbstractItemView::SelectItems );
    file_list_->setSelectionMode( QAbstractItemView::SingleSelection );
    main_layout->addWidget(file_list_);

    add_button_ = new QPushButton ("Add");
    connect (add_button_, SIGNAL(clicked()), this, SLOT(addFileSlot()));
    main_layout->addWidget(add_button_);

    delete_button_ = new QPushButton ("Remove");
    connect (delete_button_, SIGNAL(clicked()), this, SLOT(deleteFileSlot()));
    main_layout->addWidget(delete_button_);
    main_layout->addStretch();

    QFormLayout *stuff_layout = new QFormLayout;
    stuff_layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);

    object_box_ = new DBObjectComboBox (false);
    connect (object_box_, SIGNAL(changedObject()), this, SLOT(dbObjectChangedSlot()));
    stuff_layout->addRow(tr("DBObject"), object_box_);

    join_sources_check_ = new QCheckBox ();
    join_sources_check_->setChecked(task_.joinDataSources());
    connect (join_sources_check_, SIGNAL(toggled(bool)), this, SLOT(joinDataSourcesChangedSlot(bool)));
    stuff_layout->addRow("Join Data Sources", join_sources_check_);

    separate_mlat_check_ = new QCheckBox ();
    separate_mlat_check_->setChecked(task_.separateMLATData());
    connect (separate_mlat_check_, SIGNAL(toggled(bool)), this, SLOT(separateMLATChangedSlot(bool)));
    stuff_layout->addRow("Separate MLAT Data Source (if joined)", separate_mlat_check_);

    // time filter stuff
    filter_time_check_ = new QCheckBox ();
    filter_time_check_->setChecked(task_.useTimeFilter());
    connect (filter_time_check_, SIGNAL(toggled(bool)), this, SLOT(useTimeFilterChangedSlot(bool)));
    stuff_layout->addRow("Filter Time", filter_time_check_);

    filter_time_min_edit_ = new QLineEdit ();
    filter_time_min_edit_->setText(String::timeStringFromDouble(task_.timeFilterMin()).c_str());
    connect (filter_time_min_edit_, SIGNAL(textChanged(QString)), this, SLOT(timeFilterMinChangedSlot()));
    stuff_layout->addRow("Filter Time Minimum", filter_time_min_edit_);

    filter_time_max_edit_ = new QLineEdit ();
    filter_time_max_edit_->setText(String::timeStringFromDouble(task_.timeFilterMax()).c_str());
    connect (filter_time_max_edit_, SIGNAL(textChanged(QString)), this, SLOT(timeFilterMaxChangedSlot()));
    stuff_layout->addRow("Filter Time Maximum", filter_time_max_edit_);

    // position filter stuff
    filter_position_check_ = new QCheckBox ();
    filter_position_check_->setChecked(task_.usePositionFilter());
    connect (filter_position_check_, SIGNAL(toggled(bool)), this, SLOT(usePositionFilterChangedSlot(bool)));
    stuff_layout->addRow("Filter Position", filter_position_check_);

    filter_lat_min_edit_ = new QLineEdit ();
    filter_lat_min_edit_->setText(std::to_string(task_.positionFilterLatitudeMin()).c_str());
    connect (filter_lat_min_edit_, SIGNAL(textChanged(QString)), this, SLOT(positionFilterChangedSlot()));
    stuff_layout->addRow("Latitude Minimum", filter_lat_min_edit_);

    filter_lat_max_edit_ = new QLineEdit ();
    filter_lat_max_edit_->setText(std::to_string(task_.positionFilterLatitudeMax()).c_str());
    connect (filter_lat_max_edit_, SIGNAL(textChanged(QString)), this, SLOT(positionFilterChangedSlot()));
    stuff_layout->addRow("Latitude Maximum", filter_lat_max_edit_);

    filter_lon_min_edit_ = new QLineEdit ();
    filter_lon_min_edit_->setText(std::to_string(task_.positionFilterLongitudeMin()).c_str());
    connect (filter_lon_min_edit_, SIGNAL(textChanged(QString)), this, SLOT(positionFilterChangedSlot()));
    stuff_layout->addRow("Longitude Minimum", filter_lon_min_edit_);

    filter_lon_max_edit_ = new QLineEdit ();
    filter_lon_max_edit_->setText(std::to_string(task_.positionFilterLongitudeMax()).c_str());
    connect (filter_lon_max_edit_, SIGNAL(textChanged(QString)), this, SLOT(positionFilterChangedSlot()));
    stuff_layout->addRow("Longitude Maximum", filter_lon_max_edit_);

    main_layout->addLayout(stuff_layout);

    main_layout->addStretch();

    test_button_ = new QPushButton ("Test Import");
    connect(test_button_, &QPushButton::clicked, this, &JSONImporterTaskWidget::testImportSlot);
    main_layout->addWidget(test_button_);

    import_button_ = new QPushButton ("Import");
    connect(import_button_, &QPushButton::clicked, this, &JSONImporterTaskWidget::importSlot);
    main_layout->addWidget(import_button_);

    setLayout (main_layout);

    updateFileListSlot ();
    update();

    show();
}

JSONImporterTaskWidget::~JSONImporterTaskWidget()
{

}

void JSONImporterTaskWidget::addFileSlot ()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Add JSON File"));

    if (filename.size() > 0)
    {
        if (!task_.hasFile(filename.toStdString()))
            task_.addFile(filename.toStdString());
    }
}

void JSONImporterTaskWidget::deleteFileSlot ()
{
    if (!file_list_->currentItem())
    {
        QMessageBox m_warning (QMessageBox::Warning, "JSON File Deletion Failed",
                                 "Please select a file in the list.",
                                 QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    QString filename = file_list_->currentItem()->text();

    if (filename.size() > 0)
    {
        assert (task_.hasFile(filename.toStdString()));
        task_.removeFile (filename.toStdString());
    }
}

void JSONImporterTaskWidget::updateFileListSlot ()
{
    file_list_->clear();

    for (auto it : task_.fileList())
    {
        QListWidgetItem *item = new QListWidgetItem(tr(it.first.c_str()), file_list_);
        if (it.first == task_.lastFilename())
            file_list_->setCurrentItem(item);
    }
}

void JSONImporterTaskWidget::update ()
{

}

void JSONImporterTaskWidget::dbObjectChangedSlot()
{
    assert (object_box_);

    std::string object_name = object_box_->getObjectName();

    loginf << "JSONImporterTaskWidget: dbObjectChangedSlot: " << object_name;
    setDBOBject (object_name);
}

void JSONImporterTaskWidget::testImportSlot ()
{
    loginf << "JSONImporterTaskWidget: testImportSlot";

    if (!file_list_->currentItem())
    {
        QMessageBox m_warning (QMessageBox::Warning, "JSON File Test Import Failed",
                                 "Please select a file in the list.",
                                 QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    QString filename = file_list_->currentItem()->text();
    if (filename.size() > 0)
    {
        assert (task_.hasFile(filename.toStdString()));

        if (filename.endsWith(".zip") || filename.endsWith(".gz") || filename.endsWith(".tgz"))
            task_.importFileArchive(filename.toStdString(), true);
        else
            task_.importFile(filename.toStdString(), true);

        test_button_->setDisabled(true);
        import_button_->setDisabled(true);
    }
}

void JSONImporterTaskWidget::importSlot ()
{
    loginf << "JSONImporterTaskWidget: importSlot";

    if (!file_list_->currentItem())
    {
        QMessageBox m_warning (QMessageBox::Warning, "JSON File Import Failed",
                                 "Please select a file in the list.",
                                 QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    QString filename = file_list_->currentItem()->text();
    if (filename.size() > 0)
    {
        assert (task_.hasFile(filename.toStdString()));

        if (filename.endsWith(".zip") || filename.endsWith(".gz") || filename.endsWith(".tgz"))
            task_.importFileArchive(filename.toStdString(), false);
        else
            task_.importFile(filename.toStdString(), false);

        test_button_->setDisabled(true);
        import_button_->setDisabled(true);

    }
}

void JSONImporterTaskWidget::importDoneSlot (bool test)
{
    loginf << "JSONImporterTaskWidget: importDoneSlot: test " << test;
}

void JSONImporterTaskWidget::setDBOBject (const std::string& object_name)
{
    task_.dbObjectStr(object_name);

//    key_box_->showDBOOnly(object_name);
//    datasource_box_->showDBOOnly(object_name);
//    range_box_->showDBOOnly(object_name);
//    azimuth_box_->showDBOOnly(object_name);
//    altitude_box_->showDBOOnly(object_name);

//    latitude_box_->showDBOOnly(object_name);
//    longitude_box_->showDBOOnly(object_name);
}

void JSONImporterTaskWidget::joinDataSourcesChangedSlot (bool checked)
{
    task_.joinDataSources(checked);
}

void JSONImporterTaskWidget::separateMLATChangedSlot (bool checked)
{
    task_.separateMLATData(checked);
}

void JSONImporterTaskWidget::useTimeFilterChangedSlot (bool checked)
{
    task_.useTimeFilter(checked);
}

void JSONImporterTaskWidget::timeFilterMinChangedSlot ()
{
    assert (filter_time_min_edit_);

    if (filter_time_min_edit_->text().size() == 0)
        return;

    std::string tmp = filter_time_min_edit_->text().toStdString();
    float value = String::timeFromString(tmp);

    if (value > 0 && value < 24*3600)
        task_.timeFilterMin(value);
}

void JSONImporterTaskWidget::timeFilterMaxChangedSlot ()
{
    assert (filter_time_max_edit_);

    if (filter_time_max_edit_->text().size() == 0)
        return;

    std::string tmp = filter_time_max_edit_->text().toStdString();
    float value = String::timeFromString(tmp);

    if (value > 0 && value < 24*3600)
        task_.timeFilterMax(value);
}

void JSONImporterTaskWidget::usePositionFilterChangedSlot (bool checked)
{
    task_.usePositionFilter(checked);
}

void JSONImporterTaskWidget::positionFilterChangedSlot ()
{
    assert (filter_lat_min_edit_);
    assert (filter_lat_max_edit_);
    assert (filter_lon_min_edit_);
    assert (filter_lon_max_edit_);

    bool failed = false;

    try
    {
        float lat_min = std::stof(filter_lat_min_edit_->text().toStdString());

        if (lat_min > -90 && lat_min < 90)
            task_.positionFilterLatitudeMin(lat_min);
        else
            failed = true;

        float lat_max = std::stof(filter_lat_max_edit_->text().toStdString());
        if (lat_max > -90 && lat_max < 90)
            task_.positionFilterLatitudeMax(lat_max);
        else
            failed = true;

        float lon_min = std::stof(filter_lon_min_edit_->text().toStdString());
        if (lon_min > -180 && lon_min < 180)
            task_.positionFilterLongitudeMin(lon_min);
        else
            failed = true;

        float lon_max = std::stof(filter_lon_max_edit_->text().toStdString());
        if (lon_max > -180 && lon_max < 180)
            task_.positionFilterLongitudeMax(lon_max);
        else
            failed = true;
    }
    catch (std::exception& e)
    {
        loginf << "JSONImporterTaskWidget: positionFilterChangedSlot: value conversion failed";
    }

    if (failed)
        loginf << "JSONImporterTaskWidget: positionFilterChangedSlot: value conversion failed";
}
