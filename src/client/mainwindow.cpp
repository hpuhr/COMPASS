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
#include "mainwindow_commands.h"
#include "compass.h"
#include "config.h"
#include "configurationmanager.h"
#include "datasourcemanager.h"
#include "datasourcesconfigurationdialog.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "datasourcesloadwidget.h"
#include "dbcontent/variable/metavariableconfigurationdialog.h"
#include "files.h"
#include "filtermanager.h"
#include "filtermanagerwidget.h"
#include "global.h"
#include "logger.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "viewmanager.h"
#include "viewpointsimporttask.h"
#include "viewpointsimporttaskdialog.h"
#include "viewpointsreportgenerator.h"
#include "viewpointsreportgeneratordialog.h"
#include "gpstrailimporttask.h"
#include "gpstrailimporttaskdialog.h"
#include "managesectorstask.h"
#include "managesectorstaskdialog.h"
#include "evaluationmanager.h"
#include "compass.h"
#include "viewwidget.h"
#include "view.h"
#include "ui_test_common.h"

#include "asteriximporttask.h"
#include "asteriximporttaskdialog.h"
#include "jsonimporttask.h"
#include "jsonimporttaskdialog.h"

#include "radarplotpositioncalculatortask.h"
#include "radarplotpositioncalculatortaskdialog.h"
#include "createartasassociationstask.h"
#include "createartasassociationstaskdialog.h"
#include "createassociationstask.h"
#include "createassociationstaskdialog.h"

#include "test/ui_test_cmd.h"

#ifdef USE_EXPERIMENTAL_SOURCE
#include "geometrytreeitem.h"
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
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QTimer>

//#define SHOW_DEBUG_MENU

#ifdef SHOW_DEBUG_MENU
#include "test_lab.h"
#endif

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

    QWidget::setWindowTitle(title.c_str());

    QWidget* main_widget = new QWidget();

    QVBoxLayout* main_layout = new QVBoxLayout();
    main_layout->setContentsMargins(0, 0, 0, 0);

    main_widget->setLayout(main_layout);

    // initialize tabs

    tab_widget_ = new QTabWidget();
    tab_widget_->setObjectName("container0");

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    tab_widget_->addTab(COMPASS::instance().dataSourceManager().loadWidget(), "Data Sources");
    tab_widget_->addTab(COMPASS::instance().filterManager().widget(), "Filters");

    QTabBar *tabBar = tab_widget_->tabBar();

    tabBar->setTabButton(1, QTabBar::LeftSide, COMPASS::instance().filterManager().widget()->filtersCheckBox());
    //tabBar->setTabButton(0, QTabBar::RightSide, new QLabel("label0"));

    COMPASS::instance().evaluationManager().init(tab_widget_); // adds eval widget
    COMPASS::instance().viewManager().init(tab_widget_); // adds view points widget and view container

    tab_widget_->setCurrentIndex(0);

    QApplication::restoreOverrideCursor();

    //tab_widget_->setCurrentIndex(0);

    const QString tool_tip = "Add view";

    add_view_button_ = new QPushButton();
    UI_TEST_OBJ_NAME(add_view_button_, tool_tip);
    add_view_button_->setIcon(QIcon(Files::getIconFilepath("crosshair_fat.png").c_str()));
    add_view_button_->setFixedSize(UI_ICON_SIZE);
    add_view_button_->setFlat(UI_ICON_BUTTON_FLAT);
    add_view_button_->setToolTip(tr(tool_tip.toStdString().c_str()));
    add_view_button_->setDisabled(COMPASS::instance().disableAddRemoveViews());

    connect(add_view_button_, &QPushButton::clicked, this, &MainWindow::showAddViewMenuSlot);
    tab_widget_->setCornerWidget(add_view_button_);

    main_layout->addWidget(tab_widget_);

    // bottom widget

    QWidget* bottom_widget = new QWidget();
    bottom_widget->setMaximumHeight(40);

    QHBoxLayout* bottom_layout = new QHBoxLayout();
    bottom_layout->setContentsMargins(2, 2, 2, 2);

    db_label_ = new QLabel();
    bottom_layout->addWidget(db_label_);

    bottom_layout->addStretch();

    // add status & button

    status_label_ = new QLabel();
    bottom_layout->addWidget(status_label_);

    live_pause_resume_button_ = new QPushButton("Pause");
    live_pause_resume_button_->setObjectName("livebutton");
    connect(live_pause_resume_button_, &QPushButton::clicked, this, &MainWindow::livePauseResumeSlot);
    bottom_layout->addWidget(live_pause_resume_button_);

    if (!COMPASS::instance().disableLiveToOfflineSwitch())
    {
        live_stop_button_ = new QPushButton("Stop");
        connect(live_stop_button_, &QPushButton::clicked, this, &MainWindow::liveStopSlot);

        bottom_layout->addWidget(live_stop_button_);
    }

    bottom_layout->addStretch();

    // load button

    load_button_ = new QPushButton("Load");
    connect(load_button_, &QPushButton::clicked, this, &MainWindow::loadButtonSlot);
    bottom_layout->addWidget(load_button_);

    bottom_widget->setLayout(bottom_layout);

    updateBottomWidget();

    main_layout->addWidget(bottom_widget);

    setCentralWidget(main_widget);

    // do menus

    createMenus ();
    updateMenus ();

    // do signal slots
    connect (&COMPASS::instance(), &COMPASS::appModeSwitchSignal,
             this, &MainWindow::appModeSwitchSlot);

    QObject::connect(&COMPASS::instance().dbContentManager(), &DBContentManager::loadingDoneSignal,
                     this, &MainWindow::loadingDoneSlot);

    //init ui related commands
    ui_test::initUITestCommands();

    main_window::RTCommandOpenDB::init();
    main_window::RTCommandCreateDB::init();
    main_window::RTCommandImportViewPointsFile::init();
    main_window::RTCommandImportASTERIXFile::init();
    main_window::RTCommandImportASTERIXNetworkStart::init();
    main_window::RTCommandImportASTERIXNetworkStop::init();
    main_window::RTCommandImportJSONFile::init();
    main_window::RTCommandImportGPSTrail::init();
    main_window::RTCommandImportSectorsJSON::init();
    main_window::RTCommandCalculateRadarPlotPositions::init();
    main_window::RTCommandCalculateAssociations::init();
    main_window::RTCommandLoadData::init();
    main_window::RTCommandCloseDB::init();
    main_window::RTCommandQuit::init();
}

