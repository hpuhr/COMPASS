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
#include "createartasassociationstask.h"
#include "reconstructortask.h"
#include "viewmanager.h"
#include "viewpointsimporttask.h"
//#include "viewpointsimporttaskdialog.h"
#include "eval/results/report/pdfgeneratordialog.h"
#include "viewpointsreportgenerator.h"
#include "viewpointsreportgeneratordialog.h"
#include "evaluationmanager.h"
#include "sectorlayer.h"
#include "logger.h"
#include "util/files.h"
#include "rtcommand_registry.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "util/config.h"

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
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXFiles)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXNetworkStart)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXNetworkStop)
REGISTER_RTCOMMAND(main_window::RTCommandImportJSONFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportGPSTrail)
REGISTER_RTCOMMAND(main_window::RTCommandImportSectorsJSON)
REGISTER_RTCOMMAND(main_window::RTCommandCalculateRadarPlotPositions)
REGISTER_RTCOMMAND(main_window::RTCommandCalculateARTASAssociations)
REGISTER_RTCOMMAND(main_window::RTCommandReconstructReferences)
REGISTER_RTCOMMAND(main_window::RTCommandLoadData)
REGISTER_RTCOMMAND(main_window::RTCommandExportViewPointsReport)
REGISTER_RTCOMMAND(main_window::RTCommandEvaluate)
REGISTER_RTCOMMAND(main_window::RTCommandExportEvaluationReport)
REGISTER_RTCOMMAND(main_window::RTCommandQuit)
REGISTER_RTCOMMAND(main_window::RTCommandGetEvents)
REGISTER_RTCOMMAND(main_window::RTCommandReconfigure)
REGISTER_RTCOMMAND(main_window::RTCommandClientInfo)

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
    main_window::RTCommandImportASTERIXFiles::init();
    main_window::RTCommandImportASTERIXNetworkStart::init();
    main_window::RTCommandImportASTERIXNetworkStop::init();
    main_window::RTCommandImportJSONFile::init();
    main_window::RTCommandImportGPSTrail::init();
    main_window::RTCommandImportSectorsJSON::init();
    main_window::RTCommandCalculateRadarPlotPositions::init();
    main_window::RTCommandReconstructReferences::init();
    main_window::RTCommandLoadData::init();
    main_window::RTCommandExportViewPointsReport::init();
    main_window::RTCommandEvaluate::init();
    main_window::RTCommandExportEvaluationReport::init();
    main_window::RTCommandCloseDB::init();
    main_window::RTCommandQuit::init();
    main_window::RTCommandGetEvents::init();
    main_window::RTCommandReconfigure::init();
    main_window::RTCommandClientInfo::init();
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
        if (assure_open_ && COMPASS::instance().lastDbFilename() == filename_)
        {
            return true;
        }
        else
        {
            setResultMessage("Database already opened");
            return false;
        }
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
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.db’")
        ("assure_open", "Only opens the file if it is not already opened");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename") // give position
}

void RTCommandOpenDB::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
    RTCOMMAND_CHECK_VAR(variables, "assure_open", assure_open_)
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

    ADD_RTCOMMAND_POS_OPTION(positional, "filename") // give position
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

    ADD_RTCOMMAND_POS_OPTION(positional, "filename") // give position
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

    ADD_RTCOMMAND_POS_OPTION(positional, "filename") // give position
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

    vp_import_task.allowUserInteractions(false);

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

    ADD_RTCOMMAND_POS_OPTION(positional, "filename") // give position
}

void RTCommandImportViewPointsFile::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

// import asterix file

