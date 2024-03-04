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
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/target/targetlistwidget.h"
#include "datasourcesloadwidget.h"
#include "dbcontent/variable/metavariableconfigurationdialog.h"
#include "files.h"
#include "filtermanager.h"
#include "filtermanagerwidget.h"
#include "global.h"
#include "logger.h"
#include "taskmanager.h"
#include "viewmanager.h"
#include "viewpointsimporttask.h"
#include "viewpointsimporttaskdialog.h"
#include "gpstrailimporttask.h"
#include "gpstrailimporttaskdialog.h"
#include "gpsimportcsvtask.h"
#include "gpsimportcsvtaskdialog.h"
#include "managesectorstask.h"
#include "managesectorstaskdialog.h"
#include "calculatereferencestask.h"
#include "calculatereferencestaskdialog.h"
#include "evaluationmanager.h"
#include "compass.h"
#include "fftmanager.h"
#include "fftsconfigurationdialog.h"
#include "view.h"
#include "ui_test_common.h"
#include "ui_test_cmd.h"
#include "rtcommand_shell.h"

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

#ifdef USE_EXPERIMENTAL_SOURCE
#include "geometrytreeitem.h"
#include "test_lab.h"
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
#include <QShortcut>

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
    tab_widget_->addTab(COMPASS::instance().dbContentManager().targetListWidget(), "Targets");

    QTabBar *tabBar = tab_widget_->tabBar();

    tabBar->setTabButton(1, QTabBar::LeftSide, COMPASS::instance().filterManager().widget()->filtersCheckBox());
    //tabBar->setTabButton(0, QTabBar::RightSide, new QLabel("label0");

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

    load_button_->setObjectName("reload");

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

    connect(&COMPASS::instance().dbContentManager(), &DBContentManager::loadingDoneSignal,
                     this, &MainWindow::loadingDoneSlot);
    connect (&COMPASS::instance().dbContentManager(), &DBContentManager::associationStatusChangedSignal,
             this, &MainWindow::updateMenus);

    //init ui related commands
    ui_test::initUITestCommands();

    main_window::init_commands();
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
    QMenu* file_menu = menuBar()->addMenu("&File");
    file_menu->setObjectName("main_window_file_menu");
    file_menu->setToolTipsVisible(true);

    // db operations
    new_db_action_ = new QAction("&New");
    new_db_action_->setShortcuts(QKeySequence::New);
    new_db_action_->setToolTip("Create a new database");
    connect(new_db_action_, &QAction::triggered, this, &MainWindow::newDBSlot);
    file_menu->addAction(new_db_action_);

    open_existing_db_action_ = new QAction("&Open");
    open_existing_db_action_->setShortcuts(QKeySequence::Open);
    open_existing_db_action_->setToolTip("Open an existing database");
    connect(open_existing_db_action_, &QAction::triggered, this, &MainWindow::openExistingDBSlot);
    file_menu->addAction(open_existing_db_action_);

    open_recent_db_menu_ = file_menu->addMenu("Open Recent");
    open_recent_db_menu_->setToolTip("Open a recent database");

    open_recent_db_menu_->addSeparator();

    export_db_action_ = new QAction("&Export", file_menu);
    export_db_action_->setObjectName("main_window_exportdb_action");
    export_db_action_->setToolTip("Export database into file");
    connect(export_db_action_, &QAction::triggered, this, &MainWindow::exportDBSlot);
    file_menu->addAction(export_db_action_);

    QAction* clear_act = new QAction("Clear");
    connect(clear_act, &QAction::triggered, this, &MainWindow::clearExistingDBsSlot);
    open_recent_db_menu_->addAction(clear_act);

    close_db_action_ = new QAction("&Close");
    close_db_action_->setToolTip("Close opened database");
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
    quit_wo_cfg_sav_action_ = new QAction("Quit &Without Saving Config");
    quit_wo_cfg_sav_action_->setShortcut(tr("Ctrl+W"));
    quit_wo_cfg_sav_action_->setToolTip("Quit the application withour saving the configuration");
    connect(quit_wo_cfg_sav_action_, &QAction::triggered, this, &MainWindow::quitWOConfigSlot);
    file_menu->addAction(quit_wo_cfg_sav_action_);

    QAction* quit_act = new QAction("&Quit");
    quit_act->setShortcuts(QKeySequence::Quit);
    //QKeySequence("Ctrl+P");
    quit_act->setToolTip("Quit the application");
    connect(quit_act, &QAction::triggered, this, &MainWindow::quitSlot);
    file_menu->addAction(quit_act);

    // import menu

    import_menu_ = menuBar()->addMenu("&Import");
    import_menu_->setToolTipsVisible(true);

    QAction* import_ast_file_action = new QAction("&ASTERIX Recording");
    import_ast_file_action->setShortcut(tr("Ctrl+A"));
    import_ast_file_action->setToolTip("Import ASTERIX Recording File");
    connect(import_ast_file_action, &QAction::triggered, this, &MainWindow::importAsterixRecordingSlot);
    import_menu_->addAction(import_ast_file_action);

    QAction* import_pcap_file_action = new QAction("&PCAP Network Recording");
    import_pcap_file_action->setShortcut(tr("Ctrl+P"));
    import_pcap_file_action->setToolTip("Import ASTERIX data from PCAP network recording");
    connect(import_pcap_file_action, &QAction::triggered, this, &MainWindow::importAsterixFromPCAPSlot);
    import_menu_->addAction(import_pcap_file_action);

    QAction* import_ast_net_action = new QAction("ASTERIX From Network");
    import_ast_net_action->setToolTip("Import ASTERIX From Network");
    connect(import_ast_net_action, &QAction::triggered, this, &MainWindow::importAsterixFromNetworkSlot);
    import_menu_->addAction(import_ast_net_action);

    QAction* import_json_file_action = new QAction("&JSON Recording");
    import_json_file_action->setShortcut(tr("Ctrl+J"));
    import_json_file_action->setToolTip("Import JSON Recording File");
    connect(import_json_file_action, &QAction::triggered, this, &MainWindow::importJSONRecordingSlot);
    import_menu_->addAction(import_json_file_action);

    QAction* import_gps_nmea_action = new QAction("&GPS Trail NMEA");
    import_gps_nmea_action->setShortcut(tr("Ctrl+G"));
    import_gps_nmea_action->setToolTip("Import GPS Trail NMEA File");
    connect(import_gps_nmea_action, &QAction::triggered, this, &MainWindow::importGPSTrailSlot);
    import_menu_->addAction(import_gps_nmea_action);

    // deactivated, just for porto?
