#include "mainwindow_commands.h"
#include "mainwindow.h"
#include "compass.h"
#include "datasourcemanager.h"
#include "taskmanager.h"
#include "viewpointsimporttask.h"
#include "asteriximporttask.h"
#include "jsonimporttask.h"
#include "gpstrailimporttask.h"
#include "evaluationmanager.h"
#include "dbcontentmanager.h"
#include "radarplotpositioncalculatortask.h"
#include "createassociationstask.h"
#include "calculatereferencestask.h"
#include "viewmanager.h"
#include "viewpointsimporttask.h"
//#include "viewpointsimporttaskdialog.h"
#include "eval/results/report/pdfgeneratordialog.h"
#include "viewpointsreportgenerator.h"
#include "viewpointsreportgeneratordialog.h"
#include "evaluationmanager.h"
#include "logger.h"
#include "util/files.h"
#include "rtcommand_registry.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "event_log.h"

#include <QTimer>
#include <QCoreApplication>
#include <QThread>

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

using namespace std;
using namespace Utils;

REGISTER_RTCOMMAND(main_window::RTCommandOpenDB)
REGISTER_RTCOMMAND(main_window::RTCommandOpenRecentDB)
REGISTER_RTCOMMAND(main_window::RTCommandCreateDB)
REGISTER_RTCOMMAND(main_window::RTCommandCloseDB)
REGISTER_RTCOMMAND(main_window::RTCommandImportDataSourcesFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportViewPointsFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXNetworkStart)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXNetworkStop)
REGISTER_RTCOMMAND(main_window::RTCommandImportJSONFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportGPSTrail)
REGISTER_RTCOMMAND(main_window::RTCommandImportSectorsJSON)
REGISTER_RTCOMMAND(main_window::RTCommandCalculateRadarPlotPositions)
REGISTER_RTCOMMAND(main_window::RTCommandCalculateAssociations)
REGISTER_RTCOMMAND(main_window::RTCommandCalculateReferences)
REGISTER_RTCOMMAND(main_window::RTCommandLoadData)
REGISTER_RTCOMMAND(main_window::RTCommandExportViewPointsReport)
REGISTER_RTCOMMAND(main_window::RTCommandEvaluate)
REGISTER_RTCOMMAND(main_window::RTCommandExportEvaluationReport)
REGISTER_RTCOMMAND(main_window::RTCommandQuit)
REGISTER_RTCOMMAND(main_window::RTCommandGetEvents)

namespace main_window
{

void init_commands()
{
    main_window::RTCommandOpenDB::init();
    main_window::RTCommandOpenRecentDB::init();
    main_window::RTCommandCreateDB::init();
    main_window::RTCommandImportDataSourcesFile::init();
    main_window::RTCommandImportViewPointsFile::init();
    main_window::RTCommandImportASTERIXFile::init();
    main_window::RTCommandImportASTERIXNetworkStart::init();
    main_window::RTCommandImportASTERIXNetworkStop::init();
    main_window::RTCommandImportJSONFile::init();
    main_window::RTCommandImportGPSTrail::init();
    main_window::RTCommandImportSectorsJSON::init();
    main_window::RTCommandCalculateRadarPlotPositions::init();
    main_window::RTCommandCalculateAssociations::init();
    main_window::RTCommandCalculateReferences::init();
    main_window::RTCommandLoadData::init();
    main_window::RTCommandExportViewPointsReport::init();
    main_window::RTCommandEvaluate::init();
    main_window::RTCommandExportEvaluationReport::init();
    main_window::RTCommandCloseDB::init();
    main_window::RTCommandQuit::init();
    main_window::RTCommandGetEvents::init();
}

// open_db

rtcommand::IsValid RTCommandOpenDB::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename_), string("File '")+filename_+"' does not exist")

    return RTCommand::valid();
}

