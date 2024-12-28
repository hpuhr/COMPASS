#include "mainwindow_commands_import.h"
#include "mainwindow.h"
#include "compass.h"
#include "taskmanager.h"
#include "viewpointsimporttask.h"
#include "asteriximporttask.h"
#include "jsonimporttask.h"
#include "gpstrailimporttask.h"
#include "viewpointsimporttask.h"
#include "logger.h"
#include "util/files.h"
#include "util/stringconv.h"
#include "util/timeconv.h"

#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>

using namespace std;
using namespace Utils;

namespace main_window
{

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

        import_task.settings().ignore_time_jumps_ = ignore_time_jumps_;
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

        import_task.settings().ignore_time_jumps_ = ignore_time_jumps_;
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

// import asterix pcap file

RTCommandImportASTERIXPCAPFile::RTCommandImportASTERIXPCAPFile()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.asteriximporttask.doneSignal", -1); // think about max duration
}

rtcommand::IsValid  RTCommandImportASTERIXPCAPFile::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filename_.size(), "Filename empty")
    CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename_), "File '"+filename_+"' does not exist")

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

bool RTCommandImportASTERIXPCAPFile::run_impl()
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

        import_task.settings().ignore_time_jumps_ = ignore_time_jumps_;
    }
    catch (exception& e)
    {
        logerr << "RTCommandImportASTERIXPCAPFile: run_impl: setting ASTERIX options resulted in error: " << e.what();
        setResultMessage(string("Setting ASTERIX options resulted in error: ")+e.what());
        return false;
    }

    assert (filename_.size());
    assert (Files::fileExists(filename_));

    import_task.source().setSourceType(ASTERIXImportSource::SourceType::FilePCAP, {filename_});//, file_line);

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

void RTCommandImportASTERIXPCAPFile::collectOptions_impl(OptionsDescription& options,
                                                         PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
    ("filename", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.json’");
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

void RTCommandImportASTERIXPCAPFile::assignVariables_impl(const VariablesMap& variables)
    {
        RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
        RTCOMMAND_GET_VAR_OR_THROW(variables, "line", std::string, line_id_)
        RTCOMMAND_GET_VAR_OR_THROW(variables, "date", std::string, date_str_)
        RTCOMMAND_GET_VAR_OR_THROW(variables, "time_offset", std::string, time_offset_str_)
        RTCOMMAND_CHECK_VAR(variables, "ignore_time_jumps", ignore_time_jumps_)
    }

// import asterix files

RTCommandImportASTERIXPCAPFiles::RTCommandImportASTERIXPCAPFiles()
    : rtcommand::RTCommand()
{
    condition.setSignal("compass.taskmanager.asteriximporttask.doneSignal", -1); // think about max duration
}

rtcommand::IsValid RTCommandImportASTERIXPCAPFiles::valid() const
{
    CHECK_RTCOMMAND_INVALID_CONDITION(!filenames_.size(), "Filenames empty")

    for (const auto& filename : split_filenames_)
        CHECK_RTCOMMAND_INVALID_CONDITION(!Files::fileExists(filename), "File '"+filename+"' does not exist")

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

bool RTCommandImportASTERIXPCAPFiles::run_impl()
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

        import_task.settings().ignore_time_jumps_ = ignore_time_jumps_;
    }
    catch (exception& e)
    {
        logerr << "RTCommandImportASTERIXPCAPFiles: run_impl: setting ASTERIX options resulted in error: " << e.what();
        setResultMessage(string("Setting ASTERIX options resulted in error: ")+e.what());
        return false;
    }


    assert (filenames_.size());

    for (const auto& filename : split_filenames_)
    {
        loginf << "RTCommandImportASTERIXPCAPFiles: run_impl: file '" << filename << "'";

        assert (Files::fileExists(filename));
    }

    import_task.source().setSourceType(ASTERIXImportSource::SourceType::FilePCAP, split_filenames_);//, file_line);

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

void RTCommandImportASTERIXPCAPFiles::collectOptions_impl(OptionsDescription& options,
                                                          PosOptionsDescription& positional)
{
    ADD_RTCOMMAND_OPTIONS(options)
    ("filenames", po::value<std::string>()->required(), "given filenames, e.g. '/data/file1.ff;/data/file2.ff'");
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

void RTCommandImportASTERIXPCAPFiles::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filenames", std::string, filenames_)

    for (string filename : String::split(filenames_, ';'))
    {
        if(filename.rfind("~", 0) == 0)  // change tilde to home path
            filename.replace(0, 1, QDir::homePath().toStdString());

        split_filenames_.push_back(filename);
    }

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

}
