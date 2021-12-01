/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"

#include "compass.h"
#include "config.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbobjectmanagerloadwidget.h"
#include "metadbovariableconfigurationdialog.h"
#include "files.h"
#include "filtermanager.h"
#include "filtermanagerwidget.h"
#include "global.h"
//#include "jobmanager.h"
#include "logger.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "taskmanagerwidget.h"
#include "viewmanager.h"
#include "viewpointswidget.h"
#include "asteriximporttask.h"
#include "asteriximportrecordingtaskdialog.h"
#include "evaluationmanager.h"
#include "compass.h"

#include <QApplication>
#include <QFileDialog>
#include <QCloseEvent>
#include <QSettings>
#include <QStackedWidget>
#include <QTabWidget>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QMenu>
#include <QMenuBar>

using namespace Utils;
using namespace std;

MainWindow::MainWindow()
{
    logdbg << "MainWindow: constructor";

    QLocale::setDefault(QLocale::c());
    setLocale(QLocale::c());

    const char* appdir = getenv("APPDIR");
    if (appdir)
        QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs); // disable native since they cause crashes

    setMinimumSize(QSize(1200, 900));

    QIcon ats_icon(Files::getIconFilepath("ats.png").c_str());
    setWindowIcon(ats_icon);  // for the glory of the empire

    QSettings settings("COMPASS", "Client");
    restoreGeometry(settings.value("MainWindow/geometry").toByteArray());

    assert(COMPASS::instance().config().existsId("version"));
    std::string title = "OpenATS COMPASS v" + COMPASS::instance().config().getString("version");

    if (COMPASS::instance().config().existsId("save_config_on_exit"))
    {
        save_configuration_ = COMPASS::instance().config().getBool("save_config_on_exit");
        loginf << "MainWindow: constructor: save configuration on exit " << save_configuration_;
    }

    QWidget::setWindowTitle(title.c_str());

    tab_widget_ = new QTabWidget();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    tab_widget_->addTab(COMPASS::instance().objectManager().loadWidget(), "Data Sources");
    tab_widget_->addTab(COMPASS::instance().filterManager().widget(), "Filters");

    COMPASS::instance().evaluationManager().init(tab_widget_); // adds eval widget
    COMPASS::instance().viewManager().init(tab_widget_); // adds view points widget and view container

    tab_widget_->setCurrentIndex(0);

    QApplication::restoreOverrideCursor();

    setCentralWidget(tab_widget_);

    tab_widget_->setCurrentIndex(0);

    add_view_button_ = new QPushButton();
    add_view_button_->setIcon(QIcon(Files::getIconFilepath("crosshair_fat.png").c_str()));
    add_view_button_->setFixedSize(UI_ICON_SIZE);
    add_view_button_->setFlat(UI_ICON_BUTTON_FLAT);
    add_view_button_->setToolTip(tr("Add view"));
    connect(add_view_button_, &QPushButton::clicked, this, &MainWindow::showAddViewMenuSlot);
    tab_widget_->setCornerWidget(add_view_button_);

    createMenus ();
    updateMenus ();
}

MainWindow::~MainWindow()
{
    logdbg << "MainWindow: destructor";

    // remember: this not called! insert deletes into closeEvent function
}

