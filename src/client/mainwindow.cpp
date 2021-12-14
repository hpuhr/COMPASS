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
#include "evaluationmanager.h"
#include "compass.h"

#if USE_JASTERIX
#include "asteriximporttask.h"
#include "asteriximporttaskdialog.h"
#endif

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
#include <QThread>

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

    open_existing_db_action_ = new QAction(tr("&Open"));
    open_existing_db_action_->setShortcuts(QKeySequence::Open);
    open_existing_db_action_->setStatusTip(tr("Open an existing database"));
    connect(open_existing_db_action_, &QAction::triggered, this, &MainWindow::openExistingDBSlot);
    file_menu->addAction(open_existing_db_action_);

    open_recent_db_menu_ = file_menu->addMenu("Open Recent");
    open_recent_db_menu_->setStatusTip(tr("Open a recent database"));

    open_recent_db_menu_->addSeparator();

    QAction* clear_act = new QAction("Clear");
    connect(clear_act, &QAction::triggered, this, &MainWindow::clearExistingDBsSlot);
    open_recent_db_menu_->addAction(clear_act);

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

#if USE_JASTERIX
    QAction* import_ast_file_action = new QAction(tr("&ASTERIX Recording"));
    import_ast_file_action->setShortcut(tr("Ctrl+A"));
    import_ast_file_action->setStatusTip(tr("Import ASTERIX Recording File"));
    connect(import_ast_file_action, &QAction::triggered, this, &MainWindow::importAsterixRecordingSlot);
    import_menu_->addAction(import_ast_file_action);

    import_recent_asterix_menu_ = file_menu->addMenu("Recent ASTERIX Recording");
    import_recent_asterix_menu_->setStatusTip(tr("Import a recent ASTERIX Recording File"));
    import_menu_->addMenu(import_recent_asterix_menu_);

    QAction* import_ast_net_action = new QAction(tr("ASTERIX From Network"));
    import_ast_net_action->setStatusTip(tr("Import ASTERIX From Network"));
    connect(import_ast_net_action, &QAction::triggered, this, &MainWindow::importAsterixFromNetworkSlot);
    import_menu_->addAction(import_ast_net_action);
#endif

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
    assert (open_existing_db_action_);
    assert (open_recent_db_menu_);
    assert (close_db_action_);

    assert (import_menu_);

    open_recent_db_menu_->clear();

    // recent db files
    vector<string> recent_file_list = COMPASS::instance().dbFileList();

    for (auto& fn_it : recent_file_list)
    {
        QAction* file_act = new QAction(fn_it.c_str());
        file_act->setData(fn_it.c_str());
        connect(file_act, &QAction::triggered, this, &MainWindow::openRecentDBSlot);
        open_recent_db_menu_->addAction(file_act);
    }
    open_recent_db_menu_->setDisabled(recent_file_list.size() == 0);

    bool db_open = COMPASS::instance().dbOpened();

    new_db_action_->setDisabled(db_open);
    open_existing_db_action_->setDisabled(db_open);
    open_recent_db_menu_->setDisabled(db_open);
    close_db_action_->setDisabled(!db_open);

    import_menu_->setDisabled(!db_open || COMPASS::instance().taskManager().asterixImporterTask().isRunning());

#if USE_JASTERIX
    assert (import_recent_asterix_menu_);

    import_recent_asterix_menu_->clear();

    vector<string> recent_ast_list =  COMPASS::instance().taskManager().asterixImporterTask().fileList();

    for (auto& fn_it : recent_ast_list)
    {
        QAction* file_act = new QAction(fn_it.c_str());
        file_act->setData(fn_it.c_str());
        connect(file_act, &QAction::triggered, this, &MainWindow::importRecentAsterixRecordingSlot);
        import_recent_asterix_menu_->addAction(file_act);
    }
    import_recent_asterix_menu_->setDisabled(recent_ast_list.size() == 0);

#endif
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


