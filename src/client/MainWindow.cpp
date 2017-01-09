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

//#include "Buffer.h"
#include "MainWindow.h"
#include "Global.h"
#include "Logger.h"
#include "Config.h"
#include "WorkerThreadManager.h"
#include "ConfigurationManager.h"
//#include "DBObjectWidget.h"
#include "ATSDB.h"
//#include "DBSelectionWidget.h"
//#include "DBSchema.h"
//#include "DBSchemaManager.h"
//#include "DBSchemaWidget.h"
//#include "DBConnectionInfo.h"
#include "String.h"
//#include "ProjectionManager.h"
//#include "ProjectionManagerWidget.h"
//#include "MainWidget.h"

using namespace Utils;
using namespace std;

//namespace ATSDB
//{

MainWindow::MainWindow()
//: Configurable ("MainWindow", "MainWindow0"), main_widget_(0), selection_widget_(0), schema_widget_(0),
    : start_button_(0), db_opened_(false) //, object_widget_ (0)
{
    logdbg  << "MainWindow: constructor";

//    registerParameter ("pos_x", &pos_x_, 0);
//    registerParameter ("pos_y", &pos_y_, 0);
//    registerParameter ("width", &width_, 1000);
//    registerParameter ("height", &height_, 700);
//    registerParameter ("min_width", &min_width_, 1000);
//    registerParameter ("min_height", &min_height_, 700);
//    registerParameter ("native_menu", &native_menu_, false);

//    setMinimumSize(QSize(min_width_, min_height_));
//    setGeometry(pos_x_, pos_y_, width_, height_);

    createMenus();

    widget_stack_ = new QStackedWidget ();

    //schema_widget_ = new DBSchemaWidget ();
    //connect(this, SIGNAL(openedDatabase()), schema_widget_, SLOT(openedDatabase()));

    //createSubConfigurables();

    //object_widget_ = new DBObjectWidget ();

//    assert (selection_widget_);
//    assert (schema_widget_);

//    db_config_widget_ = new QWidget ();
//    assert (db_config_widget_);
//    createDBConfigWidget ();

//    widget_stack_->addWidget (db_config_widget_);
    setCentralWidget(widget_stack_);

//    widget_stack_->setCurrentIndex (0);

    menuBar()->setNativeMenuBar(native_menu_);

}

MainWindow::~MainWindow()
{
    logdbg  << "MainWindow: destructor";

    // remember: this not called! insert deletes into closeEvent function
}

void MainWindow::openedDB()
{
    assert (!db_opened_);

    db_opened_=true;

    assert (start_button_);
    start_button_->setDisabled (false);

//    main_widget_ = new MFImport::MainWidget ();
//    assert (main_widget_);
//    widget_stack_->addWidget (main_widget_);
}

void MainWindow::start ()
{
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

    ConfigurationManager::getInstance().saveConfiguration();

    if (widget_stack_)
        delete widget_stack_;

    if (db_opened_)
    {
        logdbg  << "MainWindow: closeEvent: database shutdown";

        if (ATSDB::getInstance().getDBOpened ())
            ATSDB::getInstance().shutdown();

        db_opened_=false;
    }
    assert (!ATSDB::getInstance().getDBOpened ());

    //WorkerThreadManager::getInstance().shutdown();

    logdbg  << "MainWindow: closeEvent: done";
}

void MainWindow::moveEvent (QMoveEvent *event)
{
    logdbg  << "MainWindow: moveEvent";
//    pos_x_ = event->pos().x();
//    pos_y_ = event->pos().y();
}

void MainWindow::resizeEvent (QResizeEvent *event)
{
    logdbg  << "MainWindow: resizeEvent";
//    width_ = event->size().width();
//    height_ = event->size().height();
}

