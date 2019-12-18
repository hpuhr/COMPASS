#include "postprocesstaskwidget.h"
#include "postprocesstask.h"
#include "atsdb.h"
#include "dbschemamanager.h"
#include "dbschemamanagerwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

PostProcessTaskWidget::PostProcessTaskWidget(PostProcessTask& task, QWidget* parent)
    : TaskWidget(parent), task_(task)
{
    QVBoxLayout* main_layout = new QVBoxLayout ();

    expertModeChangedSlot();

    setLayout (main_layout);
}

void PostProcessTaskWidget::expertModeChangedSlot ()
{

}