bool RTCommandOpenDB::run_impl()
{
    if (!filename_.size())
    {
        setResultMessage("Filename empty");
        return false;
    }

    if (!Files::fileExists(filename_))
    {
        setResultMessage("File '"+filename_+"' does not exist");
        return false;
    }

    if (COMPASS::instance().dbOpened())
    {
        setResultMessage("Database already opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    MainWindow* main_window = dynamic_cast<MainWindow*> (rtcommand::mainWindow());
    assert (main_window);

    main_window->openExistingDB(filename_);

    return COMPASS::instance().dbOpened();
}

void RTCommandOpenDB::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.db’");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename", 1) // give position
}

void RTCommandOpenDB::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

// open_recent_db

std::string RTCommandOpenRecentDB::getPath() const
{
    vector<string> recent_file_list = COMPASS::instance().dbFileList();
    if (recent_file_list.empty())
        return "";

    if (!filename.empty())
    {
        for (const auto& fn : recent_file_list)
        {
            if (boost::filesystem::path(fn).filename() == filename)
                return fn;
        }
        return "";
    }

    if (index >= 0)
    {
        if (index < (int)recent_file_list.size())
            return recent_file_list[index];
        
        return "";
    }

    return recent_file_list.front();
}

rtcommand::IsValid RTCommandOpenRecentDB::valid() const
{
    auto fn = getPath();

    CHECK_RTCOMMAND_INVALID_CONDITION(fn.empty(), "No recent file found")

    return RTCommand::valid();
}

bool RTCommandOpenRecentDB::run_impl()
{
    auto fn = getPath();

    if (fn.empty())
    {
        setResultMessage("No recent file found");
        return false;
    }

    if (!Files::fileExists(fn))
    {
        setResultMessage("File '"+fn+"' does not exist");
        return false;
    }

    if (COMPASS::instance().dbOpened())
    {
        setResultMessage("Database already opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    MainWindow* main_window = dynamic_cast<MainWindow*> (rtcommand::mainWindow());
    assert (main_window);

    main_window->openExistingDB(fn);

    return COMPASS::instance().dbOpened();
}

void RTCommandOpenRecentDB::collectOptions_impl(OptionsDescription& options,
                                                PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->default_value(""), "filename listed in the recent file history, e.g. ’file1.db’")
        ("index,i", po::value<int>()->default_value(-1), "index in the recent file history");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename", 1) // give position
}

void RTCommandOpenRecentDB::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename)
    RTCOMMAND_GET_VAR_OR_THROW(variables, "index", int, index)
}

// create db

rtcommand::IsValid  RTCommandCreateDB::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")

    return RTCommand::valid();
}

bool RTCommandCreateDB::run_impl()
{
    if (!filename_.size())
    {
        setResultMessage("Filename empty");
        return false;
    }

    if (COMPASS::instance().dbOpened())
    {
        setResultMessage("Database already opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    MainWindow* main_window = dynamic_cast<MainWindow*> (rtcommand::mainWindow());
    assert (main_window);

    main_window->createDB(filename_);

    return COMPASS::instance().dbOpened();
}

void RTCommandCreateDB::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.db’");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename", 1) // give position
}

void RTCommandCreateDB::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

// import ds

rtcommand::IsValid  RTCommandImportDataSourcesFile::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename_), string("File '")+filename_+"' does not exist")

    return RTCommand::valid();
}

bool RTCommandImportDataSourcesFile::run_impl()
{
    if (!filename_.size())
    {
        setResultMessage("Filename empty");
        return false;
    }

    if (!Files::fileExists(filename_))
    {
        setResultMessage("File '"+filename_+"' does not exist");
        return false;
    }

//    if (!COMPASS::instance().dbOpened())
//    {
//        setResultMessage("Database not opened");
//        return false;
//    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    COMPASS::instance().dataSourceManager().importDataSources(filename_);

    return true;
}

void RTCommandImportDataSourcesFile::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.json'");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename", 1) // give position
}

void RTCommandImportDataSourcesFile::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

// import vp