MainWindow::~MainWindow()
{
    logdbg << "MainWindow: destructor";

    // remember: this not called! insert deletes into closeEvent function
}

void MainWindow::createMenus ()
{
    bool expert_mode = COMPASS::instance().expertMode();

    menuBar()->setObjectName("mainmenu");

    // file menu
    QMenu* file_menu = menuBar()->addMenu(tr("&File"));
    file_menu->setObjectName("main_window_file_menu");
    file_menu->setToolTipsVisible(true);

    // db operations
    new_db_action_ = new QAction(tr("&New"));
    new_db_action_->setShortcuts(QKeySequence::New);
    new_db_action_->setToolTip(tr("Create a new database"));
    connect(new_db_action_, &QAction::triggered, this, &MainWindow::newDBSlot);
    file_menu->addAction(new_db_action_);

    open_existing_db_action_ = new QAction(tr("&Open"));
    open_existing_db_action_->setShortcuts(QKeySequence::Open);
    open_existing_db_action_->setToolTip(tr("Open an existing database"));
    connect(open_existing_db_action_, &QAction::triggered, this, &MainWindow::openExistingDBSlot);
    file_menu->addAction(open_existing_db_action_);

    open_recent_db_menu_ = file_menu->addMenu("Open Recent");
    open_recent_db_menu_->setToolTip(tr("Open a recent database"));

    open_recent_db_menu_->addSeparator();

    export_db_action_ = new QAction("&Export", file_menu);
    export_db_action_->setObjectName("main_window_exportdb_action");
    export_db_action_->setToolTip(tr("Export database into file"));
    connect(export_db_action_, &QAction::triggered, this, &MainWindow::exportDBSlot);
    file_menu->addAction(export_db_action_);

    QAction* clear_act = new QAction("Clear");
    connect(clear_act, &QAction::triggered, this, &MainWindow::clearExistingDBsSlot);
    open_recent_db_menu_->addAction(clear_act);

    close_db_action_ = new QAction(tr("&Close"));
    close_db_action_->setToolTip(tr("Close opened database"));
    connect(close_db_action_, &QAction::triggered, this, &MainWindow::closeDBSlot);
    file_menu->addAction(close_db_action_);

    file_menu->addSeparator();

    // config operations

    if (!COMPASS::instance().disableMenuConfigSave())
    {
        QAction* save_act = new QAction("&Save Config");
        save_act->setShortcut(tr("Ctrl+S"));
        connect(save_act, &QAction::triggered, this, &MainWindow::saveConfigSlot);
        file_menu->addAction(save_act);

        file_menu->addSeparator();
    }

    // quit operations
    quit_wo_cfg_sav_action_ = new QAction(tr("Quit &Without Saving Config"));
    quit_wo_cfg_sav_action_->setShortcut(tr("Ctrl+W"));
    quit_wo_cfg_sav_action_->setToolTip(tr("Quit the application withour saving the configuration"));
    connect(quit_wo_cfg_sav_action_, &QAction::triggered, this, &MainWindow::quitWOConfigSlot);
    file_menu->addAction(quit_wo_cfg_sav_action_);

    QAction* quit_act = new QAction(tr("&Quit"));
    quit_act->setShortcuts(QKeySequence::Quit);
    //QKeySequence(tr("Ctrl+P"));
    quit_act->setToolTip(tr("Quit the application"));
    connect(quit_act, &QAction::triggered, this, &MainWindow::quitSlot);
    file_menu->addAction(quit_act);

    // import menu

    import_menu_ = menuBar()->addMenu(tr("&Import"));
    import_menu_->setToolTipsVisible(true);

    QAction* import_ast_file_action = new QAction(tr("&ASTERIX Recording"));
    import_ast_file_action->setShortcut(tr("Ctrl+A"));
    import_ast_file_action->setToolTip(tr("Import ASTERIX Recording File"));
    connect(import_ast_file_action, &QAction::triggered, this, &MainWindow::importAsterixRecordingSlot);
    import_menu_->addAction(import_ast_file_action);

    import_recent_asterix_menu_ = import_menu_->addMenu("Recent ASTERIX Recording");
    import_recent_asterix_menu_->setToolTip(tr("Import a recent ASTERIX Recording File"));

    QAction* import_ast_net_action = new QAction(tr("ASTERIX From Network"));
    import_ast_net_action->setToolTip(tr("Import ASTERIX From Network"));
    connect(import_ast_net_action, &QAction::triggered, this, &MainWindow::importAsterixFromNetworkSlot);
    import_menu_->addAction(import_ast_net_action);

    QAction* import_json_file_action = new QAction(tr("&JSON Recording"));
    import_json_file_action->setShortcut(tr("Ctrl+J"));
    import_json_file_action->setToolTip(tr("Import JSON Recording File"));
    connect(import_json_file_action, &QAction::triggered, this, &MainWindow::importJSONRecordingSlot);
    import_menu_->addAction(import_json_file_action);

    QAction* import_gps_file_action = new QAction(tr("&GPS Trail"));
    import_gps_file_action->setShortcut(tr("Ctrl+G"));
    import_gps_file_action->setToolTip(tr("Import GPS Trail File"));
    connect(import_gps_file_action, &QAction::triggered, this, &MainWindow::importGPSTrailSlot);
    import_menu_->addAction(import_gps_file_action);

    if (!COMPASS::instance().hideViewpoints())
    {
        QAction* import_vp_file_action = new QAction(tr("&View Points"));
        import_vp_file_action->setShortcut(tr("Ctrl+V"));
        import_vp_file_action->setToolTip(tr("Import View Points File"));
        connect(import_vp_file_action, &QAction::triggered, this, &MainWindow::importViewPointsSlot);
        import_menu_->addAction(import_vp_file_action);
    }

    // configuration menu
    config_menu_ = menuBar()->addMenu(tr("&Configuration"));
    config_menu_->setToolTipsVisible(true);

    // configure operations
    QAction* ds_action = new QAction(tr("Data Sources"));
    ds_action->setToolTip(tr("Configure Data Sources"));
    connect(ds_action, &QAction::triggered, this, &MainWindow::configureDataSourcesSlot);
    config_menu_->addAction(ds_action);

    QAction* meta_action = new QAction(tr("Meta Variables"));

    if (expert_mode)
        meta_action->setToolTip(tr("Configure Meta Variables"));
    else
        meta_action->setToolTip(tr("Show Meta Variables"));

    connect(meta_action, &QAction::triggered, this, &MainWindow::configureMetaVariablesSlot);
    config_menu_->addAction(meta_action);

    sectors_action_ = new QAction(tr("Sectors"));
    sectors_action_->setToolTip(tr("Configure Sectors (stored in Database)"));
    connect(sectors_action_, &QAction::triggered, this, &MainWindow::configureSectorsSlot);
    sectors_action_->setDisabled(true);
    config_menu_->addAction(sectors_action_);

    // process menu
    process_menu_ = menuBar()->addMenu(tr("&Process"));
    process_menu_->setToolTipsVisible(true);

    QAction* calc_radar_plpos_action = new QAction(tr("Calculate Radar Plot Positions"));
    calc_radar_plpos_action->setToolTip(tr("Calculate Radar Plot Positios, only needed if Radar Position information"
                                           " was changed"));
    connect(calc_radar_plpos_action, &QAction::triggered, this, &MainWindow::calculateRadarPlotPositionsSlot);
    process_menu_->addAction(calc_radar_plpos_action);

    QAction* assoc_action = new QAction(tr("Calculate Associations"));
    assoc_action->setToolTip(tr("Create Unique Targets based on all DB Content"));
    connect(assoc_action, &QAction::triggered, this, &MainWindow::calculateAssociationsSlot);
    process_menu_->addAction(assoc_action);

    QAction* assoc_artas_action = new QAction(tr("Calculate Associations from ARTAS"));
    assoc_artas_action->setToolTip(tr("Create Unique Targets based on ARTAS TRI information"));
    connect(assoc_artas_action, &QAction::triggered, this, &MainWindow::calculateAssociationsARTASSlot);
    process_menu_->addAction(assoc_artas_action);

    // ui menu
    ui_menu_ = menuBar()->addMenu(tr("&UI"));
    ui_menu_->setToolTipsVisible(true);

    QAction* reset_views_action = new QAction(tr("Reset Views"));
    reset_views_action->setToolTip(
                "Enable all data sources, reset labels,\n"
                "disable all filters and reset Views to startup configuration");
    connect(reset_views_action, &QAction::triggered, this, &MainWindow::resetViewsMenuSlot);
    ui_menu_->addAction(reset_views_action);

#ifdef SHOW_DEBUG_MENU
    //debug menu (internal)
    createDebugMenu();
#endif
}

