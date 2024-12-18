#pragma once

#include "rtcommand.h"
#include "rtcommand_macros.h"

namespace main_window
{
extern void init_commands();

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

// calc ARTAS assoc
struct RTCommandCalculateARTASAssociations : public rtcommand::RTCommand
{
    RTCommandCalculateARTASAssociations();

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(calculate_artas_tr_usage, "associate target reports based on ARTAS usage")
    DECLARE_RTCOMMAND_NOOPTIONS
};

// calc ref
struct RTCommandReconstructReferences : public rtcommand::RTCommand
{
    RTCommandReconstructReferences();

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(reconstruct_references, "reconstruct references")
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

// get_events
struct RTCommandGetEvents : public rtcommand::RTCommand
{
    bool         fresh_     = false;
    unsigned int max_items_ = 0;

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(get_events, "retrieves the currently logged events")
    DECLARE_RTCOMMAND_OPTIONS
};

// reconfigure
struct RTCommandReconfigure : public rtcommand::RTCommand
{
    std::string path;
    std::string json_config;

    virtual rtcommand::IsValid valid() const override;

protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(reconfigure, "reconfigures the given configurable")
    DECLARE_RTCOMMAND_OPTIONS
};

// get_events
struct RTCommandClientInfo : public rtcommand::RTCommand
{
protected:
    virtual bool run_impl() override;

    DECLARE_RTCOMMAND(client_info, "retrieves information about the running compass client")
    DECLARE_RTCOMMAND_NOOPTIONS
};

}