RTCommandImportViewPointsFile::RTCommandImportViewPointsFile()
    : rtcommand::RTCommand()
{
}

rtcommand::IsValid  RTCommandImportViewPointsFile::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename_), string("File '")+filename_+"' does not exist")

    return RTCommand::valid();
}

bool RTCommandImportViewPointsFile::run_impl()
{
    if (!filename_.size())
    {
        setResultMessage("Filename empty");
        return false;
    }

    if (!Files::fileExists(filename_))
    {
        setResultMessage("File '"+filename_+"' does not exist");
        return false;
    }

    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }
    ViewPointsImportTask& vp_import_task = COMPASS::instance().taskManager().viewPointsImportTask();

    vp_import_task.importFilename(filename_);

    if (!vp_import_task.canRun())
    {
        setResultMessage("Import error '"+vp_import_task.currentError()+"'");
        return false;
    }

    vp_import_task.showDoneSummary(false);

    vp_import_task.run();
    assert (vp_import_task.done());

    // if shitty
    //setResultMessage("VP error case 3");
    //return false;

    // if ok
    return true;
}

void RTCommandImportViewPointsFile::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.json’");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename", 1) // give position
}

void RTCommandImportViewPointsFile::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

// import asterix file

RTCommandImportASTERIXFile::RTCommandImportASTERIXFile()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.asteriximporttask.doneSignal(std::string)", -1); // think about max duration
}

rtcommand::IsValid  RTCommandImportASTERIXFile::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename_), "File '"+filename_+"' does not exist")

    if (framing_.size()) // ’none’, ’ioss’, ’ioss_seq’, ’rff’
    {
        CHECK_RTCOMMAND_INVALID_CONDITION(
                    !(framing_ == "none" || framing_ == "ioss" || framing_ == "ioss_seq" || framing_ == "rff"),
                    string("Framing '")+framing_+"' does not exist")
    }

    if (line_id_.size())
    {
        CHECK_RTCOMMAND_INVALID_CONDITION(
                    !(line_id_ == "L1" || line_id_ == "L2" || line_id_ == "L3" || line_id_ == "L4"),
                    "Line '"+line_id_+"' does not exist")
    }

    if (date_str_.size())
    {
        boost::posix_time::ptime date = Time::fromDateString(date_str_);
        CHECK_RTCOMMAND_INVALID_CONDITION(date.is_not_a_date_time(), "Given date '"+date_str_+"' invalid")
    }

    if (time_offset_str_.size())
    {
        bool ok {true};

        String::timeFromString(time_offset_str_, &ok);

        CHECK_RTCOMMAND_INVALID_CONDITION(!ok, "Given time offset '"+time_offset_str_+"' invalid")
    }

    return RTCommand::valid();
}

bool RTCommandImportASTERIXFile::run_impl()
{
    if (!filename_.size())
    {
        setResultMessage("Filename empty");
        return false;
    }

    if (!Files::fileExists(filename_))
    {
        setResultMessage("File '"+filename_+"' does not exist");
        return false;
    }

    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    ASTERIXImportTask& import_task = COMPASS::instance().taskManager().asterixImporterTask();

    try
    {
        if (framing_.size())
        {
            if (framing_ == "none")
                import_task.asterixFileFraming("");
            else
                import_task.asterixFileFraming(framing_);
        }

        if (line_id_.size())
        {
            unsigned int file_line = String::lineFromStr(line_id_);
            import_task.fileLineID(file_line);
        }

        if (date_str_.size())
        {
            import_task.date(Time::fromDateString(date_str_));
        }

        if (time_offset_str_.size())
        {
            bool ok {true};

            double time_offset = String::timeFromString(time_offset_str_, &ok);
            assert (ok); // was checked in valid

            import_task.overrideTodActive(true);
            import_task.overrideTodOffset(time_offset);
        }

        if (ignore_time_jumps_)
        {

        }
    }
    catch (exception& e)
    {
        logerr << "RTCommandImportASTERIXFile: run_impl: setting ASTERIX options resulted in error: " << e.what();
        setResultMessage(string("Setting ASTERIX options resulted in error: ")+e.what());
        return false;
    }

    assert (filename_.size());

    import_task.importFilename(filename_);

    if (!import_task.canRun())
    {
        setResultMessage("ASTERIX task can not be run"); // should never happen, checked before
        return false;
    }

    import_task.showDoneSummary(false);

    import_task.run(false); // no test

    // handle errors

    // if shitty
    //setResultMessage("VP error case 3");
    //return false;

    // if ok
    return true;
}