void MainWindow::updateMenus()
{
    assert (new_db_action_);
    assert (open_existing_db_action_);
    assert (open_recent_db_menu_);
    assert (close_db_action_);

    assert (sectors_action_);

    assert (import_menu_);

    bool in_live_running = COMPASS::instance().appMode() == AppMode::LiveRunning;
    bool in_live_paused = COMPASS::instance().appMode() == AppMode::LivePaused;

    bool in_live = in_live_running || in_live_paused;

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

    if (recent_file_list.size() == 0)
        open_recent_db_menu_->setDisabled(true);
    else // add clear action
    {
        open_recent_db_menu_->addSeparator();

        QAction* clear_file_act = new QAction("Clear");
        connect(clear_file_act, &QAction::triggered, this, &MainWindow::clearExistingDBsSlot);
        open_recent_db_menu_->addAction(clear_file_act);
    }

    bool db_open = COMPASS::instance().dbOpened();

    new_db_action_->setDisabled(db_open || in_live);
    open_existing_db_action_->setDisabled(db_open || in_live);

    if (recent_file_list.size()) // is disabled otherwise
        open_recent_db_menu_->setDisabled(db_open || in_live);

    export_db_action_->setDisabled(!db_open || in_live_running);
    close_db_action_->setDisabled(!db_open || in_live);

    sectors_action_->setDisabled(!db_open || in_live_running);

    import_menu_->setDisabled(!db_open || COMPASS::instance().taskManager().asterixImporterTask().isRunning()
                              || in_live);
    process_menu_->setDisabled(!db_open || COMPASS::instance().taskManager().asterixImporterTask().isRunning()
                               || in_live);

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
    if (recent_ast_list.size() == 0)
        import_recent_asterix_menu_->setDisabled(true);
    else
    {
        import_recent_asterix_menu_->addSeparator();

        QAction* clear_file_act = new QAction("Clear");
        connect(clear_file_act, &QAction::triggered, this, &MainWindow::clearImportRecentAsterixRecordingsSlot);
        import_recent_asterix_menu_->addAction(clear_file_act);
    }

    assert (config_menu_);
    config_menu_->setDisabled(!db_open || COMPASS::instance().taskManager().asterixImporterTask().isRunning()
                              || in_live);
}

