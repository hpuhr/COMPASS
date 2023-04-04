#ifndef MAINWINDOW_COMMANDS_H
#define MAINWINDOW_COMMANDS_H

#include "rtcommand.h"
#include "rtcommand_macros.h"

namespace main_window
{
extern void init_commands();

// open_db
struct RTCommandOpenDB : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(open_db, "opens existing SQLite3 database with given filename, e.g. ’/data/file1.db’")
    DECLARE_RTCOMMAND_OPTIONS
};

// create_db
struct RTCommandCreateDB : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(create_db, "creates and opens new SQLite3 database with given filename, e.g. ’/data/file1.db’")
    DECLARE_RTCOMMAND_OPTIONS
};

// import_data_sources_file
struct RTCommandImportDataSourcesFile : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() override;

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
    virtual bool run_impl() override;

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
    bool ignore_time_jumps_ {false};

    virtual rtcommand::IsValid valid() const override;

    RTCommandImportASTERIXFile();

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(import_asterix_file, "imports ASTERIX file with given filename, e.g. '/data/file1.ff'")
    DECLARE_RTCOMMAND_OPTIONS
};

// import_asterix_network
struct RTCommandImportASTERIXNetworkStart : public rtcommand::RTCommand
{
    std::string time_offset_str_;
    int max_lines_ {-1};
    bool ignore_future_ts_ {false};

    virtual rtcommand::IsValid valid() const override;

    RTCommandImportASTERIXNetworkStart();

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(import_asterix_network, "imports ASTERIX from defined network UDP streams")
    DECLARE_RTCOMMAND_OPTIONS
};

// import_asterix_network_stop
struct RTCommandImportASTERIXNetworkStop : public rtcommand::RTCommand
{
    RTCommandImportASTERIXNetworkStop();

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(import_asterix_network_stop, "stops import ASTERIX from network")
    DECLARE_RTCOMMAND_NOOPTIONS
};

// import_json
struct RTCommandImportJSONFile : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

    RTCommandImportJSONFile();

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(import_json, "imports JSON file with given filename, e.g. ’/data/file1.json’")
    DECLARE_RTCOMMAND_OPTIONS
};

// import_gps
struct RTCommandImportGPSTrail : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

    RTCommandImportGPSTrail();

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(import_gps_trail, "imports gps trail NMEA with given filename, e.g. ’/data/file2.txt’")
    DECLARE_RTCOMMAND_OPTIONS
};

// import_sectors_json
struct RTCommandImportSectorsJSON : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(import_sectors_json,
                      "imports exported sectors JSON with given filename, e.g. ’/data/sectors.json’")
    DECLARE_RTCOMMAND_OPTIONS
};

// calc radar plot pos
struct RTCommandCalculateRadarPlotPositions : public rtcommand::RTCommand
{
    RTCommandCalculateRadarPlotPositions();

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(calculate_radar_plot_positions , "calculate radar plot positions")
    DECLARE_RTCOMMAND_NOOPTIONS
};

// calc assoc
struct RTCommandCalculateAssociations : public rtcommand::RTCommand
{
    RTCommandCalculateAssociations();

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(associate_data, "associate target reports")
    DECLARE_RTCOMMAND_NOOPTIONS
};

// load data
struct RTCommandLoadData : public rtcommand::RTCommand
{
    RTCommandLoadData();

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(load_data, "load data")
    DECLARE_RTCOMMAND_NOOPTIONS
};

// export vp report
struct RTCommandExportViewPointsReport : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(export_view_points_report,
                      "export view points report after with given filename, e.g. ’/data/db2/report.tex'")
    DECLARE_RTCOMMAND_OPTIONS
};

// evaluate
struct RTCommandEvaluate : public rtcommand::RTCommand
{
    bool run_filter_ {false};

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(evaluate, "run evaluation")
    DECLARE_RTCOMMAND_OPTIONS
};

// export evaluation report
struct RTCommandExportEvaluationReport : public rtcommand::RTCommand
{
    std::string filename_;

    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(export_eval_report,
                      "export evaluation report after start with given filename, e.g. ’/data/eval_db2/report.tex'")
    DECLARE_RTCOMMAND_OPTIONS
};

// close_db
struct RTCommandCloseDB : public rtcommand::RTCommand
{
protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(close_db, "closes a currently opened database")
    DECLARE_RTCOMMAND_NOOPTIONS
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

#endif // MAINWINDOW_COMMANDS_H