//void MainWindow::generateSubConfigurable (std::string class_id, std::string instance_id)
//{
//    if (class_id.compare("DBSelectionWidget") == 0)
//    {
//        assert (selection_widget_ == 0);
//        selection_widget_ = new DBSelectionWidget ("DBSelectionWidget", instance_id, this);
//        connect (selection_widget_, SIGNAL(databaseOpened()), this, SLOT(openedDB()));
//    }
//    else
//        throw std::runtime_error ("MainWindow: generateSubConfigurable: unknown sub-configurable "+class_id);
//}

//void MainWindow::checkSubConfigurables ()
//{
//    if (selection_widget_ == 0)
//    {
//        generateSubConfigurable ("DBSelectionWidget", "DBSelectionWidget0");
//    }
//}

void MainWindow::createDBConfigWidget ()
{
//    QFont font_bold;
//    font_bold.setBold(true);

//    QFont font_big;
//    font_big.setPointSize(18);

//    assert (selection_widget_ != 0);

//    QHBoxLayout *layout = new QHBoxLayout ();

//    layout->addWidget (selection_widget_);

//    QVBoxLayout *db_schema_layout = new QVBoxLayout ();

//    assert (schema_widget_);
//    db_schema_layout->addWidget (schema_widget_);

//    db_schema_layout->addWidget (object_widget_);

//    ProjectionManagerWidget *projmanwi = new ProjectionManagerWidget ();
//    db_schema_layout->addWidget (projmanwi);

//    db_schema_layout->addStretch();

//    QHBoxLayout *start_layout = new QHBoxLayout ();

//    start_layout->addStretch();

//    start_button_ = new QPushButton(tr("Start"));
//    start_button_->setFont (font_bold);
//    start_button_->setMinimumWidth(200);
//    connect(start_button_, SIGNAL( clicked() ), this, SLOT( start() ));
//    start_button_->setDisabled (true);
//    start_layout->addWidget(start_button_);

//    db_schema_layout->addLayout(start_layout);

//    layout->addLayout (db_schema_layout);

//    db_config_widget_->setLayout (layout);
}

void MainWindow::keyPressEvent ( QKeyEvent * event )
{
    logdbg  << "MainWindow: keyPressEvent '" << event->text().toStdString() << "'";

    if (event->modifiers()  & Qt::ControlModifier)
    {
        if (event->key() == Qt::Key_U)
        {
            unlockSchemaGui();
        }
    }
}

void MainWindow::unlockSchemaGui()
{
    loginf  << "MainWindow: unlockDBGui";

//    if (schema_widget_)
//        schema_widget_->unlock();

//    if (object_widget_)
//        object_widget_->unlock();
}

//void MainWindow::setDBType (std::string value)
//{
//    assert (selection_widget_);
//    selection_widget_->setDBType(value);
//}
//
//void MainWindow::setDBServer (std::string value)
//{
//    assert (selection_widget_);
//    selection_widget_->setDBServer(value);
//}
//void MainWindow::setDBName (std::string value)
//{
//    assert (selection_widget_);
//    selection_widget_->setDBName(value);
//}
//void MainWindow::setDBPort (std::string value)
//{
//    assert (selection_widget_);
//    selection_widget_->setDBPort(value);
//
//}
//void MainWindow::setDBUser (std::string value)
//{
//    assert (selection_widget_);
//    selection_widget_->setDBUser(value);
//
//}
//void MainWindow::setDBPassword (std::string value)
//{
//    assert (selection_widget_);
//    selection_widget_->setDBPassword(value);
//
//}
//
//void MainWindow::setDBNoPassword ()
//{
//    assert (selection_widget_);
//    selection_widget_->setDBNoPassword();
//}
//void MainWindow::setDBSchema (std::string value)
//{
//    assert (schema_widget_);
//    schema_widget_->setSchema(value);
//}
//
//
//void MainWindow::triggerAutoStart ()
//{
//    loginf  << "MainWindow: triggerAutoStart";
//    selection_widget_->connectDB ();
//    selection_widget_->openDB ();
//
//    if (db_opened_)
//        start ();
//    else
//    {
//        logerr  << "MainWindow: triggerAutoStart: db open failed";
//    }
//}

//}
