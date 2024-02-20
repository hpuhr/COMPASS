#include "asteriximporttaskdialog.h"
#include "asteriximporttaskwidget.h"
#include "asteriximporttask.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ASTERIXImportTaskDialog::ASTERIXImportTaskDialog(ASTERIXImportTask& task, 
                                                 QWidget* parent)
:   QDialog(parent)
,   task_  (task  )
{
    setWindowTitle("Import ASTERIX Recording");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(1200, 800));

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

    import_button_ = new QPushButton("Import");
    connect(import_button_, &QPushButton::clicked, this, &ASTERIXImportTaskDialog::importClickedSlot);
    button_layout->addWidget(import_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    update();
    updateTitle();
    updateButtons();

    connect(&task, &ASTERIXImportTask::configChanged, this, &ASTERIXImportTaskDialog::configChanged);
    connect(&task, &ASTERIXImportTask::decodingStateChanged, this, &ASTERIXImportTaskDialog::decodingStateChangedSlot);
}

void ASTERIXImportTaskDialog::updateTitle()
{
    setWindowTitle("Import ASTERIX From " + QString::fromStdString(task_.source().sourceTypeAsString()));
}

void ASTERIXImportTaskDialog::updateSourcesInfo()
{
    updateTitle();

    assert (task_widget_);
    task_widget_->updateSourcesGrid();
}

void ASTERIXImportTaskDialog::updateButtons()
{
    assert (import_button_);

    import_button_->setDisabled(!task_.canRun());
}

void ASTERIXImportTaskDialog::importClickedSlot()
{
    accept();
}

void ASTERIXImportTaskDialog::cancelClickedSlot()
{
    reject();
}

void ASTERIXImportTaskDialog::configChanged()
{
    //@TODO
}

void ASTERIXImportTaskDialog::decodingStateChangedSlot()
{
    updateTitle();
    updateButtons();
}
