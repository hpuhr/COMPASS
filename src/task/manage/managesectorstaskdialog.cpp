#include "managesectorstaskdialog.h"
#include "managesectorstask.h"
#include "managesectorstaskwidget.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ManageSectorsTaskDialog::ManageSectorsTaskDialog(ManageSectorsTask& task)
: QDialog(), task_(task)
{
    setWindowTitle("Manage Sectors");
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    setModal(true);

    setMinimumSize(QSize(900, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(16);

    QVBoxLayout* main_layout = new QVBoxLayout();

    task_widget_ = new ManageSectorsTaskWidget(task_, this);
    main_layout->addWidget(task_widget_);

    QHBoxLayout* button_layout = new QHBoxLayout();

    button_layout->addStretch();

    done_button_ = new QPushButton("Done");
    connect(done_button_, &QPushButton::clicked, this, &ManageSectorsTaskDialog::doneClickedSlot);
    button_layout->addWidget(done_button_);

    main_layout->addLayout(button_layout);

    setLayout(main_layout);

    update();
}

void ManageSectorsTaskDialog::updateFileList()
{
    task_widget_->updateFileListSlot();
}
void ManageSectorsTaskDialog::updateParseMessage()
{
    task_widget_->updateParseMessage();
}

void ManageSectorsTaskDialog::doneClickedSlot()
{
    emit doneSignal();
}

