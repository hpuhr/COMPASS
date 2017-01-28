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
//#include "DBObjectWidget.h"
#include "atsdb.h"
#include "dbinterfacewidget.h"
#include "dbschemamanagerwidget.h"
//#include "DBSchema.h"
//#include "DBSchemaManager.h"
//#include "DBSchemaWidget.h"
//#include "DBConnectionInfo.h"
#include "stringconv.h"
//#include "ProjectionManager.h"
//#include "ProjectionManagerWidget.h"
//#include "MainWidget.h"

using namespace Utils;
using namespace std;

//namespace ATSDB
//{

MainWindow::MainWindow()
    : dbinterface_widget_(nullptr), dbschema_manager_widget_(nullptr), start_button_(nullptr), db_opened_(false) //, object_widget_ (0)
{
    logdbg  << "MainWindow: constructor";

    QSettings settings("ATSDB", "Client");
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());

    createMenus();

    widget_stack_ = new QStackedWidget ();

    QWidget *main_widget = new QWidget ();
    QVBoxLayout *main_layout = new QVBoxLayout ();

    // for se widgets
    QHBoxLayout *layout = new QHBoxLayout ();
    dbinterface_widget_ = ATSDB::getInstance().dbInterfaceWidget();
    QObject::connect(dbinterface_widget_, SIGNAL(databaseOpenedSignal()), this, SLOT(databaseOpenedSlot()));
    layout->addWidget(dbinterface_widget_);

    dbschema_manager_widget_ = ATSDB::getInstance().dbSchemaManagerWidget();
    QObject::connect(dbinterface_widget_, SIGNAL(databaseOpenedSignal()), dbschema_manager_widget_, SLOT(databaseOpenedSlot()));
    layout->addWidget(dbschema_manager_widget_);
    main_layout->addLayout(layout);

    //object_widget_ = new DBObjectWidget ();

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
    assert (!db_opened_);

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
    assert (!ATSDB::getInstance().getDBOpened ());
    db_opened_=false;

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