void MainWindow::createMenus ()
{
    // file menu
    QMenu* file_menu = menuBar()->addMenu(tr("&File"));

    // db operations
    new_db_action_ = new QAction(tr("&New"));
    new_db_action_->setShortcuts(QKeySequence::New);
    new_db_action_->setStatusTip(tr("Create a new database"));
    connect(new_db_action_, &QAction::triggered, this, &MainWindow::newDBSlot);
    file_menu->addAction(new_db_action_);

    open_existng_action_ = new QAction(tr("&Open"));
    open_existng_action_->setShortcuts(QKeySequence::Open);
    open_existng_action_->setStatusTip(tr("Open an existing database"));
    connect(open_existng_action_, &QAction::triggered, this, &MainWindow::openExistingDBSlot);
    file_menu->addAction(open_existng_action_);

    open_recent_menu_ = file_menu->addMenu("Open Recent");
    open_recent_menu_->setStatusTip(tr("Open a recent database"));

    open_recent_menu_->addSeparator();

    QAction* clear_act = new QAction("Clear");
    connect(clear_act, &QAction::triggered, this, &MainWindow::clearExistingDBsSlot);
    open_recent_menu_->addAction(clear_act);

    close_db_action_ = new QAction(tr("&Close"));
    close_db_action_->setShortcut(tr("Ctrl+C"));
    close_db_action_->setStatusTip(tr("Close opened database"));
    connect(close_db_action_, &QAction::triggered, this, &MainWindow::closeDBSlot);
    file_menu->addAction(close_db_action_);

    file_menu->addSeparator();

    // config operations

    QAction* save_act = new QAction("&Save Config");
    save_act->setShortcut(tr("Ctrl+S"));
    connect(save_act, &QAction::triggered, this, &MainWindow::saveConfigSlot);
    file_menu->addAction(save_act);

    file_menu->addSeparator();

    // quit operations

    QAction* quit2_act = new QAction(tr("Quit &Without Saving Config"));
    quit2_act->setShortcut(tr("Ctrl+W"));
    quit2_act->setStatusTip(tr("Quit the application withour saving the configuration"));
    connect(quit2_act, &QAction::triggered, this, &MainWindow::quitWOConfigSlot);
    file_menu->addAction(quit2_act);

    QAction* quit_act = new QAction(tr("&Quit"));
    quit_act->setShortcuts(QKeySequence::Quit);
    //QKeySequence(tr("Ctrl+P"));
    quit_act->setStatusTip(tr("Quit the application"));
    connect(quit_act, &QAction::triggered, this, &MainWindow::quitSlot);
    file_menu->addAction(quit_act);


    // import menu

    import_menu_ = menuBar()->addMenu(tr("&Import"));

    QAction* import_ast_file_action = new QAction(tr("&ASTERIX Recording"));
    import_ast_file_action->setShortcut(tr("Ctrl+A"));
    import_ast_file_action->setStatusTip(tr("Import ASTERIX Recording File"));
    connect(import_ast_file_action, &QAction::triggered, this, &MainWindow::importAsterixRecordingSlot);
    import_menu_->addAction(import_ast_file_action);

    // configuration
    QMenu* config_menu = menuBar()->addMenu(tr("&Configuration"));

    // db operations
    QAction* meta_action = new QAction(tr("Meta Variables"));
    meta_action->setStatusTip(tr("Configure Meta Variables"));
    connect(meta_action, &QAction::triggered, this, &MainWindow::configureMetaVariablesSlot);
    config_menu->addAction(meta_action);
}

void MainWindow::updateMenus()
{
    assert (new_db_action_);
    assert (open_existng_action_);
    assert (open_recent_menu_);
    assert (close_db_action_);

    assert (import_menu_);

    open_recent_menu_->clear();

    vector<string> recent_file_list = COMPASS::instance().dbFileList();

    for (auto& fn_it : recent_file_list)
    {
        QAction* file_act = new QAction(fn_it.c_str());
        file_act->setData(fn_it.c_str());
        connect(file_act, &QAction::triggered, this, &MainWindow::openRecentDBSlot);
        open_recent_menu_->addAction(file_act);
    }
    open_recent_menu_->setDisabled(recent_file_list.size() == 0);

    bool db_open = COMPASS::instance().dbOpened();

    new_db_action_->setDisabled(db_open);
    open_existng_action_->setDisabled(db_open);
    open_recent_menu_->setDisabled(db_open);
    close_db_action_->setDisabled(!db_open);

    import_menu_->setDisabled(!db_open);
}

void MainWindow::disableConfigurationSaving()
{
    logdbg << "MainWindow: disableConfigurationSaving";
    save_configuration_ = false;
}

void MainWindow::showEvaluationTab()
{
    assert (started_);
    assert (tab_widget_->count() > 1);
    tab_widget_->setCurrentIndex(1);
}

void MainWindow::showViewPointsTab()
{
    assert (started_);
    assert (tab_widget_->count() > 2);
    tab_widget_->setCurrentIndex(2);
}


void MainWindow::newDBSlot()
{
    loginf << "MainWindow: newDBSlot";

    string filename = QFileDialog::getSaveFileName(this, "New SQLite3 File").toStdString();

    if (filename.size() > 0)
    {
        COMPASS::instance().createNewDBFile(filename);

        updateMenus();
    }
}

void MainWindow::openExistingDBSlot()
{
    loginf << "MainWindow: openExistingDBSlot";

    string filename = QFileDialog::getOpenFileName(this, "Add SQLite3 File").toStdString();

    if (filename.size() > 0)
    {
        COMPASS::instance().openDBFile(filename);

        updateMenus();
    }
}