void MainWindow::updateBottomWidget()
{
    COMPASS& compass = COMPASS::instance();

    assert (db_label_);

    if (compass.dbOpened())
        db_label_->setText(("DB: "+compass.lastDbFilename()).c_str());
    else
        db_label_->setText("No Database");

    assert (status_label_);
    status_label_->setText(compass.appModeStr().c_str());

    assert (load_button_);
    assert (live_pause_resume_button_);

    AppMode app_mode = compass.appMode();

    if (!compass.dbOpened())
    {
        load_button_->setHidden(true);

        live_pause_resume_button_->setHidden(true);

        if (live_stop_button_)
            live_stop_button_->setHidden(true);
    }
    else if (app_mode == AppMode::Offline)
    {
        load_button_->setHidden(false);

        live_pause_resume_button_->setHidden(true);

        if (live_stop_button_)
            live_stop_button_->setHidden(true);
    }
    else if (app_mode == AppMode::LivePaused)
    {
        load_button_->setHidden(false);

        live_pause_resume_button_->setHidden(false);
        live_pause_resume_button_->setText("Resume");

        if (live_stop_button_)
            live_stop_button_->setHidden(false);
    }
    else if (app_mode == AppMode::LiveRunning)
    {
        load_button_->setHidden(true);

        live_pause_resume_button_->setHidden(false);
        live_pause_resume_button_->setText("Pause");

        if (live_stop_button_)
            live_stop_button_->setHidden(false);
    }
    else
        logerr << "MainWindow: updateBottomWidget: unknown app mode " << (unsigned int) COMPASS::instance().appMode();
}

void MainWindow::disableConfigurationSaving()
{
    logdbg << "MainWindow: disableConfigurationSaving";
    save_configuration_ = false;

    assert (quit_wo_cfg_sav_action_);
    quit_wo_cfg_sav_action_->setEnabled(save_configuration_);
}

void MainWindow::showEvaluationTab()
{
    assert (!COMPASS::instance().hideEvaluation());

    assert (tab_widget_->count() > 2);
    tab_widget_->setCurrentIndex(2);
}

void MainWindow::showViewPointsTab()
{
    assert (!COMPASS::instance().hideViewpoints());

    if (COMPASS::instance().hideEvaluation())
    {
        assert (tab_widget_->count() > 2);
        tab_widget_->setCurrentIndex(2);
    }
    else
    {
        assert (tab_widget_->count() > 3);
        tab_widget_->setCurrentIndex(3);
    }
}

void MainWindow::importDataSourcesFile(const std::string& filename)
{
    loginf << "MainWindow: importDataSourcesFile: filename '" << filename << "'";

    automatic_tasks_defined_ = true;
    data_sources_import_file_ = true;
    data_sources_import_filename_ = filename;
}

void MainWindow::importViewPointsFile(const std::string& filename)
{
    loginf << "MainWindow: importViewPointsFile: filename '" << filename << "'";

    automatic_tasks_defined_ = true;
    view_points_import_file_ = true;
    view_points_import_filename_ = filename;
}

void MainWindow::openExistingDB(const std::string& filename)
{
    loginf << "MainWindow: openExistingDB: filename '" << filename << "'";

    COMPASS::instance().openDBFile(filename);

    updateBottomWidget();
    updateMenus();
}

void MainWindow::createDB(const std::string& filename)
{
    loginf << "MainWindow: createDB: filename '" << filename << "'";

    COMPASS::instance().createNewDBFile(filename);

    updateBottomWidget();
    updateMenus();
}


void MainWindow::createAndOpenNewSqlite3DB(const std::string& filename)
{
    loginf << "MainWindow: createAndOpenNewSqlite3DB: filename '" << filename << "'";

    automatic_tasks_defined_ = true;
    sqlite3_create_new_db_ = true;
    sqlite3_create_new_db_filename_ = filename;
}

void MainWindow::openSqlite3DB(const std::string& filename)
{
    loginf << "MainWindow: openSqlite3DB: filename '" << filename << "'";

    automatic_tasks_defined_ = true;
    sqlite3_open_db_ = true;
    sqlite3_open_db_filename_ = filename;
}

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

int MainWindow::importAsterixNetworkMaxLines() const
{
    return asterix_import_network_max_lines_;
}

void MainWindow::importAsterixNetworkMaxLines(int value)
{
    loginf << "MainWindow: importAsterixNetworkMaxLines: value " << value;

    asterix_import_network_max_lines_ = value;
}

void MainWindow::importJSONFile(const std::string& filename)
{
    loginf << "MainWindow: importJSONFile: filename '" << filename << "'";

    automatic_tasks_defined_ = true;

    json_import_filename_ = filename;
}

void MainWindow::importGPSTrailFile(const std::string& filename)
{
    automatic_tasks_defined_ = true;
    gps_trail_import_file_ = true;
    gps_trail_import_filename_ = filename;
}

void MainWindow::importSectorsFile(const std::string& filename)
{
    automatic_tasks_defined_ = true;
    sectors_import_file_ = true;
    sectors_import_filename_ = filename;
}


void MainWindow::calculateRadarPlotPositions(bool value)
{
    loginf << "MainWindow: calculateRadarPlotPositions: value " << value;

    automatic_tasks_defined_ = true;
    calculate_radar_plot_postions_ = value;
}

void MainWindow::associateData(bool value)
{
    loginf << "MainWindow: associateData: value " << value;

    automatic_tasks_defined_ = true;
    associate_data_ = value;
}

void MainWindow::loadData(bool value)
{
    loginf << "MainWindow: loadData: value " << value;

    automatic_tasks_defined_ = true;
    load_data_ = value;
}

void MainWindow::exportViewPointsReportFile(const std::string& filename)
{
    loginf << "TaskManager: exportViewPointsReport: file '" << filename << "'";

    automatic_tasks_defined_ = true;
    export_view_points_report_ = true;
    export_view_points_report_filename_ = filename;
}

void MainWindow::exportEvalReportFile(const std::string& filename)
{
    automatic_tasks_defined_ = true;
    export_eval_report_ = true;
    export_eval_report_filename_ = filename;
}

void MainWindow::evaluateRunFilter(bool value)
{
    automatic_tasks_defined_ = true;
    evaluate_run_filter_ = value;
}

