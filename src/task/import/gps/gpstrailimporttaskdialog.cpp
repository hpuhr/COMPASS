#include "gpstrailimporttaskdialog.h"
#include "gpstrailimporttask.h"
#include "gpstrailimporttaskwidget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

GPSTrailImportTaskDialog::GPSTrailImportTaskDialog(GPSTrailImportTask& task)
: QDialog(), task_(task)
{
    setWindowTitle("Import GPS Trail");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    task_widget_ = new GPSTrailImportTaskWidget(task_, this);
    main_layout->addWidget(task_widget_);

    QHBoxLayout* button_layout = new QHBoxLayout();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &GPSTrailImportTaskDialog::cancelClickedSlot);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    import_button_ = new QPushButton("Import");
    connect(import_button_, &QPushButton::clicked, this, &GPSTrailImportTaskDialog::importClickedSlot);
    button_layout->addWidget(import_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    connect(&task_, &GPSTrailImportTask::fileChanged, this, &GPSTrailImportTaskDialog::updateButtons);

    update();
}

void GPSTrailImportTaskDialog::updateButtons()
{
    assert (import_button_);

    import_button_->setDisabled(!task_.canRun());
}

void GPSTrailImportTaskDialog::importClickedSlot()
{
    emit importSignal();
}

void GPSTrailImportTaskDialog::cancelClickedSlot()
{
    emit cancelSignal();
}