void MainWindow::openRecentDBSlot()
{
    loginf << "MainWindow: openRecentDBSlot";

    QAction* action = dynamic_cast<QAction*> (QObject::sender());
    assert (action);

    string filename = action->data().toString().toStdString();

    assert (filename.size());

    COMPASS::instance().openDBFile(filename);

    updateMenus();
}

void MainWindow::clearExistingDBsSlot()
{
    loginf << "MainWindow: clearExistingDBsSlot";

    COMPASS::instance().clearDBFileList();

    updateMenus();
}

void MainWindow::closeDBSlot()
{
    loginf << "MainWindow: closeDBSlot";

    COMPASS::instance().closeDB();

    updateMenus();
}

void MainWindow::saveConfigSlot()
{
    loginf << "MainWindow: saveConfigSlot";

    ConfigurationManager::getInstance().saveConfiguration();
}

void MainWindow::quitWOConfigSlot()
{
    loginf << "MainWindow: quitWOConfigSlot";

    save_configuration_ = false;

    shutdown();

    QApplication::quit();
}
void MainWindow::quitSlot()
{
    loginf << "MainWindow: quitSlot";

    shutdown();

    QApplication::quit();
}

void MainWindow::importAsterixRecordingSlot()
{
    loginf << "MainWindow: importAsterixRecordingSlot";

    COMPASS::instance().taskManager().asterixImporterTask().dialog()->show();
}

void MainWindow::configureMetaVariablesSlot()
{
    loginf << "MainWindow: configureMetaVariablesSlot";

    COMPASS::instance().objectManager().metaVariableConfigdialog()->show();
}

//void MainWindow::startSlot()
//{
//    loginf << "MainWindow: startSlot";
//    assert (!started_);

//    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

////    QMessageBox* msg_box = new QMessageBox(this);
////    msg_box->setWindowTitle("Starting");
////    msg_box->setText("Please wait...");
////    msg_box->setStandardButtons(0);
////    msg_box->show();

////    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
////    while ((boost::posix_time::microsec_clock::local_time()-start_time).total_milliseconds() < 50)
////    {
////        QCoreApplication::processEvents();
////        QThread::msleep(1);
////    }

//    emit startedSignal();

////    assert(task_manager_widget_);
////    tab_widget_->removeTab(0);

//    // close any opened dbobject widgets
////    for (auto& obj_it : COMPASS::instance().objectManager())
////        obj_it.second->closeWidget();

//    assert(management_widget_);
//    tab_widget_->addTab(management_widget_, "Load");

////    start_time = boost::posix_time::microsec_clock::local_time();
////    while ((boost::posix_time::microsec_clock::local_time()-start_time).total_milliseconds() < 50)
////    {
////        QCoreApplication::processEvents();
////        QThread::msleep(1);
////    }

//    COMPASS::instance().evaluationManager().init(tab_widget_); // adds eval widget
//    COMPASS::instance().viewManager().init(tab_widget_); // adds view points widget and view container

//    tab_widget_->setCurrentIndex(0);
//    add_view_button_->setDisabled(false);

//    emit JobManager::instance().databaseIdle();  // to enable ViewManager add button, slightly HACKY

////    msg_box->close();
////    delete msg_box;

//    QApplication::restoreOverrideCursor();

//    started_ = true;
//}

void MainWindow::quitRequestedSlot()
{
    shutdown();
    QApplication::quit();
}

void MainWindow::showAddViewMenuSlot()
{
    loginf << "MainWindow: showAddViewMenuSlot";
    COMPASS::instance().viewManager().showMainViewContainerAddView();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    loginf << "MainWindow: closeEvent";

    shutdown();
    event->accept();

    logdbg << "MainWindow: closeEvent: done";
}


void MainWindow::shutdown()
{
    QSettings settings("COMPASS", "Client");
    settings.setValue("MainWindow/geometry", saveGeometry());

    COMPASS::instance().viewManager().unsetCurrentViewPoint(); // needed to remove temporary stuff

    if (save_configuration_)
        ConfigurationManager::getInstance().saveConfiguration();
    else
        loginf << "MainWindow: closeEvent: configuration not saved";

    COMPASS::instance().shutdown();

    if (tab_widget_)
    {
        delete tab_widget_;
        tab_widget_ = nullptr;
    }
}

// void MainWindow::keyPressEvent ( QKeyEvent * event )
//{
//    logdbg << "MainWindow: keyPressEvent '" << event->text().toStdString() << "'";
//}