RTCommandImportASTERIXFile::RTCommandImportASTERIXFile()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.asteriximporttask.doneSignal", -1); // think about max duration
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
    unsigned int file_line {0};

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
            file_line = String::lineFromStr(line_id_);

            //@TODO: remove if per-file line id works as expected
            import_task.settings().file_line_id_ = file_line;
        }

        if (date_str_.size())
        {
            import_task.settings().date_ = Time::fromDateString(date_str_);
        }

        if (time_offset_str_.size())
        {
            bool ok {true};

            double time_offset = String::timeFromString(time_offset_str_, &ok);
            assert (ok); // was checked in valid

            import_task.settings().override_tod_active_ = true;
            import_task.settings().override_tod_offset_ = time_offset;
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
    assert (Files::fileExists(filename_));

    import_task.source().setSourceType(ASTERIXImportSource::SourceType::FileASTERIX, {filename_});//, file_line);

    if (!import_task.canRun())
    {
        setResultMessage("ASTERIX task can not be run"); // should never happen, checked before
        return false;
    }

    import_task.allowUserInteractions(false);

    import_task.run();

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

    ADD_RTCOMMAND_POS_OPTION(positional, "filename") // give position
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

// import asterix files

RTCommandImportASTERIXFiles::RTCommandImportASTERIXFiles()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.asteriximporttask.doneSignal", -1); // think about max duration
}

rtcommand::IsValid RTCommandImportASTERIXFiles::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filenames_.size(), "Filenames empty")

    for (const auto& filename : split_filenames_)
        CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename), "File '"+filename+"' does not exist")

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

bool RTCommandImportASTERIXFiles::run_impl()
{
    if (!filenames_.size())
    {
        setResultMessage("Filenames empty");
        return false;
    }

    for (const auto& filename : split_filenames_)
    {
        if (!Files::fileExists(filename))
        {
            setResultMessage("File '"+filename+"' does not exist");
            return false;
        }
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

    unsigned int file_line {0};

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
            file_line = String::lineFromStr(line_id_);

            //@TODO: remove if per-file line id works as expected
            import_task.settings().file_line_id_ = file_line;
        }

        if (date_str_.size())
        {
            import_task.settings().date_ = Time::fromDateString(date_str_);
        }

        if (time_offset_str_.size())
        {
            bool ok {true};

            double time_offset = String::timeFromString(time_offset_str_, &ok);
            assert (ok); // was checked in valid

            import_task.settings().override_tod_active_ = true;
            import_task.settings().override_tod_offset_ = time_offset;
        }

        if (ignore_time_jumps_)
        {

        }
    }
    catch (exception& e)
    {
        logerr << "RTCommandImportASTERIXFiles: run_impl: setting ASTERIX options resulted in error: " << e.what();
        setResultMessage(string("Setting ASTERIX options resulted in error: ")+e.what());
        return false;
    }


    assert (filenames_.size());

    for (const auto& filename : split_filenames_)
    {
        loginf << "RTCommandImportASTERIXFiles: run_impl: file '" << filename << "'";

        assert (Files::fileExists(filename));
    }

    import_task.source().setSourceType(ASTERIXImportSource::SourceType::FileASTERIX, split_filenames_);//, file_line);

    if (!import_task.canRun())
    {
        setResultMessage("ASTERIX task can not be run"); // should never happen, checked before
        return false;
    }

    import_task.allowUserInteractions(false);

    import_task.run();

    // handle errors

    // if shitty
    //setResultMessage("VP error case 3");
    //return false;

    // if ok
    return true;
}

void RTCommandImportASTERIXFiles::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filenames", po::value<std::string>()->required(), "given filenames, e.g. '/data/file1.ff;/data/file2.ff'");
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

    ADD_RTCOMMAND_POS_OPTION(positional, "filenames") // give position
}

