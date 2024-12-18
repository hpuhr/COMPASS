#include "mainwindow_commands.h"
#include "mainwindow.h"
#include "compass.h"
#include "datasourcemanager.h"
#include "taskmanager.h"
#include "evaluationmanager.h"
#include "dbcontentmanager.h"
#include "radarplotpositioncalculatortask.h"
#include "createartasassociationstask.h"
#include "reconstructortask.h"
#include "viewmanager.h"
#include "eval/results/report/pdfgeneratordialog.h"
#include "viewpointsreportgenerator.h"
#include "viewpointsreportgeneratordialog.h"
#include "evaluationmanager.h"
#include "sectorlayer.h"
#include "logger.h"
#include "util/files.h"
#include "rtcommand_registry.h"
#include "util/config.h"

#include "mainwindow_commands_file.h"
#include "mainwindow_commands_import.h"

#include "event_log.h"

#include <QTimer>
#include <QCoreApplication>
#include <QThread>

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

using namespace std;
using namespace Utils;

//file
REGISTER_RTCOMMAND(main_window::RTCommandOpenDB)
REGISTER_RTCOMMAND(main_window::RTCommandOpenRecentDB)
REGISTER_RTCOMMAND(main_window::RTCommandCreateDB)
REGISTER_RTCOMMAND(main_window::RTCommandCloseDB)
REGISTER_RTCOMMAND(main_window::RTCommandQuit)

//import
REGISTER_RTCOMMAND(main_window::RTCommandImportViewPointsFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXFiles)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXPCAPFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXPCAPFiles)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXNetworkStart)
REGISTER_RTCOMMAND(main_window::RTCommandImportASTERIXNetworkStop)
REGISTER_RTCOMMAND(main_window::RTCommandImportJSONFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportGPSTrail)

REGISTER_RTCOMMAND(main_window::RTCommandImportDataSourcesFile)
REGISTER_RTCOMMAND(main_window::RTCommandImportSectorsJSON)
REGISTER_RTCOMMAND(main_window::RTCommandCalculateRadarPlotPositions)
REGISTER_RTCOMMAND(main_window::RTCommandCalculateARTASAssociations)
REGISTER_RTCOMMAND(main_window::RTCommandReconstructReferences)
REGISTER_RTCOMMAND(main_window::RTCommandLoadData)
REGISTER_RTCOMMAND(main_window::RTCommandExportViewPointsReport)
REGISTER_RTCOMMAND(main_window::RTCommandEvaluate)
REGISTER_RTCOMMAND(main_window::RTCommandExportEvaluationReport)

REGISTER_RTCOMMAND(main_window::RTCommandGetEvents)
REGISTER_RTCOMMAND(main_window::RTCommandReconfigure)
REGISTER_RTCOMMAND(main_window::RTCommandClientInfo)

namespace main_window
{

void init_commands()
{
    // file
    main_window::RTCommandOpenDB::init();
    main_window::RTCommandOpenRecentDB::init();
    main_window::RTCommandCreateDB::init();
    main_window::RTCommandImportDataSourcesFile::init();
    main_window::RTCommandCloseDB::init();
    main_window::RTCommandQuit::init();

    // import
    main_window::RTCommandImportViewPointsFile::init();
    main_window::RTCommandImportASTERIXFile::init();
    main_window::RTCommandImportASTERIXFiles::init();
    main_window::RTCommandImportASTERIXPCAPFile::init();
    main_window::RTCommandImportASTERIXPCAPFiles::init();
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

    main_window::RTCommandGetEvents::init();
    main_window::RTCommandReconfigure::init();
    main_window::RTCommandClientInfo::init();
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
