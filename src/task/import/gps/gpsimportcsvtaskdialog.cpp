#include "gpsimportcsvtaskdialog.h"
#include "gpsimportcsvtask.h"
#include "gpsimportcsvtaskwidget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

GPSImportCSVTaskDialog::GPSImportCSVTaskDialog(GPSImportCSVTask& task)
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

    task_widget_ = new GPSImportCSVTaskWidget(task_, this);
    main_layout->addWidget(task_widget_);

    QHBoxLayout* button_layout = new QHBoxLayout();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &GPSImportCSVTaskDialog::cancelClickedSlot);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    import_button_ = new QPushButton("Import");
    connect(import_button_, &QPushButton::clicked, this, &GPSImportCSVTaskDialog::importClickedSlot);
    button_layout->addWidget(import_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    update();
}

void GPSImportCSVTaskDialog::updateButtons()
{
    assert (import_button_);

    import_button_->setDisabled(!task_.canRun());
}

void GPSImportCSVTaskDialog::importClickedSlot()
{
    emit importSignal();
}

void GPSImportCSVTaskDialog::cancelClickedSlot()
{
    emit cancelSignal();
}