void RTCommandImportASTERIXFiles::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filenames", std::string, filenames_)

    for (string filename : String::split(filenames_, ';'))
    {
        if(filename.rfind("~", 0) == 0)  // change tilde to home path
            filename.replace(0, 1, QDir::homePath().toStdString());

        split_filenames_.push_back(filename);
    }

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

            import_task.settings().override_tod_active_ = true;
            import_task.settings().override_tod_offset_ = time_offset;
        }

        if (max_lines_ != -1)
            import_task.settings().max_network_lines_ = max_lines_;
    }
    catch (exception& e)
    {
        logerr << "RTCommandImportASTERIXFile: run_impl: setting ASTERIX options resulted in error: " << e.what();
        setResultMessage(string("Setting ASTERIX options resulted in error: ")+e.what());
        return false;
    }

    import_task.source().setSourceType(ASTERIXImportSource::SourceType::NetASTERIX);

    if (!import_task.canRun())
    {
        setResultMessage("ASTERIX task can not be run"); // should never happen, checked before
        return false;
    }

    import_task.allowUserInteractions(false);

    import_task.run();

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

    if (!import_task.isRunning() || !import_task.source().isNetworkType())
    {
        setResultMessage("No ASTERIX network import running");
        return false;
    }

    import_task.allowUserInteractions(false);

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
    condition.setSignal("compass.taskmanager.jsonimporttask.doneSignal", -1);  // think about max duration
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

    import_task.allowUserInteractions(false);

    import_task.run();

    return true;
}

void RTCommandImportJSONFile::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.json’");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename") // give position
}

void RTCommandImportJSONFile::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

// import gps trail
RTCommandImportGPSTrail::RTCommandImportGPSTrail()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.gpstrailimporttask.doneSignal", -1); // think about max duration
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

    import_task.allowUserInteractions(false);

    import_task.run();

    return true;
}

void RTCommandImportGPSTrail::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.json’");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename") // give position
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

    try
    {
        COMPASS::instance().evaluationManager().importSectors(filename_);
    }
    catch(const std::exception& e)
    {
        setResultMessage(e.what());
        return false;
    }
    catch(...)
    {
        setResultMessage("Unknown error");
        return false;
    }

    size_t num_sectors = 0;

    assert (COMPASS::instance().evaluationManager().sectorsLoaded());
    const auto& sector_layers = COMPASS::instance().evaluationManager().sectorsLayers();

    for (const auto& sl : sector_layers)
        num_sectors += sl->size();
    
    //return some information on imported sectors
    nlohmann::json reply;
    reply[ "num_sector_layers" ] = sector_layers.size();
    reply[ "num_sectors"       ] = num_sectors;

    setJSONReply(reply);

    return true;
}

void RTCommandImportSectorsJSON::collectOptions_impl(OptionsDescription& options,
                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.json’");

    ADD_RTCOMMAND_POS_OPTION(positional, "filename") // give position
}

void RTCommandImportSectorsJSON::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

// calc radar plot pos
RTCommandCalculateRadarPlotPositions::RTCommandCalculateRadarPlotPositions()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.radarplotpositioncalculatortask.doneSignal", -1); // think about max duration
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

    task.allowUserInteractions(false);
    task.run();

    // if ok
    return true;
}

// associate ARTAS target reports
RTCommandCalculateARTASAssociations::RTCommandCalculateARTASAssociations()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.createartasassociationstask.doneSignal", -1); // think about max duration
}

bool RTCommandCalculateARTASAssociations::run_impl()
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

    CreateARTASAssociationsTask& task = COMPASS::instance().taskManager().createArtasAssociationsTask();

    if(!task.canRun())
    {
        setResultMessage("Calculate ARTAS associations task can not be run");
        return false;
    }

    task.allowUserInteractions(false);
    task.run();

    // if ok
    return true;
}

// calc ref
RTCommandReconstructReferences::RTCommandReconstructReferences()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.reconstructortask.doneSignal", -1); // think about max duration
}

bool RTCommandReconstructReferences::run_impl()
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

    ReconstructorTask& task = COMPASS::instance().taskManager().reconstructReferencesTask();

    if(!task.canRun())
    {
        setResultMessage("Reconstruct references task can not be run");
        return false;
    }

    task.allowUserInteractions(false);
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

    ADD_RTCOMMAND_POS_OPTION(positional, "filename") // give position
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

    ADD_RTCOMMAND_POS_OPTION(positional, "filename") // give position
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
    auto query = max_items_ > 0 ? logger::EventQuery(fresh_, logger::EventQuery::EventType::All, logger::EventQuery::QueryType::Newest, max_items_) :
                                  logger::EventQuery(fresh_, logger::EventQuery::EventType::All, logger::EventQuery::QueryType::All);

    auto json_obj = Logger::getInstance().getEventLog()->getEventsAsJSON(query);

    setJSONReply(json_obj);

    return true;
}

