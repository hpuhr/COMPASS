/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * MainWindow.cpp
 *
 *  Created on: Aug 2, 2011
 *      Author: sk
 */

#include <boost/bind.hpp>

#include <QMenuBar>
#include <QMessageBox>
#include <QAction>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QLineEdit>
#include <QGridLayout>
#include <QSettings>
#include <QThread>
#include <QTabWidget>

//#include "Buffer.h"
#include "mainwindow.h"
#include "global.h"
#include "logger.h"
#include "config.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbobjectmanagerwidget.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dbinterfacewidget.h"
#include "dbschemamanager.h"
#include "dbschemamanagerwidget.h"
#include "managementwidget.h"
#include "stringconv.h"
#include "jobmanager.h"
#include "viewmanager.h"
//#include "ProjectionManager.h"
//#include "ProjectionManagerWidget.h"

using namespace Utils;
using namespace std;

//namespace ATSDB
//{

MainWindow::MainWindow()
    : tab_widget_(nullptr), dbinterface_widget_(nullptr), dbschema_manager_widget_(nullptr),  object_manager_widget_ (nullptr), management_widget_(nullptr), start_button_(nullptr)
{
    logdbg  << "MainWindow: constructor";

    QSettings settings("ATSDB", "Client");
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());

    createMenus();

    tab_widget_ = new QTabWidget ();

    QWidget *main_widget = new QWidget ();
    QVBoxLayout *main_layout = new QVBoxLayout ();

    // for se widgets
    QHBoxLayout *widget_layout = new QHBoxLayout();
    dbinterface_widget_ = ATSDB::instance().interface().widget();
    QObject::connect(dbinterface_widget_, SIGNAL(databaseOpenedSignal()), this, SLOT(databaseOpenedSlot()));
    widget_layout->addWidget(dbinterface_widget_, 1);

    dbschema_manager_widget_ = ATSDB::instance().schemaManager().widget();
    QObject::connect(dbinterface_widget_, SIGNAL(databaseOpenedSignal()), dbschema_manager_widget_, SLOT(databaseOpenedSlot()));
    widget_layout->addWidget(dbschema_manager_widget_, 1);

    object_manager_widget_ = ATSDB::instance().objectManager().widget();
    QObject::connect(dbinterface_widget_, SIGNAL(databaseOpenedSignal()), object_manager_widget_, SLOT(databaseOpenedSlot()));
    widget_layout->addWidget(object_manager_widget_, 1);

    main_layout->addLayout(widget_layout);

    QHBoxLayout *start_layout = new QHBoxLayout ();
    start_layout->addStretch();

    start_button_ = new QPushButton ("Start");
    start_button_->setDisabled(true);
    QObject::connect(start_button_, SIGNAL(clicked()), this, SLOT(startSlot()));
    start_layout->addWidget(start_button_);

    main_layout->addLayout(start_layout, 1);

    main_widget->setLayout(main_layout);

    main_widget->setAutoFillBackground(true);
    tab_widget_->addTab(main_widget, "DB Config");

    // management widget
    management_widget_ = new ManagementWidget ();
    management_widget_->setAutoFillBackground(true);

    setCentralWidget(tab_widget_);

    tab_widget_->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    logdbg  << "MainWindow: destructor";

    // remember: this not called! insert deletes into closeEvent function
}

void MainWindow::databaseOpenedSlot()
{
    logdbg  << "MainWindow: databaseOpenedSlot";

    assert (start_button_);
    start_button_->setDisabled (false);

//    main_widget_ = new MFImport::MainWidget ();
//    assert (main_widget_);
//    widget_stack_->addWidget (main_widget_);
}

void MainWindow::startSlot ()
{
    logdbg  << "MainWindow: startSlot";

    assert (management_widget_);
    tab_widget_->addTab (management_widget_, "Management");

    ATSDB::instance().viewManager().init(tab_widget_);
    start_button_->setDisabled (true);

    // TODO lock stuff
    tab_widget_->setCurrentIndex(1);

}

void MainWindow::createMenus()
{
    logdbg  << "MainWindow: createMenus";

    QAction *exit_action = new QAction(tr("E&xit"), this);
    exit_action->setShortcuts(QKeySequence::Quit);
    exit_action->setStatusTip(tr("Exit the application"));
    connect(exit_action, SIGNAL(triggered()), this, SLOT(close()));

    QMenu *file_menu = menuBar()->addMenu(tr("&File"));
    file_menu->addAction(exit_action);

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    logdbg  << "MainWindow: closeEvent: start";

    QSettings settings("ATSDB", "Client");
    settings.setValue("MainWindow/geometry", saveGeometry());

    ConfigurationManager::getInstance().saveConfiguration();

    ATSDB::instance().shutdown();
    assert (!ATSDB::instance().ready());

    if (tab_widget_)
    {
        delete tab_widget_;
        tab_widget_=nullptr;
    }

    QWidget::closeEvent(event);
    logdbg  << "MainWindow: closeEvent: done";
}

void MainWindow::keyPressEvent ( QKeyEvent * event )
{
    logdbg  << "MainWindow: keyPressEvent '" << event->text().toStdString() << "'";

//    if (event->modifiers()  & Qt::ControlModifier)
//    {
//        if (event->key() == Qt::Key_U)
//        {
//            unlockSchemaGui();
//        }
//    }
}

//}
