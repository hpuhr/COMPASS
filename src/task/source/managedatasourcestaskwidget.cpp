#include "managedatasourcestaskwidget.h"
#include "managedatasourcestask.h"
#include "atsdb.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "dboeditdatasourceswidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

ManageDataSourcesTaskWidget::ManageDataSourcesTaskWidget(ManageDataSourcesTask& task, QWidget* parent)
    : TaskWidget(parent), task_(task)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout_ = new QVBoxLayout ();

    tab_widget_ = new QTabWidget ();
    main_layout_->addWidget(tab_widget_);

    for (auto& dbo_it : ATSDB::instance().objectManager())
    {
        tab_widget_->addTab(task_.editDataSourcesWidget(dbo_it.first), dbo_it.first.c_str());
        connect (task_.editDataSourcesWidget(dbo_it.first), &DBOEditDataSourcesWidget::dbItemChangedSignal,
                 this, &ManageDataSourcesTaskWidget::dbItemChangedSlot);
    }

    expertModeChangedSlot();

    // bottom buttons
    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        QLabel *button_label = new QLabel ("Configuration Data Sources");
        button_label->setFont (font_bold);
        button_layout->addWidget (button_label);

        QPushButton* export_button_ = new QPushButton ("Export All");
        connect(export_button_, &QPushButton::clicked, this, &ManageDataSourcesTaskWidget::exportConfigDataSourcesSlot);
        button_layout->addWidget(export_button_);

        QPushButton* clear_button_ = new QPushButton ("Clear All");
        connect(clear_button_, &QPushButton::clicked, this, &ManageDataSourcesTaskWidget::clearConfigDataSourcesSlot);
        button_layout->addWidget(clear_button_);

        QPushButton* import_button_ = new QPushButton ("Import");
        connect(import_button_, &QPushButton::clicked, this, &ManageDataSourcesTaskWidget::importConfigDataSourcesSlot);
        button_layout->addWidget(import_button_);

        main_layout_->addLayout(button_layout);
    }

    setLayout (main_layout_);
}

void ManageDataSourcesTaskWidget::expertModeChangedSlot ()
{
}

void ManageDataSourcesTaskWidget::exportConfigDataSourcesSlot ()
{
    loginf << "ManageDataSourcesTaskWidget: exportConfigDataSourcesSlot";
    task_.exportConfigDataSources();
}

void ManageDataSourcesTaskWidget::clearConfigDataSourcesSlot ()
{
    loginf << "ManageDataSourcesTaskWidget: clearConfigDataSourcesSlot";
    task_.clearConfigDataSources();
}

void ManageDataSourcesTaskWidget::importConfigDataSourcesSlot ()
{
    loginf << "ManageDataSourcesTaskWidget: importConfigDataSourcesSlot";
    task_.importConfigDataSources();
}

void ManageDataSourcesTaskWidget::dbItemChangedSlot ()
{
    loginf << "ManageDataSourcesTaskWidget: dbItemChangedSlot";
    emit task_.statusChangedSignal(task_.name());
}