// reconfigure

rtcommand::IsValid RTCommandReconfigure::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(path.empty(), "Path empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(json_config.empty(), "JSON config empty")

    return RTCommand::valid(); 
}

void RTCommandReconfigure::collectOptions_impl(OptionsDescription& options,
                                               PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("path", po::value<std::string>()->required(), "configurable to reconfigure")
        ("config", po::value<std::string>()->required(), "json configuration to apply");

    ADD_RTCOMMAND_POS_OPTION(positional, "path")
    ADD_RTCOMMAND_POS_OPTION(positional, "config")
}

void RTCommandReconfigure::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "path", std::string, path)
    RTCOMMAND_GET_VAR_OR_THROW(variables, "config", std::string, json_config)
}

bool RTCommandReconfigure::run_impl()
{
    //try to find configurable in root
    auto find_result = COMPASS::instance().findSubConfigurable(path);
    if (find_result.first != rtcommand::FindObjectErrCode::NoError)
    {
        setResultMessage("Configurable '" + path + "' not found");
        return false;
    }

    nlohmann::json config;

    try
    {
        config = nlohmann::json::parse(json_config);
    }
    catch(const std::exception& e)
    {
        setResultMessage("Parsing json config failed: " + std::string(e.what()));
        return false;
    }
    catch(...)
    {
        setResultMessage("Parsing json config failed: Unknown error");
        return false;
    }

    //reconfigure using json config
    std::vector<Configurable::MissingKey> missing_subconfig_keys;
    std::vector<Configurable::MissingKey> missing_param_keys;

    auto result = find_result.second->reconfigure(config, &missing_subconfig_keys, &missing_param_keys, false);

    bool ok = false;

    //everything ok?
    if (result.first == Configurable::ReconfigureError::NoError)
        ok = true;
    else if (result.first == Configurable::ReconfigureError::GeneralError)
        setResultMessage(result.second);
    else if (result.first == Configurable::ReconfigureError::PreCheckFailed)
        setResultMessage("Configuration incompatible");
    else if (result.first == Configurable::ReconfigureError::ApplyFailed)
        setResultMessage("Configuration could not be applied");
    else // Configurable::ReconfigureError::UnknownError
        setResultMessage("Unknown error");

    auto missingKeyType2String = [ & ] (Configurable::MissingKeyType type)
    {
        if (type == Configurable::MissingKeyType::CreationFailed)
            return "creation_failed";
        else if (type == Configurable::MissingKeyType::Skipped)
            return "skipped";
        else if (type == Configurable::MissingKeyType::Missing)
            return "missing";
        return "";
    };

    auto createKeyInformation = [ & ] (const std::vector<Configurable::MissingKey>& keys)
    {
        auto missing_keys_vec = nlohmann::json::array();
    
        for (const auto& key : missing_subconfig_keys)
        {
            nlohmann::json entry;
            entry[ "id"   ] = key.first;
            entry[ "type" ] = missingKeyType2String(key.second);

            missing_keys_vec.push_back(entry);
        }

        return missing_keys_vec;
    };
    
    //store missing keys to reply
    nlohmann::json json_reply;

    json_reply[ "missing_subconfig_keys" ] = createKeyInformation(missing_subconfig_keys);
    json_reply[ "missing_param_keys"     ] = createKeyInformation(missing_param_keys    );

    setJSONReply(json_reply);

    return ok;
}

// client_info

bool RTCommandClientInfo::run_impl()
{
    nlohmann::json info;

    info[ "appimage" ] = COMPASS::instance().isAppImage();
    info[ "version"  ] = COMPASS::instance().config().getString("version");
    info[ "appmode"  ] = COMPASS::instance().appModeStr();

    setJSONReply(info);

    return true;
}

}
