#ifndef MAINWINDOW_COMMANDS_H
#define MAINWINDOW_COMMANDS_H

#include "rtcommand.h"
#include "rtcommand_macros.h"

namespace main_window
{

// open_db
struct RTCommandOpenDB : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(open_db, "opens existing SQLite3 database with given filename, e.g. ’/data/file1.db’")
    DECLARE_RTCOMMAND_OPTIONS
};

// create_db
struct RTCommandCreateDB : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(create_db, "creates and opens new SQLite3 database with given filename, e.g. ’/data/file1.db’")
    DECLARE_RTCOMMAND_OPTIONS
};

// import_data_sources_file
struct RTCommandImportDataSourcesFile : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(import_data_sources, "imports data sources JSON file with given filename, e.g. '/data/ds1.json'")
    DECLARE_RTCOMMAND_OPTIONS
};

// import_view_points
struct RTCommandImportViewPointsFile : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

    RTCommandImportViewPointsFile();

protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(import_view_points, "imports view points JSON file with given filename, e.g. '/data/file1.json'")
    DECLARE_RTCOMMAND_OPTIONS
};

// import_asterix_file
struct RTCommandImportASTERIXFile : public rtcommand::RTCommand
{
    std::string filename_;
    std::string framing_;
    std::string line_id_;
    std::string date_str_;
    std::string time_offset_str_;

    virtual rtcommand::IsValid valid() const override;

    RTCommandImportASTERIXFile();

protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(import_asterix_file, "imports ASTERIX file with given filename, e.g. '/data/file1.ff'")
    DECLARE_RTCOMMAND_OPTIONS
};

// close_db
struct RTCommandCloseDB : public rtcommand::RTCommand
{
protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(close_db, "closes a currently opened database")
    DECLARE_RTCOMMAND_NOOPTIONS
};

// quit
struct RTCommandQuit : public rtcommand::RTCommand
{
protected:
    virtual bool run_impl() const override;

    DECLARE_RTCOMMAND(quit, "closes a currently opened database")
    DECLARE_RTCOMMAND_NOOPTIONS
};


}

#endif // MAINWINDOW_COMMANDS_H

