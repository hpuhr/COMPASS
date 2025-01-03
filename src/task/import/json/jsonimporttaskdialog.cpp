#include "jsonimporttaskdialog.h"
#include "jsonimporttaskwidget.h"
#include "jsonimporttask.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

JSONImportTaskDialog::JSONImportTaskDialog(JSONImportTask& task)
: QDialog(), task_(task)
{
    setWindowTitle("Import JSON Recording");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(1200, 800));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    task_widget_ = new JSONImportTaskWidget(task_, this);
    main_layout->addWidget(task_widget_);

    QHBoxLayout* button_layout = new QHBoxLayout();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &JSONImportTaskDialog::cancelClickedSlot);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    test_button_ = new QPushButton("Test Import");
    connect(test_button_, &QPushButton::clicked, this, &JSONImportTaskDialog::testImportClickedSlot);
    button_layout->addWidget(test_button_);

    button_layout->addStretch();

    import_button_ = new QPushButton("Import");
    connect(import_button_, &QPushButton::clicked, this, &JSONImportTaskDialog::importClickedSlot);
    button_layout->addWidget(import_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    update();
}

void JSONImportTaskDialog::updateSource()
{
    assert (task_widget_);
    task_widget_->updateSourceLabel();
}

void JSONImportTaskDialog::updateButtons()
{
    assert (import_button_);
    assert (test_button_);

    import_button_->setDisabled(!task_.canRun());
    test_button_->setDisabled(!task_.canRun());
}

void JSONImportTaskDialog::testImportClickedSlot()
{
    emit testTmportSignal();
}


void JSONImportTaskDialog::importClickedSlot()
{
    emit importSignal();
}

void JSONImportTaskDialog::cancelClickedSlot()
{
    emit cancelSignal();
}