void RTCommandImportASTERIXFile::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.json’");
    ADD_RTCOMMAND_OPTIONS(options)
        ("framing,f", po::value<std::string>()->default_value(""),
         "imports ASTERIX file with given  ASTERIX framing, e.g. ’none’, ’ioss’, ’ioss_seq’, ’rff’");
    ADD_RTCOMMAND_OPTIONS(options)
        ("line,l", po::value<std::string>()->default_value(""), "imports ASTERIX file with given line e.g. ’L2’");
    ADD_RTCOMMAND_OPTIONS(options)
        ("date,d", po::value<std::string>()->default_value(""),
         "imports ASTERIX file with given date, in YYYY-MM-DD format e.g. ’2020-04-20’");
    ADD_RTCOMMAND_OPTIONS(options)
        ("time_offset,t", po::value<std::string>()->default_value(""),
         "imports ASTERIX file with given Time of Day override, in HH:MM:SS.ZZZ’");
    ADD_RTCOMMAND_OPTIONS(options)("ignore_time_jumps,i", "ignore 24h time jumps");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename", 1) // give position
}

void RTCommandImportASTERIXFile::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
    RTCOMMAND_GET_VAR_OR_THROW(variables, "framing", std::string, framing_)
    RTCOMMAND_GET_VAR_OR_THROW(variables, "line", std::string, line_id_)
    RTCOMMAND_GET_VAR_OR_THROW(variables, "date", std::string, date_str_)
    RTCOMMAND_GET_VAR_OR_THROW(variables, "time_offset", std::string, time_offset_str_)
    RTCOMMAND_CHECK_VAR(variables, "ignore_time_jumps", ignore_time_jumps_)
}

// import asterix network

RTCommandImportASTERIXNetworkStart::RTCommandImportASTERIXNetworkStart()
    : rtcommand::RTCommand()
{
    condition.setDelay(500); // think about max duration
}

rtcommand::IsValid  RTCommandImportASTERIXNetworkStart::valid() const
{
    if (time_offset_str_.size())
    {
        bool ok {true};

        String::timeFromString(time_offset_str_, &ok);

        CHECK_RTCOMMAND_INVALID_CONDITION(!ok, "Given time offset '"+time_offset_str_+"' invalid")
    }

    return RTCommand::valid();
}

bool RTCommandImportASTERIXNetworkStart::run_impl()
{
    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    ASTERIXImportTask& import_task = COMPASS::instance().taskManager().asterixImporterTask();

    try
    {
        if (time_offset_str_.size())
        {
            bool ok {true};

            double time_offset = String::timeFromString(time_offset_str_, &ok);
            assert (ok); // was checked in valid

            import_task.overrideTodActive(true);
            import_task.overrideTodOffset(time_offset);
        }

        if (max_lines_ != -1)
            import_task.maxNetworkLines(max_lines_);
    }
    catch (exception& e)
    {
        logerr << "RTCommandImportASTERIXFile: run_impl: setting ASTERIX options resulted in error: " << e.what();
        setResultMessage(string("Setting ASTERIX options resulted in error: ")+e.what());
        return false;
    }

    import_task.importNetwork();

    if (!import_task.canRun())
    {
        setResultMessage("ASTERIX task can not be run"); // should never happen, checked before
        return false;
    }

    import_task.showDoneSummary(false);

    import_task.run(false); // no test

    // handle errors

    // if shitty
    //setResultMessage("VP error case 3");
    //return false;

    // if ok
    return true;
}