void MainWindow::evaluate(bool evaluate)
{
    automatic_tasks_defined_ = true;
    evaluate_ = evaluate;
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

    if (!(sqlite3_create_new_db_ || sqlite3_open_db_ || data_sources_import_file_ || quit_))
    {
        logerr << "MainWindow: performAutomaticTasks: neither create nor open db nor ds import is set";
        return;
    }

    if (sqlite3_create_new_db_ && sqlite3_open_db_)
    {
        logerr << "MainWindow: performAutomaticTasks: both create and open sqlite3 are set";
        return;
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (sqlite3_create_new_db_)
    {
        loginf << "MainWindow: performAutomaticTasks: creating and opening new sqlite3 database '"
               << sqlite3_create_new_db_filename_ << "'";

        if (Files::fileExists(sqlite3_create_new_db_filename_))
            Files::deleteFile(sqlite3_create_new_db_filename_);

        COMPASS::instance().openDBFile(sqlite3_create_new_db_filename_);

        updateBottomWidget();
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
            QApplication::restoreOverrideCursor();
            return;
        }

        COMPASS::instance().openDBFile(sqlite3_open_db_filename_);

        updateBottomWidget();
        updateMenus();
    }

    QApplication::restoreOverrideCursor();

    loginf << "MainWindow: performAutomaticTasks: database opened";

    // do longer wait on startup for things to settle
    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    while ((boost::posix_time::microsec_clock::local_time()-start_time).total_milliseconds() < 50)
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(1);
    }
    // does not show widget
    //QCoreApplication::processEvents();

    // does cause application halt
    //    while (QCoreApplication::hasPendingEvents())
    //        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    loginf << "MainWindow: performAutomaticTasks: waiting done";

    if (data_sources_import_file_)
    {
        loginf << "MainWindow: performAutomaticTasks: importing data sources file '"
               << data_sources_import_filename_ << "'";

        if (!Files::fileExists(data_sources_import_filename_))
        {
            logerr << "MainWindow: performAutomaticTasks: data sources file '" << data_sources_import_filename_
                   << "' does not exist";
            return;
        }

        COMPASS::instance().dataSourceManager().importDataSources(data_sources_import_filename_);
    }

    if (!(sqlite3_create_new_db_ || sqlite3_open_db_))
    {
        if (quit_)
        {
            quitSlot();
            return;
        }

        logerr << "MainWindow: performAutomaticTasks: tasks can not be performed since no db was opened";
        return;
    }

    if (view_points_import_file_)
    {
        loginf << "MainWindow: performAutomaticTasks: importing view points file '"
               << view_points_import_filename_ << "'";

        if (!Files::fileExists(view_points_import_filename_))
        {
            logerr << "MainWindow: performAutomaticTasks: view points file '" << view_points_import_filename_
                   << "' does not exist";
            return;
        }

        ViewPointsImportTask& vp_import_task = COMPASS::instance().taskManager().viewPointsImportTask();

        vp_import_task.importFilename(view_points_import_filename_);

        assert(vp_import_task.canRun());
        vp_import_task.showDoneSummary(false);

        vp_import_task.run();
    }

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

        ast_import_task.importFilename(asterix_import_filename_);

        assert(ast_import_task.canRun());
        ast_import_task.showDoneSummary(false);

        ast_import_task.run(false); // no test

        while (!ast_import_task.done())
        {
            if (QCoreApplication::hasPendingEvents())
                QCoreApplication::processEvents();

            QThread::msleep(1);
        }
    }

    if (asterix_import_network_)
    {
        loginf << "MainWindow: performAutomaticTasks: importing ASTERIX from network";

        ASTERIXImportTask& ast_import_task = COMPASS::instance().taskManager().asterixImporterTask();

        ast_import_task.importNetwork();

        if (ast_import_task.canRun())
        {
            ast_import_task.showDoneSummary(false);

            ast_import_task.run(false); // no test
        }
        else
            logwrn << "MainWindow: performAutomaticTasks: importing ASTERIX from network not possible";
    }


    if (json_import_filename_.size())
    {
        loginf << "MainWindow: performAutomaticTasks: importing JSON file '"
               << json_import_filename_ << "'";

        if (!Files::fileExists(json_import_filename_))
        {
            logerr << "MainWindow: performAutomaticTasks: JSON file '" << json_import_filename_
                   << "' does not exist";
            return;
        }

        //        if(!json_import_task_->hasSchema(json_import_schema_))
        //        {
        //            logerr << "MainWindow: performAutomaticTasks: JSON schema '" << json_import_schema_
        //                   << "' does not exist";
        //            return;
        //        }

        //        widget_->setCurrentTask(*json_import_task_);
        //        if(widget_->getCurrentTaskName() != json_import_task_->name())
        //        {
        //            logerr << "MainWindow: performAutomaticTasks: wrong task '" << widget_->getCurrentTaskName()
        //                   << "' selected, aborting";
        //            return;
        //        }

        //        JSONImportTaskWidget* json_import_task_widget =
        //                dynamic_cast<JSONImportTaskWidget*>(json_import_task_->widget());
        //        assert(json_import_task_widget);

        //        json_import_task_widget->addFile(json_import_filename_);
        //        json_import_task_widget->selectFile(json_import_filename_);
        //        json_import_task_widget->selectSchema(json_import_schema_);

        //        assert(json_import_task_->canRun());
        //        json_import_task_->showDoneSummary(false);

        //        widget_->runTask(*json_import_task_);

        JSONImportTask& json_import_task = COMPASS::instance().taskManager().jsonImporterTask();

        json_import_task.importFilename(json_import_filename_);

        assert(json_import_task.canRun());
        json_import_task.showDoneSummary(false);

        json_import_task.run(); // no test

        while (!json_import_task.done())
        {
            if (QCoreApplication::hasPendingEvents())
                QCoreApplication::processEvents();

            QThread::msleep(1);
        }

        loginf << "MainWindow: performAutomaticTasks: importing JSON file done";
    }

    if (gps_trail_import_file_)
    {
        loginf << "MainWindow: performAutomaticTasks: importing GPS trail file '"
               << gps_trail_import_filename_ << "'";

        if (!Files::fileExists(gps_trail_import_filename_))
        {
            logerr << "MainWindow: performAutomaticTasks: GPS trail file '" << gps_trail_import_filename_
                   << "' does not exist";
            return;
        }

        GPSTrailImportTask& trail_task = COMPASS::instance().taskManager().gpsTrailImportTask();

        trail_task.importFilename(gps_trail_import_filename_);

        if(!trail_task.canRun())
        {
            logerr << "MainWindow: performAutomaticTasks: gps file can not be imported";
            return;
        }

        trail_task.showDoneSummary(false);
        trail_task.run();

        while (!trail_task.done())
        {
            QCoreApplication::processEvents();
            QThread::msleep(1);
        }
    }

    if (sectors_import_file_)
    {
        loginf << "MainWindow: performAutomaticTasks: importing sectors file '"
               << sectors_import_filename_ << "'";

        if (!Files::fileExists(sectors_import_filename_))
        {
            logerr << "MainWindow: performAutomaticTasks: sectors file file '" << sectors_import_filename_
                   << "' does not exist";
            return;
        }

        COMPASS::instance().evaluationManager().importSectors(sectors_import_filename_);
    }

    if (calculate_radar_plot_postions_)
    {
        RadarPlotPositionCalculatorTask& task = COMPASS::instance().taskManager().radarPlotPositionCalculatorTask();

        if(!task.canRun())
        {
            logerr << "MainWindow: performAutomaticTasks: calculate radar plot positions task can not be run";
            return;
        }

        task.showDoneSummary(false);
        task.run();

        while (!task.done())
        {
            QCoreApplication::processEvents();
            QThread::msleep(1);
        }
    }

    if (associate_data_)
    {
        CreateAssociationsTask& assoc_task = COMPASS::instance().taskManager().createAssociationsTask();

        if(!assoc_task.canRun())
        {
            logerr << "MainWindow: performAutomaticTasks: associations task can not be run";
            return;
        }

        assoc_task.showDoneSummary(false);
        assoc_task.run();

        while (!assoc_task.done())
        {
            QCoreApplication::processEvents();
            QThread::msleep(1);
        }
    }

    if (load_data_)
    {
        loginf << "MainWindow: performAutomaticTasks: loading data";

        DBContentManager& obj_man = COMPASS::instance().dbContentManager();

        obj_man.load();

        while (obj_man.loadInProgress())
        {
            QCoreApplication::processEvents();
            QThread::msleep(1);
        }
    }
    else
        loginf << "MainWindow: performAutomaticTasks: not loading data";

    if (export_view_points_report_)
    {
        loginf << "MainWindow: performAutomaticTasks: exporting view points report";

        showViewPointsTab();

        ViewPointsReportGenerator& gen = COMPASS::instance().viewManager().viewPointsGenerator();

        ViewPointsReportGeneratorDialog& dialog = gen.dialog();
        dialog.show();

        QCoreApplication::processEvents();

        gen.reportPathAndFilename(export_view_points_report_filename_);
        gen.showDone(false);

        gen.run();

        while (gen.isRunning()) // not sure if needed here but what the hell
        {
            QCoreApplication::processEvents();
            QThread::msleep(1);
        }

        gen.showDone(true);
    }

    if (evaluate_ || export_eval_report_)
    {
        loginf << "MainWindow: performAutomaticTasks: running evaluation";

        showEvaluationTab();

        EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

        if (eval_man.canLoadData())
        {
            loginf << "MainWindow: performAutomaticTasks: loading evaluation data";

            eval_man.loadData();

            while (!eval_man.dataLoaded())
            {
                QCoreApplication::processEvents();
                QThread::msleep(1);
            }

            assert (eval_man.dataLoaded());

            if (evaluate_run_filter_)
                eval_man.autofilterUTNs();

            if (eval_man.canEvaluate())
            {
                loginf << "MainWindow: performAutomaticTasks: doing evaluation";

                eval_man.evaluate();

                assert (eval_man.evaluated());

                loginf << "MainWindow: performAutomaticTasks: evaluation done";

                if (export_eval_report_)
                {
                    if (eval_man.canGenerateReport())
                    {
                        loginf << "MainWindow: performAutomaticTasks: generating report";

                        EvaluationResultsReport::PDFGenerator& gen = eval_man.pdfGenerator();

                        EvaluationResultsReport::PDFGeneratorDialog& dialog = gen.dialog();
                        dialog.show();

                        QCoreApplication::processEvents();

                        gen.reportPathAndFilename(export_eval_report_filename_);
                        gen.showDone(false);

                        gen.run();

                        while (gen.isRunning()) // not sure if needed here but what the hell
                        {
                            QCoreApplication::processEvents();
                            QThread::msleep(1);
                        }

                        gen.showDone(true);

                        loginf << "MainWindow: performAutomaticTasks: generating evaluation report done";
                    }
                    else
                        logerr << "MainWindow: performAutomaticTasks: "
                                  "exporting evaluation report not possible since report can't be generated";
                }
            }
            else
                logerr << "MainWindow: performAutomaticTasks: "
                          "evaluation not possible since evaluation can not be performed";
        }
    }

    if (quit_)
    {
        loginf << "MainWindow: performAutomaticTasks: quit requested";

        start_time = boost::posix_time::microsec_clock::local_time();

        while ((boost::posix_time::microsec_clock::local_time()-start_time).total_milliseconds() < 2000)
        {
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            QThread::msleep(1);
        }

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
        createDB(filename);
}

