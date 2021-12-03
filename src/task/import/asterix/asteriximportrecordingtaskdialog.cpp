#include "asteriximportrecordingtaskdialog.h"
#include "asteriximporttaskwidget.h"
#include "asteriximporttask.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ASTERIXImportRecordingTaskDialog::ASTERIXImportRecordingTaskDialog(ASTERIXImportTask& task)
: QDialog(), task_(task)
{
    setWindowTitle("Import ASTERIX Recording");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(1000, 800));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    task_widget_ = new ASTERIXImportTaskWidget(task_, this);
    main_layout->addWidget(task_widget_);

    QHBoxLayout* button_layout = new QHBoxLayout();

    cancel_button_ = new QPushButton("Cancel");
    connect(cancel_button_, &QPushButton::clicked, this, &ASTERIXImportRecordingTaskDialog::cancelClickedSlot);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    test_button_ = new QPushButton("Test Import");
    connect(test_button_, &QPushButton::clicked, this, &ASTERIXImportRecordingTaskDialog::testImportClickedSlot);
    button_layout->addWidget(test_button_);

    button_layout->addStretch();

    import_button_ = new QPushButton("Import");
    connect(import_button_, &QPushButton::clicked, this, &ASTERIXImportRecordingTaskDialog::importClickedSlot);
    button_layout->addWidget(import_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    update();
}

void ASTERIXImportRecordingTaskDialog::updateButtons()
{
    assert (import_button_);
    assert (test_button_);

    import_button_->setDisabled(!task_.canRun());
    import_button_->setDisabled(!task_.canRun());
}

void ASTERIXImportRecordingTaskDialog::testImportClickedSlot()
{
    emit testTmportSignal();
}


void ASTERIXImportRecordingTaskDialog::importClickedSlot()
{
    emit importSignal();
}

void ASTERIXImportRecordingTaskDialog::cancelClickedSlot()
{
    emit cancelSignal();
}
