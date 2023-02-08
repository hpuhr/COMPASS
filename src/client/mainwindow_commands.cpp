#include "mainwindow_commands.h"
#include "mainwindow.h"
#include "compass.h"
#include "logger.h"
#include "util/files.h"
#include "rtcommand_registry.h"

#include <QTimer>

#include <boost/program_options.hpp>

using namespace Utils;

REGISTER_RTCOMMAND(main_window::RTCommandOpenDB)
REGISTER_RTCOMMAND(main_window::RTCommandCreateDB)
REGISTER_RTCOMMAND(main_window::RTCommandCloseDB)
REGISTER_RTCOMMAND(main_window::RTCommandQuit)

namespace main_window
{

// open db

bool RTCommandOpenDB::valid() const
{
    if (!filename_.size())
        return false;

    if (!Files::fileExists(filename_))
        return false;

    return RTCommand::valid();
}

bool RTCommandOpenDB::run_impl() const
{
    if (!filename_.size())
        return false;

    if (!Files::fileExists(filename_))
        return false;

    if (COMPASS::instance().dbOpened())
        return false;

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
        return false;

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

bool RTCommandCreateDB::valid() const
{
    if (!filename_.size())
        return false;

    return RTCommand::valid();
}

bool RTCommandCreateDB::run_impl() const
{
    if (!filename_.size())
        return false;

    if (COMPASS::instance().dbOpened())
        return false;

    if (COMPASS::instance().appMode() != AppMode::Offline) // to be sure
        return false;

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