void MainWindow::openExistingDBSlot()
{
    loginf << "MainWindow: openExistingDBSlot";

    string filename = QFileDialog::getOpenFileName(this, "Open SQLite3 File").toStdString();

    if (filename.size() > 0)
        openExistingDB(filename);
}

void MainWindow::openRecentDBSlot()
{
    loginf << "MainWindow: openRecentDBSlot";

    QAction* action = dynamic_cast<QAction*> (QObject::sender());
    assert (action);

    string filename = action->data().toString().toStdString();

    assert (filename.size());

    openExistingDB(filename);
}

void MainWindow::exportDBSlot()
{
    loginf << "MainWindow: exportDBSlot";

    string filename = QFileDialog::getSaveFileName(this, "Export DB SQLite3 File").toStdString();

    if (filename.size() > 0)
    {
        COMPASS::instance().exportDBFile(filename);
    }
}

void MainWindow::clearExistingDBsSlot()
{
    loginf << "MainWindow: clearExistingDBsSlot";

    COMPASS::instance().clearDBFileList();

    updateBottomWidget();
    updateMenus();
}

void MainWindow::closeDBSlot()
{
    loginf << "MainWindow: closeDBSlot";

    COMPASS::instance().closeDB();

    updateBottomWidget();
    updateMenus();
}