//    QAction* import_gps_csv_action = new QAction("&GPS Trail CSV");
//    import_gps_csv_action->setToolTip("Import GPS Trail CSV File");
//    connect(import_gps_csv_action, &QAction::triggered, this, &MainWindow::importGPSCSVSlot);
//    import_menu_->addAction(import_gps_csv_action);

    if (!COMPASS::instance().hideViewpoints())
    {
        QAction* import_vp_file_action = new QAction("&View Points");
        import_vp_file_action->setShortcut(tr("Ctrl+V"));
        import_vp_file_action->setToolTip("Import View Points File");
        connect(import_vp_file_action, &QAction::triggered, this, &MainWindow::importViewPointsSlot);
        import_menu_->addAction(import_vp_file_action);
    }

    // configuration menu
    config_menu_ = menuBar()->addMenu("&Configuration");
    config_menu_->setToolTipsVisible(true);

    // configure operations
    QAction* ds_action = new QAction("Data Sources");
    ds_action->setToolTip("Configure Data Sources");
    connect(ds_action, &QAction::triggered, this, &MainWindow::configureDataSourcesSlot);
    config_menu_->addAction(ds_action);

    QAction* ffts_action = new QAction("FFTs");
    ffts_action->setToolTip("Configure FFTs");
    connect(ffts_action, &QAction::triggered, this, &MainWindow::configureFFTsSlot);
    config_menu_->addAction(ffts_action);

    QAction* meta_action = new QAction("Meta Variables");

    if (expert_mode)
        meta_action->setToolTip("Configure Meta Variables");
    else
        meta_action->setToolTip("Show Meta Variables");

    connect(meta_action, &QAction::triggered, this, &MainWindow::configureMetaVariablesSlot);
    config_menu_->addAction(meta_action);

    sectors_action_ = new QAction("Sectors");
    sectors_action_->setToolTip("Configure Sectors (stored in Database)");
    connect(sectors_action_, &QAction::triggered, this, &MainWindow::configureSectorsSlot);
    sectors_action_->setDisabled(true);
    config_menu_->addAction(sectors_action_);

    config_menu_->addSeparator();

    ViewManager& view_manager = COMPASS::instance().viewManager();

    auto auto_refresh_views_action = new QAction("Refresh Views Automatically");
    auto_refresh_views_action->setCheckable(true);
    auto_refresh_views_action->setChecked(view_manager.automaticReloadEnabled());
    connect(auto_refresh_views_action, &QAction::toggled, &view_manager, &ViewManager::enableAutomaticReload);
    config_menu_->addAction(auto_refresh_views_action);

    // process menu
    process_menu_ = menuBar()->addMenu("&Process");
    process_menu_->setToolTipsVisible(true);

    QAction* calc_radar_plpos_action = new QAction("Calculate Radar Plot Positions");
    calc_radar_plpos_action->setToolTip("Calculate Radar Plot Positions, only needed if Radar Position information"
                                           " was changed");
    connect(calc_radar_plpos_action, &QAction::triggered, this, &MainWindow::calculateRadarPlotPositionsSlot);
    process_menu_->addAction(calc_radar_plpos_action);

    QAction* assoc_action = new QAction("Calculate Unique Targets");
    assoc_action->setToolTip("Create Unique Targets based on all DB Content");
    connect(assoc_action, &QAction::triggered, this, &MainWindow::calculateAssociationsSlot);
    process_menu_->addAction(assoc_action);

    QAction* assoc_artas_action = new QAction("Calculate ARTAS Target Report Usage");
    assoc_artas_action->setToolTip("Create target report usage based on ARTAS TRI information");
    connect(assoc_artas_action, &QAction::triggered, this, &MainWindow::calculateAssociationsARTASSlot);
    process_menu_->addAction(assoc_artas_action);

    calculate_references_action_ = new QAction("Calculate References");
    calculate_references_action_->setToolTip("Calculate References from System Tracker and ADS-B data");
    connect(calculate_references_action_, &QAction::triggered, this, &MainWindow::calculateReferencesSlot);
    process_menu_->addAction(calculate_references_action_);

    // ui menu
    ui_menu_ = menuBar()->addMenu("&UI");
    ui_menu_->setToolTipsVisible(true);

    QAction* reset_views_action = new QAction("Reset Views");
    reset_views_action->setToolTip(
                "Enable all data sources, reset labels,\n"
                "disable all filters and reset Views to startup configuration");
    connect(reset_views_action, &QAction::triggered, this, &MainWindow::resetViewsMenuSlot);
    ui_menu_->addAction(reset_views_action);

    //debug menu (internal)
    createDebugMenu();
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

    assert (calculate_references_action_);
    calculate_references_action_->setEnabled(COMPASS::instance().dbContentManager().hasAssociations());

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

    assert (tab_widget_->count() > 3);
    tab_widget_->setCurrentIndex(3);
}

