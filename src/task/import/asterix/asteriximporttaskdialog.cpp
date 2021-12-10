#include "asteriximporttaskdialog.h"
#include "asteriximporttaskwidget.h"
#include "asteriximporttask.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ASTERIXImportTaskDialog::ASTERIXImportTaskDialog(ASTERIXImportTask& task)
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
    connect(cancel_button_, &QPushButton::clicked, this, &ASTERIXImportTaskDialog::cancelClickedSlot);
    button_layout->addWidget(cancel_button_);

    button_layout->addStretch();

    test_button_ = new QPushButton("Test Import");
    connect(test_button_, &QPushButton::clicked, this, &ASTERIXImportTaskDialog::testImportClickedSlot);
    button_layout->addWidget(test_button_);

    button_layout->addStretch();

    import_button_ = new QPushButton("Import");
    connect(import_button_, &QPushButton::clicked, this, &ASTERIXImportTaskDialog::importClickedSlot);
    button_layout->addWidget(import_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    update();
}

void ASTERIXImportTaskDialog::updateButtons()
{
    assert (import_button_);
    assert (test_button_);

    if (task_.isImportNetwork()) // import from network
    {
        import_button_->setDisabled(!task_.canRun());
        test_button_->setDisabled(true);
    }
    else // import file
    {
        import_button_->setDisabled(!task_.canRun());
        test_button_->setDisabled(!task_.canRun());
    }
}

void ASTERIXImportTaskDialog::testImportClickedSlot()
{
    emit testTmportSignal();
}


void ASTERIXImportTaskDialog::importClickedSlot()
{
    emit importSignal();
}

void ASTERIXImportTaskDialog::cancelClickedSlot()
{
    emit cancelSignal();
}