void RTCommandImportASTERIXNetworkStart::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("time_offset,t", po::value<std::string>()->default_value(""),
         "imports ASTERIX file with given Time of Day override, in HH:MM:SS.ZZZ’");

    ADD_RTCOMMAND_OPTIONS(options)
        ("max_lines,m", po::value<int>()->default_value(-1),
         "maximum number of lines per data source during ASTERIX network import, 1..4");

    ADD_RTCOMMAND_OPTIONS(options)("ignore_future_ts,i", "ignore future timestamps");
}

void RTCommandImportASTERIXNetworkStart::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "time_offset", std::string, time_offset_str_)
    RTCOMMAND_GET_VAR_OR_THROW(variables, "max_lines", int, max_lines_)
    RTCOMMAND_CHECK_VAR(variables, "ignore_future_ts", ignore_future_ts_)
}

// import asterix network stop

RTCommandImportASTERIXNetworkStop::RTCommandImportASTERIXNetworkStop()
    : rtcommand::RTCommand()
{
    condition.setDelay(500); // think about max duration
}

bool RTCommandImportASTERIXNetworkStop::run_impl()
{
    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() == AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    ASTERIXImportTask& import_task = COMPASS::instance().taskManager().asterixImporterTask();

    if (!import_task.isRunning() || !import_task.isImportNetwork())
    {
        setResultMessage("No ASTERIX network import running");
        return false;
    }

    import_task.showDoneSummary(false);

    MainWindow* main_window = dynamic_cast<MainWindow*> (rtcommand::mainWindow());
    assert (main_window);

    main_window->liveStopSlot();

    // handle errors

    // if shitty
    //setResultMessage("VP error case 3");
    //return false;

    // if ok
    return true;
}

// import json
RTCommandImportJSONFile::RTCommandImportJSONFile()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.jsonimporttask.doneSignal(std::string)", -1);  // think about max duration
}

rtcommand::IsValid  RTCommandImportJSONFile::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename_), string("File '")+filename_+"' does not exist")

    return RTCommand::valid();
}

bool RTCommandImportJSONFile::run_impl()
{
    if (!filename_.size())
    {
        setResultMessage("Filename empty");
        return false;
    }

    if (!Files::fileExists(filename_))
    {
        setResultMessage("File '"+filename_+"' does not exist");
        return false;
    }

    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    JSONImportTask& import_task = COMPASS::instance().taskManager().jsonImporterTask();

    import_task.importFilename(filename_);

    if (!import_task.canRun())
    {
        setResultMessage("JSON import task can not be run");
        return false;
    }

    import_task.showDoneSummary(false);

    import_task.run();

    return true;
}

void RTCommandImportJSONFile::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.json’");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename", 1) // give position
}

void RTCommandImportJSONFile::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

// import gps trail
RTCommandImportGPSTrail::RTCommandImportGPSTrail()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.gpstrailimporttask.doneSignal(std::string)", -1); // think about max duration
}

rtcommand::IsValid  RTCommandImportGPSTrail::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename_), string("File '")+filename_+"' does not exist")

    return RTCommand::valid();
}

bool RTCommandImportGPSTrail::run_impl()
{
    if (!filename_.size())
    {
        setResultMessage("Filename empty");
        return false;
    }

    if (!Files::fileExists(filename_))
    {
        setResultMessage("File '"+filename_+"' does not exist");
        return false;
    }

    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    GPSTrailImportTask& import_task = COMPASS::instance().taskManager().gpsTrailImportTask();

    import_task.importFilename(filename_);

    if (!import_task.canRun())
    {
        setResultMessage("GPS trail import task can not be run");
        return false;
    }

    import_task.showDoneSummary(false);

    import_task.run();

    return true;
}

void RTCommandImportGPSTrail::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.json’");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename", 1) // give position
}

void RTCommandImportGPSTrail::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

