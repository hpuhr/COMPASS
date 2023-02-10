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
#include "radarplotpositioncalculatortask.h"
#include "createassociationstask.h"
#include "logger.h"
#include "util/files.h"
#include "rtcommand_registry.h"
#include "stringconv.h"

#include <QTimer>

#include <boost/program_options.hpp>

using namespace std;
using namespace Utils;

REGISTER_RTCOMMAND(main_window::RTCommandOpenDB)
REGISTER_RTCOMMAND(main_window::RTCommandCreateDB)
REGISTER_RTCOMMAND(main_window::RTCommandCloseDB)
REGISTER_RTCOMMAND(main_window::RTCommandImportViewPointsFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXNetworkStart)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXNetworkStop)
REGISTER_RTCOMMAND(main_window::RTCommandImportJSONFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportGPSTrail)
REGISTER_RTCOMMAND(main_window::RTCommandImportSectorsJSON)
REGISTER_RTCOMMAND(main_window::RTCommandCalculateRadarPlotPositions)
REGISTER_RTCOMMAND(main_window::RTCommandCalculateAssociations)
REGISTER_RTCOMMAND(main_window::RTCommandLoadData)
REGISTER_RTCOMMAND(main_window::RTCommandQuit)

namespace main_window
{

// open db

rtcommand::IsValid  RTCommandOpenDB::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename_), string("File '")+filename_+"' does not exist")

    return RTCommand::valid();
}

bool RTCommandOpenDB::run_impl() const
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

// create db

rtcommand::IsValid  RTCommandCreateDB::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")

    return RTCommand::valid();
}

bool RTCommandCreateDB::run_impl() const
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

bool RTCommandImportDataSourcesFile::run_impl() const
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

bool RTCommandImportViewPointsFile::run_impl() const
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
    condition.type = rtcommand::RTCommandWaitCondition::Type::Signal;
    condition.obj = "compass.taskmanager.asteriximporttask";
    condition.value = "doneSignal(std::string)";
    condition.timeout_ms = -1; // think about max duration
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
        CHECK_RTCOMMAND_INVALID_CONDITION(!date.is_not_a_date_time(), "Given date '"+date_str_+"' invalid")
    }

    if (time_offset_str_.size())
    {
        bool ok {true};

        String::timeFromString(time_offset_str_, &ok);

        CHECK_RTCOMMAND_INVALID_CONDITION(!ok, "Given time offset '"+time_offset_str_+"' invalid")
    }

    return RTCommand::valid();
}

bool RTCommandImportASTERIXFile::run_impl() const
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

    ADD_RTCOMMAND_POS_OPTION(positional, "filename", 1) // give position
}

void RTCommandImportASTERIXFile::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
    RTCOMMAND_GET_VAR_OR_THROW(variables, "framing", std::string, framing_)
    RTCOMMAND_GET_VAR_OR_THROW(variables, "line", std::string, line_id_)
    RTCOMMAND_GET_VAR_OR_THROW(variables, "date", std::string, date_str_)
    RTCOMMAND_GET_VAR_OR_THROW(variables, "time_offset", std::string, time_offset_str_)
}

// import asterix network

RTCommandImportASTERIXNetworkStart::RTCommandImportASTERIXNetworkStart()
    : rtcommand::RTCommand()
{
    condition.type = rtcommand::RTCommandWaitCondition::Type::Delay;
    condition.timeout_ms = 500; // think about max duration
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

bool RTCommandImportASTERIXNetworkStart::run_impl() const
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

    ADD_RTCOMMAND_OPTIONS(options)("ignore_future_ts,i","ignore future timestamps");
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
    condition.type = rtcommand::RTCommandWaitCondition::Type::Delay;
    condition.timeout_ms = 500; // think about max duration
}

bool RTCommandImportASTERIXNetworkStop::run_impl() const
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
    condition.type = rtcommand::RTCommandWaitCondition::Type::Signal;
    condition.obj = "compass.taskmanager.jsonimporttask";
    condition.value = "doneSignal(std::string)";
    condition.timeout_ms = -1; // think about max duration
}

rtcommand::IsValid  RTCommandImportJSONFile::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename_), string("File '")+filename_+"' does not exist")

    return RTCommand::valid();
}

bool RTCommandImportJSONFile::run_impl() const
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
    condition.type = rtcommand::RTCommandWaitCondition::Type::Signal;
    condition.obj = "compass.taskmanager.gpstrailimporttask";
    condition.value = "doneSignal(std::string)";
    condition.timeout_ms = -1; // think about max duration
}

rtcommand::IsValid  RTCommandImportGPSTrail::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename_), string("File '")+filename_+"' does not exist")

    return RTCommand::valid();
}

bool RTCommandImportGPSTrail::run_impl() const
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

bool RTCommandImportSectorsJSON::run_impl() const
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
    condition.type = rtcommand::RTCommandWaitCondition::Type::Signal;
    condition.obj = "compass.taskmanager.radarplotpositioncalculatortask";
    condition.value = "doneSignal(std::string)";
    condition.timeout_ms = -1; // think about max duration
}

bool RTCommandCalculateRadarPlotPositions::run_impl() const
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
    condition.type = rtcommand::RTCommandWaitCondition::Type::Signal;
    condition.obj = "compass.taskmanager.createassociationstask";
    condition.value = "doneSignal(std::string)";
    condition.timeout_ms = -1; // think about max duration
}

bool RTCommandCalculateAssociations::run_impl() const
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

// load data
RTCommandLoadData::RTCommandLoadData()
    : rtcommand::RTCommand()
{
    condition.type = rtcommand::RTCommandWaitCondition::Type::Signal;
    condition.obj = "compass.dbcontentmanager";
    condition.value = "loadingDoneSignal";
    condition.timeout_ms = -1; // think about max duration
}

bool RTCommandLoadData::run_impl() const
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

// close db

bool RTCommandCloseDB::run_impl() const
{
    if (!COMPASS::instance().dbOpened())
        return false;

    if (COMPASS::instance().appMode() != AppMode::Offline)
        return false;

    MainWindow* main_window = dynamic_cast<MainWindow*> (rtcommand::mainWindow());
    assert (main_window);

    main_window->closeDBSlot();

    return !COMPASS::instance().dbOpened();
}

// quit app

bool RTCommandQuit::run_impl() const
{
    MainWindow* main_window = dynamic_cast<MainWindow*> (rtcommand::mainWindow());
    assert (main_window);

    QTimer::singleShot(100, [main_window] () { main_window->quitSlot(); });

    return true;
}

}
