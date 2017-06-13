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

//#include "Buffer.h"
#include "mainwindow.h"
#include "global.h"
#include "logger.h"
#include "config.h"
#include "workerthreadmanager.h"
#include "configurationmanager.h"
#include "dbobjectmanager.h"
#include "dbobjectmanagerwidget.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dbinterfacewidget.h"
#include "dbschemamanager.h"
#include "dbschemamanagerwidget.h"
#include "stringconv.h"
//#include "ProjectionManager.h"
//#include "ProjectionManagerWidget.h"

using namespace Utils;
using namespace std;

//namespace ATSDB
//{

MainWindow::MainWindow()
    : dbinterface_widget_(nullptr), dbschema_manager_widget_(nullptr),  object_manager_widget_ (nullptr), start_button_(nullptr)
{
    logdbg  << "MainWindow: constructor";

    QSettings settings("ATSDB", "Client");
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());

    createMenus();

    widget_stack_ = new QStackedWidget ();

    QWidget *main_widget = new QWidget ();
    QVBoxLayout *main_layout = new QVBoxLayout ();

    // for se widgets
    QHBoxLayout *widget_layout = new QHBoxLayout();
    dbinterface_widget_ = ATSDB::getInstance().dbInterface().widget();
    QObject::connect(dbinterface_widget_, SIGNAL(databaseOpenedSignal()), this, SLOT(databaseOpenedSlot()));
    widget_layout->addWidget(dbinterface_widget_);


    dbschema_manager_widget_ = ATSDB::getInstance().schemaManager().widget();
    QObject::connect(dbinterface_widget_, SIGNAL(databaseOpenedSignal()), dbschema_manager_widget_, SLOT(databaseOpenedSlot()));
    widget_layout->addWidget(dbschema_manager_widget_);

    object_manager_widget_ = ATSDB::getInstance().dbObjectManager().widget();
    QObject::connect(dbinterface_widget_, SIGNAL(databaseOpenedSignal()), object_manager_widget_, SLOT(databaseOpenedSlot()));
    widget_layout->addWidget(object_manager_widget_);

    main_layout->addLayout(widget_layout);

    QHBoxLayout *start_layout = new QHBoxLayout ();
    start_layout->addStretch();

    start_button_ = new QPushButton ("Start");
    start_layout->addWidget(start_button_);

    main_layout->addLayout(start_layout);

    main_widget->setLayout(main_layout);

    widget_stack_->addWidget (main_widget);
    setCentralWidget(widget_stack_);

    widget_stack_->setCurrentIndex (0);

    menuBar()->setNativeMenuBar(native_menu_);

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
//    if (db_opened_)
//    {
//        if (schema_widget_->hasSelectedSchema ())
//        {
//            assert (DBSchemaManager::getInstance().hasCurrentSchema());

//            widget_stack_->setCurrentIndex (1);
//            repaint();
//            start_button_->setDisabled (true);
//        }
//    }
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

    ATSDB::getInstance().shutdown();
    assert (!ATSDB::getInstance().ready());

    if (widget_stack_)
        delete widget_stack_;

    //WorkerThreadManager::getInstance().shutdown();

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