void MainWindow::showViewPointsTab()
{
    assert (!COMPASS::instance().hideViewpoints());

    if (COMPASS::instance().hideEvaluation())
    {
        assert (tab_widget_->count() > 3);
        tab_widget_->setCurrentIndex(3);
    }
    else
    {
        assert (tab_widget_->count() > 4);
        tab_widget_->setCurrentIndex(4);
    }
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


void MainWindow::newDBSlot()
{
    loginf << "MainWindow: newDBSlot";

    string filename = QFileDialog::getSaveFileName(
                this, "New SQLite3 File", COMPASS::instance().lastUsedPath().c_str()).toStdString();

    if (filename.size() > 0)
        createDB(filename);
}

void MainWindow::openExistingDBSlot()
{
    loginf << "MainWindow: openExistingDBSlot";

    string filename = QFileDialog::getOpenFileName(
                this, "Open SQLite3 File", COMPASS::instance().lastUsedPath().c_str()).toStdString();

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

    string filename = QFileDialog::getSaveFileName(
                this, "Export DB SQLite3 File", COMPASS::instance().lastUsedPath().c_str()).toStdString();

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

    QFileDialog dialog(this, "Import ASTERIX File(s)");
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setDirectory(COMPASS::instance().lastUsedPath().c_str());

    ASTERIXImportTask& task = COMPASS::instance().taskManager().asterixImporterTask();

    task.source().setSourceType(ASTERIXImportSource::SourceType::FileASTERIX);

    if (dialog.exec())
    {
        QStringList filenames = dialog.selectedFiles();

        vector<string> filenames_vec;

        for (auto& filename : filenames)
        {
            assert (Files::fileExists(filename.toStdString()));
            COMPASS::instance().lastUsedPath(Files::getDirectoryFromPath(filename.toStdString()));

            filenames_vec.push_back(filename.toStdString());
        }

        task.source().addFiles(filenames_vec);//, file_line);

        //task.addImportFileNames(filenames_vec);

        updateMenus();

        task.runDialog(this);
    }

//    string filename = QFileDialog::getOpenFileName(this, "Import ASTERIX File").toStdString();

//    if (filename.size() > 0)
//    {
//        //COMPASS::instance().taskManager().asterixImporterTask().importFilename(filename); // also adds

//        updateMenus();

//        COMPASS::instance().taskManager().asterixImporterTask().dialog()->updateSource();
//        COMPASS::instance().taskManager().asterixImporterTask().dialog()->show();
//    }
}

void MainWindow::importRecentAsterixRecordingSlot()
{
    loginf << "MainWindow: importRecentAsterixRecordingSlot";

    QAction* action = dynamic_cast<QAction*> (QObject::sender());
    assert (action);

    string filename = action->data().toString().toStdString();

    assert (filename.size());

    auto& task = COMPASS::instance().taskManager().asterixImporterTask();

    task.source().setSourceType(ASTERIXImportSource::SourceType::FileASTERIX, {filename});

    updateMenus();

    task.runDialog(this);
}

void MainWindow::importAsterixFromPCAPSlot()
{
    loginf << "MainWindow: importAsterixFromPCAPSlot";

    auto fn = QFileDialog::getOpenFileName(this, 
                                           "Import PCAP File", 
                                           COMPASS::instance().lastUsedPath().c_str());
    if (fn.isEmpty())
        return;

    ASTERIXImportTask& task = COMPASS::instance().taskManager().asterixImporterTask();

    task.source().setSourceType(ASTERIXImportSource::SourceType::FilePCAP, { fn.toStdString() });

    updateMenus();

    task.runDialog(this);
}

void MainWindow::importAsterixFromNetworkSlot()
{
    loginf << "MainWindow: importAsterixFromNetworkSlot";

    ASTERIXImportTask& task = COMPASS::instance().taskManager().asterixImporterTask();

    task.source().setSourceType(ASTERIXImportSource::SourceType::NetASTERIX);

    task.runDialog(this);
}

void MainWindow::importJSONRecordingSlot()
{
    string filename = QFileDialog::getOpenFileName(
                this, "Import JSON File", COMPASS::instance().lastUsedPath().c_str(),
                "JSON Files (*.json *.zip)").toStdString();

    if (filename.size() > 0)
    {
        COMPASS::instance().taskManager().jsonImporterTask().importFilename(filename); // also adds
        COMPASS::instance().lastUsedPath(Files::getDirectoryFromPath(filename));

        updateMenus();

        COMPASS::instance().taskManager().jsonImporterTask().dialog()->updateSource();
        COMPASS::instance().taskManager().jsonImporterTask().dialog()->show();
    }
}

void MainWindow::importGPSTrailSlot()
{
    string filename = QFileDialog::getOpenFileName(this, "Import GPS Trail",
                                                   COMPASS::instance().lastUsedPath().c_str(),
                                                   "Text Files (*.nmea *.txt)").toStdString();

    if (filename.size() > 0)
    {
        COMPASS::instance().taskManager().gpsTrailImportTask().importFilename(filename);
        COMPASS::instance().lastUsedPath(Files::getDirectoryFromPath(filename));

        updateMenus();

        COMPASS::instance().taskManager().gpsTrailImportTask().dialog()->show();
    }
}

void MainWindow::importGPSCSVSlot()
{
    string filename = QFileDialog::getOpenFileName(this, "Import GPS Trail CSV",
                                                   COMPASS::instance().lastUsedPath().c_str(),
                                                   "Text Files (*.csv *.txt)").toStdString();

    if (filename.size() > 0)
    {
        COMPASS::instance().taskManager().gpsImportCSVTask().importFilename(filename);
        COMPASS::instance().lastUsedPath(Files::getDirectoryFromPath(filename));

        updateMenus();

        COMPASS::instance().taskManager().gpsImportCSVTask().dialog()->show();
    }
}

void MainWindow::importViewPointsSlot()
{
    string filename = QFileDialog::getOpenFileName(this, "Import View Points",
                                                   COMPASS::instance().lastUsedPath().c_str(), "*.json").toStdString();

    if (filename.size() > 0)
    {
        COMPASS::instance().taskManager().viewPointsImportTask().importFilename(filename);
        COMPASS::instance().lastUsedPath(Files::getDirectoryFromPath(filename));

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

void  MainWindow::calculateReferencesSlot()
{
    loginf << "MainWindow: calculateReferencesSlot";

    COMPASS::instance().taskManager().calculateReferencesTask().dialog()->show();
}

void MainWindow::configureDataSourcesSlot()
{
    loginf << "MainWindow: configureDataSourcesSlot";

    COMPASS::instance().dataSourceManager().configurationDialog()->show();
}

void MainWindow::configureFFTsSlot()
{
    loginf << "MainWindow: configureFFTsSlot";

    COMPASS::instance().fftManager().configurationDialog()->show();
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

    emit dataLoaded();
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
    //add debug menu entry
    auto debug_menu = menuBar()->addMenu("Debug");

    //add test lab entries
    #ifdef USE_EXPERIMENTAL_SOURCE
    {
        TestLabCollection().appendTestLabs(debug_menu);
    }
    #endif

    //add command shell
    {
        auto action = debug_menu->addAction("Command Shell");
        connect(action, &QAction::triggered, [ this ] () { this->showCommandShell(); });

        auto shortcut = new QShortcut(this);
        shortcut->setKey(QKeySequence(tr("Ctrl+Alt+S")));
        connect(shortcut, &QShortcut::activated, [ this ] () { this->showCommandShell(); });
    }

    debug_menu->menuAction()->setVisible(!COMPASS::instance().isAppImage());
}

void MainWindow::showCommandShell()
{
    QDialog dlg(this);
    QHBoxLayout* layout = new QHBoxLayout;
    dlg.setLayout(layout);

    rtcommand::RTCommandShell* shell = new rtcommand::RTCommandShell(&dlg);
    layout->addWidget(shell);

    dlg.resize(800, 600);
    dlg.exec();
}