void MainWindow::createAndOpenNewSqlite3DB(const std::string& filename)
{
    loginf << "MainWindow: sqlite3CreateNewDB: filename '" << filename << "'";

    automatic_tasks_defined_ = true;
    sqlite3_create_new_db_ = true;
    sqlite3_create_new_db_filename_ = filename;
}

void MainWindow::openSqlite3DB(const std::string& filename)
{
    loginf << "MainWindow: sqlite3OpenDB: filename '" << filename << "'";

    automatic_tasks_defined_ = true;
    sqlite3_open_db_ = true;
    sqlite3_open_db_filename_ = filename;
}

#if USE_JASTERIX
void MainWindow::importASTERIXFile(const std::string& filename)
{
    loginf << "MainWindow: asterixImportFile: filename '" << filename << "'";

    automatic_tasks_defined_ = true;
    asterix_import_file_ = true;
    asterix_import_filename_ = filename;
}

void MainWindow::importASTERIXFromNetwork()
{
    loginf << "MainWindow: importASTERIXFromNetwork";

    automatic_tasks_defined_ = true;
    asterix_import_network_ = true;
}

void MainWindow::importASTERIXFromNetworkTimeOffset(float value)
{
    loginf << "MainWindow: importASTERIXFromNetworkTimeOffset: offset " << String::timeStringFromDouble(value);

    asterix_import_network_time_offset_ = value;
}

float MainWindow::importASTERIXFromNetworkTimeOffset()
{
    return asterix_import_network_time_offset_;
}

#endif

void MainWindow::loadData(bool value)
{
    loginf << "MainWindow: loadData: value " << value;

    automatic_tasks_defined_ = true;
    load_data_ = value;
}

void MainWindow::quit(bool value)
{
    loginf << "MainWindow: autoQuitAfterProcess: value " << value;

    automatic_tasks_defined_ = true;
    quit_ = value;
}

bool MainWindow::quitNeeded()
{
    return quit_;
}

bool MainWindow::automaticTasksDefined() const
{
    return automatic_tasks_defined_;
}

