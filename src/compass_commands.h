#ifndef COMPASS_COMMANDS_H
#define COMPASS_COMMANDS_H

#include "rtcommand.h"
#include "rtcommand_macros.h"

namespace compass
{

struct RTCommandOpenFileDB : public rtcommand::RTCommand
{
    std::string filename_;

    virtual bool valid() const override;

protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(open_db, "opens existing SQLite3 database with given filename, e.g. ’/data/file1.db’")
    DECLARE_RTCOMMAND_OPTIONS
};

}

#endif // COMPASS_COMMANDS_H

