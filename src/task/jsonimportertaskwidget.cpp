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
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidget>

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

    QGridLayout *grid = new QGridLayout ();
    unsigned int row_cnt=0;

    grid->addWidget (new QLabel ("DBObject"), row_cnt, 0);

    object_box_ = new DBObjectComboBox (false);
    connect (object_box_, SIGNAL(changedObject()), this, SLOT(dbObjectChangedSlot()));
    grid->addWidget (object_box_, row_cnt, 1);

    // TODO

    main_layout->addLayout(grid);

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