void MainWindow::saveConfigSlot()
{
    loginf << "MainWindow: saveConfigSlot";

    assert (!COMPASS::instance().disableMenuConfigSave());

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

        COMPASS::instance().taskManager().asterixImporterTask().dialog()->updateSource();
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

    COMPASS::instance().taskManager().asterixImporterTask().importFilename(filename);

    updateMenus();

    COMPASS::instance().taskManager().asterixImporterTask().dialog()->updateSource();
    COMPASS::instance().taskManager().asterixImporterTask().dialog()->show();
}

void MainWindow::clearImportRecentAsterixRecordingsSlot()
{
    loginf << "MainWindow: clearImportRecentAsterixRecordingsSlot";

    COMPASS::instance().taskManager().asterixImporterTask().clearFileList();

    updateMenus();
}

void MainWindow::importAsterixFromNetworkSlot()
{
    loginf << "MainWindow: importAsterixFromNetworkSlot";

    COMPASS::instance().taskManager().asterixImporterTask().importNetwork();

    COMPASS::instance().taskManager().asterixImporterTask().dialog()->updateSource();
    COMPASS::instance().taskManager().asterixImporterTask().dialog()->show();
}

void MainWindow::importJSONRecordingSlot()
{
    string filename = QFileDialog::getOpenFileName(this, "Import JSON File", "", "JSON Files (*.json *.zip)").toStdString();

    if (filename.size() > 0)
    {
        COMPASS::instance().taskManager().jsonImporterTask().importFilename(filename); // also adds

        updateMenus();

        COMPASS::instance().taskManager().jsonImporterTask().dialog()->updateSource();
        COMPASS::instance().taskManager().jsonImporterTask().dialog()->show();
    }

}

void MainWindow::importGPSTrailSlot()
{
    string filename = QFileDialog::getOpenFileName(this, "Import GPS Trail", "",
                                                   tr("Text Files (*.nmea *.txt)")).toStdString();

    if (filename.size() > 0)
    {
        COMPASS::instance().taskManager().gpsTrailImportTask().importFilename(filename);

        updateMenus();

        COMPASS::instance().taskManager().gpsTrailImportTask().dialog()->show();
    }
}

void MainWindow::importViewPointsSlot()
{
    string filename = QFileDialog::getOpenFileName(this, "Import View Points", "", "*.json").toStdString();

    if (filename.size() > 0)
    {
        COMPASS::instance().taskManager().viewPointsImportTask().importFilename(filename);

        updateMenus();

        COMPASS::instance().taskManager().viewPointsImportTask().dialog()->show();
    }
}

void MainWindow::calculateRadarPlotPositionsSlot()
{
    loginf << "MainWindow: calculateRadarPlotPositionsSlot";

    COMPASS::instance().taskManager().radarPlotPositionCalculatorTask().dialog()->show();
}

void MainWindow::calculateAssociationsARTASSlot()
{
    loginf << "MainWindow: calculateAssociationsARTASSlot";

    COMPASS::instance().taskManager().createArtasAssociationsTask().dialog()->show();
}

void MainWindow::calculateAssociationsSlot()
{
    loginf << "MainWindow: calculateAssociationsSlot";

    COMPASS::instance().taskManager().createAssociationsTask().dialog()->show();
}

void MainWindow::configureDataSourcesSlot()
{
    loginf << "MainWindow: configureDataSourcesSlot";

    COMPASS::instance().dataSourceManager().configurationDialog()->show();
}

void MainWindow::configureMetaVariablesSlot()
{
    loginf << "MainWindow: configureMetaVariablesSlot";

    COMPASS::instance().dbContentManager().metaVariableConfigdialog()->show();
}

void MainWindow::configureSectorsSlot()
{
    loginf << "MainWindow: configureSectorsSlot";

    COMPASS::instance().taskManager().manageSectorsTask().dialog()->show();
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

    assert (!COMPASS::instance().disableAddRemoveViews());
    COMPASS::instance().viewManager().showMainViewContainerAddView();
}

