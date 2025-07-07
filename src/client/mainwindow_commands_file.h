#pragma once

#include "rtcommand.h"
#include "rtcommand_macros.h"

namespace main_window
{
// open_db
struct RTCommandOpenDB : public rtcommand::RTCommand
{
    std::string filename_;
    bool        assure_open_ = false;

    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(open_db, "opens existing database file with given filename, e.g. ’/data/file1.db’")
    DECLARE_RTCOMMAND_OPTIONS
};

// open_recent_db
struct RTCommandOpenRecentDB : public rtcommand::RTCommand
{
    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() override;

    std::string getPath() const;

    int         index = -1;
    std::string filename;

    DECLARE_RTCOMMAND(open_recent_db, "opens a database file from the recent file history")
    DECLARE_RTCOMMAND_OPTIONS
};

// create_db
struct RTCommandCreateDB : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(create_db, "creates and opens new database file with given filename, e.g. ’/data/file1.db’")
    DECLARE_RTCOMMAND_OPTIONS
};

// close_db
struct RTCommandCloseDB : public rtcommand::RTCommand
{
    bool strict_ = false;
protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(close_db, "closes a currently opened database")
    DECLARE_RTCOMMAND_OPTIONS
};

// quit
struct RTCommandQuit : public rtcommand::RTCommand
{
protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(quit, "quits the application")
    DECLARE_RTCOMMAND_NOOPTIONS
};
}
