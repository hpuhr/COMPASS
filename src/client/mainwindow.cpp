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
#include <QCheckBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QGridLayout>
#include <QSettings>
#include <QThread>
#include <QTabWidget>

#include "mainwindow.h"
#include "global.h"
#include "logger.h"
#include "config.h"
#include "configurationmanager.h"
#include "atsdb.h"
#include "filtermanager.h"
#include "managementwidget.h"
#include "stringconv.h"
#include "jobmanager.h"
#include "viewmanager.h"
#include "taskmanager.h"
#include "taskmanagerwidget.h"
#include "jsonimportertask.h"
#include "jsonimportertaskwidget.h"
#include "radarplotpositioncalculatortask.h"
#include "radarplotpositioncalculatortaskwidget.h"
#include "createartasassociationstask.h"
#include "createartasassociationstaskwidget.h"
#include "files.h"
#include "config.h"

#if USE_JASTERIX
#include "asteriximporttask.h"
#include "asteriximporttaskwidget.h"
#endif

using namespace Utils;
using namespace std;

//namespace ATSDB
//{

MainWindow::MainWindow()
{
    logdbg  << "MainWindow: constructor";

    setMinimumSize(QSize(1200, 900));

    QIcon atsdb_icon(Files::getIconFilepath("atsdb.png").c_str());
    setWindowIcon(atsdb_icon); // for the glory of the empire

    QSettings settings("ATSDB", "Client");
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());

    SimpleConfig config ("config.json"); // TODO move to ATSDB
    assert (config.existsId("version"));
    std::string title =  "ATSDB v"+config.getString("version");

    QWidget::setWindowTitle (title.c_str());

    tab_widget_ = new QTabWidget ();
    tab_widget_->setAutoFillBackground(true);

    task_manager_widget_ = ATSDB::instance().taskManager().widget();
    tab_widget_->addTab(task_manager_widget_, "Tasks");

    connect (&ATSDB::instance().taskManager(), &TaskManager::startInspectionSignal,
             this, &MainWindow::startSlot);

    setAutoFillBackground(true);

    // management widget
    management_widget_ = new ManagementWidget ();
    management_widget_->setAutoFillBackground(true);

    setCentralWidget(tab_widget_);

    tab_widget_->setCurrentIndex(0);

   QObject::connect (this, &MainWindow::startedSignal,
                     &ATSDB::instance().filterManager(), &FilterManager::startedSlot);
}

MainWindow::~MainWindow()
{
    logdbg  << "MainWindow: destructor";

    // remember: this not called! insert deletes into closeEvent function
}

void MainWindow::databaseOpenedSlot()
{
    logdbg  << "MainWindow: databaseOpenedSlot";
}

void MainWindow::startSlot ()
{
    loginf  << "MainWindow: startSlot";

    emit startedSignal ();

    assert (task_manager_widget_);
    tab_widget_->removeTab(0);
    //ATSDB::instance().taskManager().deleteWidgets();

    assert (management_widget_);
    tab_widget_->addTab (management_widget_, "Management");

    ATSDB::instance().viewManager().init(tab_widget_);

    tab_widget_->setCurrentIndex(0);
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

    event->accept();

    //QWidget::closeEvent(event);
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