void MainWindow::resetViewsMenuSlot()
{
    loginf << "MainWindow: resetViewsMenuSlot";

    QMessageBox::StandardButton reply;

    if (COMPASS::instance().disableConfirmResetViews())
        reply = QMessageBox::Yes;
    else
    {

        reply = QMessageBox::question(
                    this, "Reset Views",
                    "Confirm to enable all data sources, reset labels,\n"
                    "disable all filters and reset Views to startup configuration?",
                    QMessageBox::Yes|QMessageBox::No);
    }

    if (reply == QMessageBox::Yes)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        assert (tab_widget_);
        int index = tab_widget_->currentIndex();

        QMessageBox msg_box;
        msg_box.setWindowTitle("Resetting");
        msg_box.setText( "Please wait...");
        msg_box.setStandardButtons(QMessageBox::NoButton);
        msg_box.setWindowModality(Qt::ApplicationModal);
        msg_box.show();

        boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

        setVisible(false);

        while ((boost::posix_time::microsec_clock::local_time()-start_time).total_milliseconds() < 50)
        {
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            QThread::msleep(1);
        }

        // reset stuff
        COMPASS::instance().dbContentManager().resetToStartupConfiguration();

        COMPASS::instance().dataSourceManager().resetToStartupConfiguration();

        COMPASS::instance().filterManager().resetToStartupConfiguration();

#ifdef USE_EXPERIMENTAL_SOURCE
        GeometryTreeItem::clearHiddenIdentifierStrs(); // clears hidden layers
#endif

        COMPASS::instance().viewManager().resetToStartupConfiguration();

        // set AppMode
        if (COMPASS::instance().appMode() == AppMode::LivePaused)
            COMPASS::instance().appMode(AppMode::LiveRunning);
        else
        {
            COMPASS::instance().viewManager().appModeSwitchSlot(
                        COMPASS::instance().appMode(), COMPASS::instance().appMode());
        }

        msg_box.close();

        tab_widget_->setCurrentIndex(index);

        setVisible(true);

        QApplication::restoreOverrideCursor();
    }
}

void MainWindow::appModeSwitchSlot (AppMode app_mode_previous, AppMode app_mode_current)
{
    bool enable_tabs = app_mode_current == AppMode::Offline;

    loginf << "MainWindow: appModeSwitch: app_mode " << COMPASS::instance().appModeStr()
           << " enable_tabs " << enable_tabs;

    if (COMPASS::instance().hideEvaluation() && COMPASS::instance().hideViewpoints())
    {
        // nothing
    }
    else if (!COMPASS::instance().hideEvaluation() && !COMPASS::instance().hideViewpoints())
    {
        // both
        assert (tab_widget_->count() > 3); // 2 eval, 3 vp

        tab_widget_->setTabEnabled(2, enable_tabs);
        tab_widget_->setTabEnabled(3, enable_tabs);
    }
    else
    {
        // one
        assert (tab_widget_->count() > 2); // 2 is the other

        tab_widget_->setTabEnabled(2, enable_tabs);
    }

    updateBottomWidget();
    updateMenus();

    if (app_mode_current == AppMode::LivePaused)
    {
        assert (!auto_resume_timer_);

        auto_resume_timer_ = new QTimer();

        connect(auto_resume_timer_, &QTimer::timeout, this, &MainWindow::autoResumeTimerSlot);
        auto_resume_timer_->start(COMPASS::instance().autoLiveRunningResumeAskTime() * 60 * 1000); // min -> ms
    }
    else if (auto_resume_timer_)
    {
        delete auto_resume_timer_;
        auto_resume_timer_ = nullptr;
    }

    if (auto_resume_dialog_)
    {
        auto_resume_dialog_->close();
        auto_resume_dialog_ = nullptr;
    }
}

void MainWindow::autoResumeTimerSlot()
{
    loginf << "MainWindow: autoResumeTimerSlot";

    assert (!auto_resume_dialog_);

    auto_resume_dialog_.reset(new AutoResumeDialog(COMPASS::instance().autoLiveRunningResumeAskWaitTime() * 60));
    // min to s
    connect (auto_resume_dialog_.get(), &AutoResumeDialog::resumeSignal, this, &MainWindow::autoResumeResumeSlot);
    connect (auto_resume_dialog_.get(), &AutoResumeDialog::stayPausedSignal, this, &MainWindow::autoResumeStaySlot);

    auto_resume_dialog_->show();
}

void MainWindow::autoResumeResumeSlot()
{
    loginf << "MainWindow: autoResumeResumeSlot";

    assert (auto_resume_dialog_);
    auto_resume_dialog_->close();

    auto_resume_dialog_ = nullptr;

    livePauseResumeSlot();
}

void MainWindow::autoResumeStaySlot()
{
    loginf << "MainWindow: autoResumeStaySlot";

    auto_resume_dialog_->close();

    auto_resume_dialog_ = nullptr;

    // restart timer
    auto_resume_timer_->start(COMPASS::instance().autoLiveRunningResumeAskTime() * 60 * 1000); // min -> ms
}

void MainWindow::loadButtonSlot()
{
    loginf << "MainWindow: loadButtonSlot";

    if (COMPASS::instance().viewManager().getViews().size() == 0)
    {
        QMessageBox m_warning(QMessageBox::Warning, "Loading Not Possible",
                              "There are no Views active, so loading is not possible.",
                              QMessageBox::Ok);

        m_warning.exec();
        return;
    }

    assert(load_button_);

    if (loading_)
    {
        load_button_->setDisabled(true);
        COMPASS::instance().dbContentManager().quitLoading();
        return;
    }

    loading_ = true;
    load_button_->setText("Stop");

    COMPASS::instance().dbContentManager().load();
}

void MainWindow::loadingDoneSlot()
{
    loginf << "MainWindow: loadingDoneSlot";

    assert (load_button_);

    loading_ = false;
    load_button_->setText("Load");
    load_button_->setDisabled(false);
}

void MainWindow::livePauseResumeSlot()
{
    loginf << "MainWindow: livePauseSlot";

    AppMode app_mode = COMPASS::instance().appMode();

    assert (app_mode == AppMode::LivePaused || AppMode::LiveRunning);

    if (app_mode == AppMode::LiveRunning)
        COMPASS::instance().appMode(AppMode::LivePaused);
    else // AppMode::LivePaused)
        COMPASS::instance().appMode(AppMode::LiveRunning);
}

void MainWindow::liveStopSlot()
{
    loginf << "MainWindow: liveStopSlot";

    COMPASS::instance().appMode(AppMode::Offline);
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
        loginf << "MainWindow: shutdown: configuration not saved";

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

void MainWindow::createDebugMenu()
{
#ifdef SHOW_DEBUG_MENU
    auto debug_menu = menuBar()->addMenu("Debug");

    {
        TestLabCollection().appendTestLabs(debug_menu);
    }
#endif
}