void MainWindow::performAutomaticTasks ()
{
    loginf << "MainWindow: performAutomaticTasks";
    assert (automatic_tasks_defined_);

    if (!(sqlite3_create_new_db_ || sqlite3_open_db_))
    {
        logerr << "MainWindow: performAutomaticTasks: neither create nor open sqlite3 is set";
        return;
    }

    if (sqlite3_create_new_db_ && sqlite3_open_db_)
    {
        logerr << "MainWindow: performAutomaticTasks: both create and open sqlite3 are set";
        return;
    }

    if (sqlite3_create_new_db_)
    {
        loginf << "MainWindow: performAutomaticTasks: creating and opening new sqlite3 database '"
               << sqlite3_create_new_db_filename_ << "'";

        if (Files::fileExists(sqlite3_create_new_db_filename_))
            Files::deleteFile(sqlite3_create_new_db_filename_);

        COMPASS::instance().openDBFile(sqlite3_create_new_db_filename_);

        updateMenus();
    }
    else if (sqlite3_open_db_)
    {
        loginf << "MainWindow: performAutomaticTasks: opening existing sqlite3 database '"
               << sqlite3_open_db_filename_ << "'";

        if (!Files::fileExists(sqlite3_open_db_filename_))
        {
            logerr << "MainWindow: performAutomaticTasks: sqlite3 database '" << sqlite3_open_db_filename_
                   << "' does not exist";
            return;
        }

        COMPASS::instance().openDBFile(sqlite3_open_db_filename_);

        updateMenus();
    }

    loginf << "MainWindow: performAutomaticTasks: database opened";

//    // do longer wait on startup for things to settle
//    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

//    while ((boost::posix_time::microsec_clock::local_time()-start_time).total_milliseconds() < 50)
//    {
//        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
//        QThread::msleep(1);
//    }
//    // does not show widget
//    //QCoreApplication::processEvents();

//    // does cause application halt
//    //    while (QCoreApplication::hasPendingEvents())
//    //        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

//    loginf << "MainWindow: performAutomaticTasks: waiting done";

//    if (view_points_import_file_)
//    {
//        loginf << "MainWindow: performAutomaticTasks: importing view points file '"
//               << view_points_import_filename_ << "'";

//        if (!Files::fileExists(view_points_import_filename_))
//        {
//            logerr << "MainWindow: performAutomaticTasks: view points file '" << view_points_import_filename_
//                   << "' does not exist";
//            return;
//        }

//        widget_->setCurrentTask(*view_points_import_task_);
//        if(widget_->getCurrentTaskName() != view_points_import_task_->name())
//        {
//            logerr << "MainWindow: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                   << "' selected, aborting";
//            return;
//        }

//        ViewPointsImportTaskWidget* view_points_import_task_widget =
//                dynamic_cast<ViewPointsImportTaskWidget*>(view_points_import_task_->widget());
//        assert(view_points_import_task_widget);

//        view_points_import_task_widget->addFile(view_points_import_filename_);
//        view_points_import_task_widget->selectFile(view_points_import_filename_);

//        assert(view_points_import_task_->canImport());
//        view_points_import_task_->showDoneSummary(false);

//        view_points_import_task_widget->importSlot();

//        while (!view_points_import_task_->finished())
//        {
//            QCoreApplication::processEvents();
//            QThread::msleep(1);
//        }
//    }

#if USE_JASTERIX
    assert (!(asterix_import_file_ && asterix_import_network_)); // check done in client

    if (asterix_import_file_)
    {
        loginf << "MainWindow: performAutomaticTasks: importing ASTERIX file '"
               << asterix_import_filename_ << "'";

        if (!Files::fileExists(asterix_import_filename_))
        {
            logerr << "MainWindow: performAutomaticTasks: ASTERIX file '" << asterix_import_filename_
                   << "' does not exist";
            return;
        }

        ASTERIXImportTask& ast_import_task = COMPASS::instance().taskManager().asterixImporterTask();

//        if (!ast_import_task.hasFile(asterix_import_filename_))
//            ast_import_task.addFile(asterix_import_filename_);

        ast_import_task.importFilename(asterix_import_filename_);

        assert(ast_import_task.canRun());
        ast_import_task.showDoneSummary(false);

        ast_import_task.run(false); // no test
    }

    if (asterix_import_network_)
    {
        loginf << "MainWindow: performAutomaticTasks: importing ASTERIX from network";

        ASTERIXImportTask& ast_import_task = COMPASS::instance().taskManager().asterixImporterTask();

        ast_import_task.importNetwork();

        assert(ast_import_task.canRun());
        ast_import_task.showDoneSummary(false);

        ast_import_task.run(false); // no test
    }
#endif

////    if (json_import_file_)
////    {
////        loginf << "MainWindow: performAutomaticTasks: importing JSON file '"
////               << json_import_filename_ << "'";

////#if USE_JASTERIX
////        if (!Files::fileExists(json_import_filename_))
////        {
////            logerr << "MainWindow: performAutomaticTasks: JSON file '" << asterix_import_filename_
////                   << "' does not exist";
////            return;
////        }
////#endif

////        if(!json_import_task_->hasSchema(json_import_schema_))
////        {
////            logerr << "MainWindow: performAutomaticTasks: JSON schema '" << json_import_schema_
////                   << "' does not exist";
////            return;
////        }

////        widget_->setCurrentTask(*json_import_task_);
////        if(widget_->getCurrentTaskName() != json_import_task_->name())
////        {
////            logerr << "MainWindow: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
////                   << "' selected, aborting";
////            return;
////        }

////        JSONImportTaskWidget* json_import_task_widget =
////                dynamic_cast<JSONImportTaskWidget*>(json_import_task_->widget());
////        assert(json_import_task_widget);

////        json_import_task_widget->addFile(json_import_filename_);
////        json_import_task_widget->selectFile(json_import_filename_);
////        json_import_task_widget->selectSchema(json_import_schema_);

////        assert(json_import_task_->canRun());
////        json_import_task_->showDoneSummary(false);

////        widget_->runTask(*json_import_task_);

////        while (!json_import_task_->done())
////        {
////            QCoreApplication::processEvents();
////            QThread::msleep(1);
////        }

////        loginf << "MainWindow: performAutomaticTasks: importing JSON file done";
////    }

//    if (gps_trail_import_file_)
//    {
//        loginf << "MainWindow: performAutomaticTasks: importing GPS trail file '"
//               << gps_trail_import_filename_ << "'";

//        if (!Files::fileExists(gps_trail_import_filename_))
//        {
//            logerr << "MainWindow: performAutomaticTasks: GPS trail file '" << gps_trail_import_filename_
//                   << "' does not exist";
//            return;
//        }

//        widget_->setCurrentTask(*gps_trail_import_task_);
//        if(widget_->getCurrentTaskName() != gps_trail_import_task_->name())
//        {
//            logerr << "MainWindow: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                   << "' selected, aborting";
//            return;
//        }

//        GPSTrailImportTaskWidget* gps_import_task_widget =
//                dynamic_cast<GPSTrailImportTaskWidget*>(gps_trail_import_task_->widget());
//        assert(gps_import_task_widget);

//        gps_import_task_widget->addFile(gps_trail_import_filename_);
//        gps_import_task_widget->selectFile(gps_trail_import_filename_);

//        assert(gps_trail_import_task_->canRun());
//        gps_trail_import_task_->showDoneSummary(false);

//        widget_->runTask(*gps_trail_import_task_);

//        while (!gps_trail_import_task_->done())
//        {
//            QCoreApplication::processEvents();
//            QThread::msleep(1);
//        }
//    }

//    if (sectors_import_file_)
//    {
//        loginf << "MainWindow: performAutomaticTasks: importing sectors file '"
//               << sectors_import_filename_ << "'";

//        if (!Files::fileExists(sectors_import_filename_))
//        {
//            logerr << "MainWindow: performAutomaticTasks: sectors file file '" << sectors_import_filename_
//                   << "' does not exist";
//            return;
//        }

//        widget_->setCurrentTask(*manage_sectors_task_);
//        if(widget_->getCurrentTaskName() != manage_sectors_task_->name())
//        {
//            logerr << "MainWindow: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                   << "' selected, aborting";
//            return;
//        }

//        ManageSectorsTaskWidget* manage_sectors_task_widget =
//                dynamic_cast<ManageSectorsTaskWidget*>(manage_sectors_task_->widget());
//        assert(manage_sectors_task_widget);

//        manage_sectors_task_->showDoneSummary(false);
//        manage_sectors_task_widget->importSectorsJSON(sectors_import_filename_);

//        //widget_->runTask(*manage_sectors_task_);

//        //        while (!manage_sectors_task_->done())
//        //        {
//        //            QCoreApplication::processEvents();
//        //            QThread::msleep(1);
//        //        }
//    }

//    start_time = boost::posix_time::microsec_clock::local_time();
//    while ((boost::posix_time::microsec_clock::local_time()-start_time).total_milliseconds() < 50)
//    {
//        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
//        QThread::msleep(1);
//    }

//    if (auto_process_)
//    {
//        // calculate radar plot positions
//        if (radar_plot_position_calculator_task_->isRecommended())
//        {
//            loginf << "MainWindow: performAutomaticTasks: starting radar plot position calculation task";

//            widget_->setCurrentTask(*radar_plot_position_calculator_task_);
//            if(widget_->getCurrentTaskName() != radar_plot_position_calculator_task_->name())
//            {
//                logerr << "MainWindow: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                       << "' selected, aborting";
//                return;
//            }
//            radar_plot_position_calculator_task_->showDoneSummary(false);

//            widget_->runTask(*radar_plot_position_calculator_task_);

//            while (!radar_plot_position_calculator_task_->done())
//            {
//                QCoreApplication::processEvents();
//                QThread::msleep(1);
//            }
//        }

//        // post-process
////        loginf << "MainWindow: performAutomaticTasks: starting post-processing task";

////        if (!post_process_task_->isRecommended())
////        {

////            logerr << "MainWindow: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
////                   << "' selected, aborting";
////            return;
////        }

////        assert(post_process_task_->isRecommended());
////        assert(post_process_task_->isRequired());

////        widget_->setCurrentTask(*post_process_task_);
////        if(widget_->getCurrentTaskName() != post_process_task_->name())
////            widget_->setCurrentTask(*post_process_task_);

////        widget_->runTask(*post_process_task_);

////        while (!post_process_task_->done())
////        {
////            QCoreApplication::processEvents();
////            QThread::msleep(1);
////        }

////        loginf << "MainWindow: performAutomaticTasks: post-processing task done";

//        // artas assocs
//        if (create_artas_associations_task_->isRecommended())
//        {
//            loginf << "MainWindow: performAutomaticTasks: starting association task";

//            widget_->setCurrentTask(*create_artas_associations_task_);
//            if(widget_->getCurrentTaskName() != create_artas_associations_task_->name())
//            {
//                logerr << "MainWindow: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                       << "' selected, aborting";
//                return;
//            }

//            create_artas_associations_task_->showDoneSummary(false);

//            widget_->runTask(*create_artas_associations_task_);

//            while (!create_artas_associations_task_->done())
//            {
//                QCoreApplication::processEvents();
//                QThread::msleep(1);
//            }
//        }
//    }

//    if (associate_data_)
//    {
//        if (create_associations_task_->canRun())
//        {
//            widget_->setCurrentTask(*create_associations_task_);
//            if(widget_->getCurrentTaskName() != create_associations_task_->name())
//            {
//                logerr << "MainWindow: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
//                       << "' selected, aborting";
//                return;
//            }

//            create_associations_task_->showDoneSummary(false);

//            widget_->runTask(*create_associations_task_);

//            while (!create_associations_task_->done())
//            {
//                QCoreApplication::processEvents();
//                QThread::msleep(1);
//            }
//        }
//        else
//            logerr << "MainWindow: performAutomaticTasks: associate data task can not be run";
//    }

//    loginf << "MainWindow: performAutomaticTasks: done with startup tasks";

//    bool started = false;

//    if (start_)
//    {
//        loginf << "MainWindow: performAutomaticTasks: starting";

//        if(widget_->isStartPossible())
//        {
//            widget_->startSlot();
//            QCoreApplication::processEvents();

//            started = true;
//        }
//        else
//            loginf << "MainWindow: performAutomaticTasks: start not possible";
//    }

    if (load_data_)
    {
        loginf << "MainWindow: performAutomaticTasks: loading data";

        DBObjectManager& obj_man = COMPASS::instance().objectManager();

        obj_man.startLoading();

        while (obj_man.loadInProgress())
        {
            QCoreApplication::processEvents();
            QThread::msleep(1);
        }
    }
    else
        loginf << "MainWindow: performAutomaticTasks: not loading data";

//    if (export_view_points_report_)
//    {
//        if (!started)
//        {
//            logerr << "MainWindow: performAutomaticTasks: exporting view points report not possible since not started";
//        }
//        else
//        {
//            loginf << "MainWindow: performAutomaticTasks: exporting view points report";

//            getMainWindow()->showViewPointsTab();

//            ViewPointsReportGenerator& gen = COMPASS::instance().viewManager().viewPointsGenerator();

//            ViewPointsReportGeneratorDialog& dialog = gen.dialog();
//            dialog.show();

//            QCoreApplication::processEvents();

//            gen.reportPathAndFilename(export_view_points_report_filename_);
//            gen.showDone(false);

//            gen.run();

//            while (gen.isRunning()) // not sure if needed here but what the hell
//            {
//                QCoreApplication::processEvents();
//                QThread::msleep(1);
//            }

//            gen.showDone(true);
//        }
//    }

//    if (evaluate_ || export_eval_report_)
//    {
//        if (!started)
//        {
//            logerr << "MainWindow: performAutomaticTasks: evaluation not possible since not started";
//        }
//        else
//        {
//            loginf << "MainWindow: performAutomaticTasks: running evaluation";

//            getMainWindow()->showEvaluationTab();

//            EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

//            if (eval_man.canLoadData())
//            {
//                loginf << "MainWindow: performAutomaticTasks: loading evaluation data";

//                eval_man.loadData();

//                while (!eval_man.dataLoaded())
//                {
//                    QCoreApplication::processEvents();
//                    QThread::msleep(1);
//                }

//                assert (eval_man.dataLoaded());

//                if (eval_man.canEvaluate())
//                {
//                    loginf << "MainWindow: performAutomaticTasks: doing evaluation";

//                    eval_man.evaluate();

////                    while (!eval_man.evaluated())
////                    {
////                        QCoreApplication::processEvents();
////                        QThread::msleep(1);
////                    }

//                    assert (eval_man.evaluated());

//                    loginf << "MainWindow: performAutomaticTasks: evaluation done";

//                    if (export_eval_report_)
//                    {
//                        if (eval_man.canGenerateReport())
//                        {
//                            loginf << "MainWindow: performAutomaticTasks: generating report";

//                            EvaluationResultsReport::PDFGenerator& gen = eval_man.pdfGenerator();

//                            EvaluationResultsReport::PDFGeneratorDialog& dialog = gen.dialog();
//                            dialog.show();

//                            QCoreApplication::processEvents();

//                            gen.reportPathAndFilename(export_eval_report_filename_);
//                            gen.showDone(false);

//                            gen.run();

//                            while (gen.isRunning()) // not sure if needed here but what the hell
//                            {
//                                QCoreApplication::processEvents();
//                                QThread::msleep(1);
//                            }

//                            gen.showDone(true);

//                            loginf << "MainWindow: performAutomaticTasks: generating evaluation report done";
//                        }
//                        else
//                            logerr << "MainWindow: performAutomaticTasks: "
//                                      "exporting evaluation report not possible since report can't be generated";
//                    }
//                }
//                else
//                    logerr << "MainWindow: performAutomaticTasks: "
//                              "evaluation not possible since evaluation can not be made";
//            }
//            else
//                logerr << "MainWindow: performAutomaticTasks: "
//                          "evaluation not possible since no data can be loaded";
//        }
//    }

    if (quit_)
    {
        loginf << "MainWindow: performAutomaticTasks: quit requested";

        quitSlot();
    }
    else
        loginf << "MainWindow: performAutomaticTasks: not quitting";
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

    string filename = QFileDialog::getOpenFileName(this, "Import ASTERIX File").toStdString();

    if (filename.size() > 0)
    {
        COMPASS::instance().taskManager().asterixImporterTask().importFilename(filename); // also adds

        updateMenus();

        COMPASS::instance().taskManager().asterixImporterTask().dialog()->show();
    }
}

void MainWindow::importRecentAsterixRecordingSlot()
{
    loginf << "MainWindow: importRecentAsterixRecordingSlot";

    QAction* action = dynamic_cast<QAction*> (QObject::sender());
    assert (action);

    string filename = action->data().toString().toStdString();

    assert (filename.size());

    COMPASS::instance().taskManager().asterixImporterTask().importFilename(filename); // also adds

    updateMenus();

    COMPASS::instance().taskManager().asterixImporterTask().dialog()->show();
}

void MainWindow::importAsterixFromNetworkSlot()
{
    loginf << "MainWindow: importAsterixFromNetworkSlot";

    COMPASS::instance().taskManager().asterixImporterTask().importNetwork();

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
