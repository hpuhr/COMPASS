#include "createassociationstaskwidget.h"
#include "createassociationstask.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

CreateAssociationsTaskWidget::CreateAssociationsTaskWidget(
        CreateAssociationsTask& task, QWidget* parent, Qt::WindowFlags f)
    : TaskWidget(parent, f), task_(task)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

    expertModeChangedSlot();

    setLayout(main_layout);
}

CreateAssociationsTaskWidget::~CreateAssociationsTaskWidget() {}

void CreateAssociationsTaskWidget::expertModeChangedSlot()
{
}
