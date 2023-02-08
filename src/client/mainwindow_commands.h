#ifndef MAINWINDOW_COMMANDS_H
#define MAINWINDOW_COMMANDS_H

#include "rtcommand.h"
#include "rtcommand_macros.h"

namespace main_window
{

struct RTCommandOpenDB : public rtcommand::RTCommand
{
    std::string filename_;

    virtual bool valid() const override;

protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(open_db, "opens existing SQLite3 database with given filename, e.g. ’/data/file1.db’")
    DECLARE_RTCOMMAND_OPTIONS
};

struct RTCommandCreateDB : public rtcommand::RTCommand
{
    std::string filename_;

    virtual bool valid() const override;

protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(create_db, "creates and opens new SQLite3 database with given filename, e.g. ’/data/file1.db’")
    DECLARE_RTCOMMAND_OPTIONS
};

struct RTCommandCloseDB : public rtcommand::RTCommand
{
protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(close_db, "closes a currently opened database")
    DECLARE_RTCOMMAND_NOOPTIONS
};

struct RTCommandQuit : public rtcommand::RTCommand
{
protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(quit, "closes a currently opened database")
    DECLARE_RTCOMMAND_NOOPTIONS
};


}

#endif // MAINWINDOW_COMMANDS_H

