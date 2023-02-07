#include "compass_commands.h"
#include "compass.h"
#include "logger.h"
#include "util/files.h"
#include "rtcommand_registry.h"

#include <boost/program_options.hpp>

using namespace Utils;


REGISTER_RTCOMMAND(compass::RTCommandOpenFileDB)

namespace compass
{

bool RTCommandOpenFileDB::valid() const
{
    if (!filename_.size())
        return false;

    if (!Files::fileExists(filename_))
        return false;

    return RTCommand::valid();
}

bool RTCommandOpenFileDB::run_impl() const
{
    if (!filename_.size())
        return false;

    if (!Files::fileExists(filename_))
        return false;

    if (COMPASS::instance().dbOpened())
        return false;

    COMPASS::instance().openDBFile(filename_);

    return COMPASS::instance().dbOpened();
}

/**
 */
void RTCommandOpenFileDB::collectOptions_impl(OptionsDescription& options)
{
    ADD_RTCOMMAND_OPTIONS(options)
        ("filename,f", po::value<std::string>()->required(), "given filename, e.g. ’/data/file1.db’");
}

/**
 */
void RTCommandOpenFileDB::assignVariables_impl(const VariablesMap& variables)
{
    RTCOMMAND_GET_VAR_OR_THROW(variables, "filename", std::string, filename_)
}

}