// import sectors json
rtcommand::IsValid  RTCommandImportSectorsJSON::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename_), string("File '")+filename_+"' does not exist")

    return RTCommand::valid();
}

bool RTCommandImportSectorsJSON::run_impl()
{
    if (!filename_.size())
    {
        setResultMessage("Filename empty");
        return false;
    }

    if (!Files::fileExists(filename_))
    {
        setResultMessage("File '"+filename_+"' does not exist");
        return false;
    }

    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    COMPASS::instance().evaluationManager().importSectors(filename_);

    return true;
}

void RTCommandImportSectorsJSON::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.json’");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename", 1) // give position
}

void RTCommandImportSectorsJSON::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

// calc radar plot pos
RTCommandCalculateRadarPlotPositions::RTCommandCalculateRadarPlotPositions()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.radarplotpositioncalculatortask.doneSignal(std::string)", -1); // think about max duration
}

bool RTCommandCalculateRadarPlotPositions::run_impl()
{
    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    RadarPlotPositionCalculatorTask& task = COMPASS::instance().taskManager().radarPlotPositionCalculatorTask();

    if(!task.canRun())
    {
        setResultMessage("Calculate radar plot positions task can not be run");
        return false;
    }

    task.showDoneSummary(false);
    task.run();

    // if ok
    return true;
}

// associate data
RTCommandCalculateAssociations::RTCommandCalculateAssociations()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.createassociationstask.doneSignal(std::string)", -1); // think about max duration
}

bool RTCommandCalculateAssociations::run_impl()
{
    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    CreateAssociationsTask& task = COMPASS::instance().taskManager().createAssociationsTask();

    if(!task.canRun())
    {
        setResultMessage("Calculate associations task can not be run");
        return false;
    }

    task.showDoneSummary(false);
    task.run();

    // if ok
    return true;
}


// calc ref
RTCommandCalculateReferences::RTCommandCalculateReferences()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.calculatereferencestask.doneSignal(std::string)", -1); // think about max duration
}

bool RTCommandCalculateReferences::run_impl()
{
    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    CalculateReferencesTask& task = COMPASS::instance().taskManager().calculateReferencesTask();

    if(!task.canRun())
    {
        setResultMessage("Calculate references task can not be run");
        return false;
    }

    task.showDoneSummary(false);
    task.run();

    // if ok
    return true;
}

// load data
RTCommandLoadData::RTCommandLoadData()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.dbcontentmanager.loadingDoneSignal", -1); // think about max duration
}

bool RTCommandLoadData::run_impl()
{
    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    MainWindow* main_window = dynamic_cast<MainWindow*> (rtcommand::mainWindow());
    assert (main_window);

    main_window->loadButtonSlot();

    // if ok
    return true;
}

// export vp report
rtcommand::IsValid  RTCommandExportViewPointsReport::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")

    return RTCommand::valid();
}

bool RTCommandExportViewPointsReport::run_impl()
{
    if (!filename_.size())
    {
        setResultMessage("Filename empty");
        return false;
    }

    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    MainWindow* main_window = dynamic_cast<MainWindow*> (rtcommand::mainWindow());
    assert (main_window);

    main_window->showViewPointsTab();

    ViewPointsReportGenerator& gen = COMPASS::instance().viewManager().viewPointsGenerator();

    ViewPointsReportGeneratorDialog& dialog = gen.dialog();
    dialog.show();

    QCoreApplication::processEvents();

    gen.reportPathAndFilename(filename_);
    gen.showDone(false);

    gen.run();

    return true;
}

void RTCommandExportViewPointsReport::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/db2/report.tex'");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename", 1) // give position
}

void RTCommandExportViewPointsReport::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

// evaluate
bool RTCommandEvaluate::run_impl()
{
    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    MainWindow* main_window = dynamic_cast<MainWindow*> (rtcommand::mainWindow());
    assert (main_window);

    if (run_filter_)
        COMPASS::instance().dbContentManager().autoFilterUTNS();

    main_window->showEvaluationTab();

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    if (!eval_man.canLoadData())
    {
        setResultMessage("Unable to load evaluation data");
        return false;
    }

    loginf << "RTCommandEvaluate: run_impl: loading evaluation data";

    eval_man.loadData();

    while (!eval_man.dataLoaded())
    {
        QCoreApplication::processEvents();
        QThread::msleep(1);
    }

    assert (eval_man.dataLoaded());

    if (!eval_man.canEvaluate())
    {
        setResultMessage("Unable to evaluate");
        return false;
    }

    loginf << "RTCommandEvaluate: run_impl: doing evaluation";

    eval_man.evaluate();

    return eval_man.evaluated();
}

void RTCommandEvaluate::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("run_filter,f", "run evaluation filter before evaluation");
}

void RTCommandEvaluate::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_CHECK_VAR(variables, "run_filter", run_filter_)
}

// export evaluation report
rtcommand::IsValid  RTCommandExportEvaluationReport::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")

    return RTCommand::valid();
}

bool RTCommandExportEvaluationReport::run_impl()
{
    if (!filename_.size())
    {
        setResultMessage("Filename empty");
        return false;
    }

    if (!COMPASS::instance().dbOpened())
    {
        setResultMessage("Database not opened");
        return false;
    }

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
    {
        setResultMessage("Wrong application mode "+COMPASS::instance().appModeStr());
        return false;
    }

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    if (!eval_man.evaluated())
    {
        setResultMessage("No evaluation was performed, unable to generate report");
        return false;
    }

    EvaluationResultsReport::PDFGenerator& gen = eval_man.pdfGenerator();

    EvaluationResultsReport::PDFGeneratorDialog& dialog = gen.dialog();
    dialog.show();

    QCoreApplication::processEvents();

    gen.reportPathAndFilename(filename_);
    gen.showDone(false);

    gen.run();

    return true;
}

void RTCommandExportEvaluationReport::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/db2/report.tex'");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename", 1) // give position
}

void RTCommandExportEvaluationReport::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

// close db

void RTCommandCloseDB::collectOptions_impl(OptionsDescription& options,
                                           PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("strict", "fail if no database was opened");
}

void RTCommandCloseDB::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_CHECK_VAR(variables, "strict", strict_)
}

bool RTCommandCloseDB::run_impl()
{
    //lets return true if there's nothing to be done
    if (!COMPASS::instance().dbOpened())
        return !strict_;

    if (COMPASS::instance().appMode() != AppMode::Offline)
        return false;

    MainWindow* main_window = dynamic_cast<MainWindow*> (rtcommand::mainWindow());
    assert (main_window);

    main_window->closeDBSlot();

    return !COMPASS::instance().dbOpened();
}

// quit app

bool RTCommandQuit::run_impl()
{
    MainWindow* main_window = dynamic_cast<MainWindow*> (rtcommand::mainWindow());
    assert (main_window);

    QTimer::singleShot(100, [main_window] () { main_window->quitSlot(); });

    return true;
}

// get_events

void RTCommandGetEvents::collectOptions_impl(OptionsDescription& options,
                                             PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("fresh", "return only fresh (unseen) events")
        ("max_items", po::value<unsigned int>()->default_value(0), "maximum number of items to return");
}

void RTCommandGetEvents::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_CHECK_VAR(variables, "fresh", fresh_)
    RTCOMMAND_GET_VAR_OR_THROW(variables, "max_items", unsigned int, max_items_)
}

bool RTCommandGetEvents::run_impl()
{
    auto query = max_items_ > 0 ? logger::EventQuery(fresh_, logger::EventQuery::Type::Newest, max_items_) :
                                  logger::EventQuery(fresh_, logger::EventQuery::Type::All);

    auto json_obj = Logger::getInstance().getEventLog()->getEventsAsJSON(query);

    setJSONReply(json_obj);

    return true;
}

}
