/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "evaluationmanager.h"
#include "evaluationmanagerwidget.h"
#include "eval/results/report/pdfgeneratordialog.h"
#include "evaluationstandard.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/baseconfig.h"
#include "eval/evaluation_commands.h"
#include "compass.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "datasourcemanager.h"
#include "sectorlayer.h"
#include "sector.h"
#include "airspace.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/variable/variable.h"
#include "buffer.h"
#include "filtermanager.h"
#include "dbfilter.h"
#include "viewabledataconfig.h"
#include "viewmanager.h"
#include "stringconv.h"
#include "util/timeconv.h"
#include "global.h"
#include "viewpoint.h"
#include "projectionmanager.h"
#include "projection.h"

#include "json.hpp"

#include <QTabWidget>
#include <QApplication>
#include <QCoreApplication>
#include <QThread>
#include <QMessageBox>

#include <memory>
#include <fstream>
#include <cstdlib>
#include <system.h>

using namespace Utils;
using namespace std;
using namespace nlohmann;
using namespace boost::posix_time;

EvaluationManagerSettings::EvaluationManagerSettings()
:   line_id_ref_                      (0)
,   active_sources_ref_               ()
,   line_id_tst_                      (0)
,   active_sources_tst_               ()
,   current_standard_                 ("")
,   use_grp_in_sector_                ()
,   use_requirement_                  ()
,   max_ref_time_diff_                (4.0)
,   load_only_sector_data_            (true)
,   use_load_filter_                  (false)
,   use_timestamp_filter_             (false)
,   use_ref_traj_accuracy_filter_     (false)
,   ref_traj_minimum_accuracy_        (30.0f)
,   use_adsb_filter_                  (false)
,   use_v0_                           (true)
,   use_v1_                           (true)
,   use_v2_                           (true)
,   use_min_nucp_                     (true)
,   min_nucp_                         (4u)
,   use_max_nucp_                     (true)
,   max_nucp_                         (4u)
,   use_min_nic_                      (true)
,   min_nic_                          (5u)
,   use_max_nic_                      (true)
,   max_nic_                          (5u)
,   use_min_nacp_                     (true)
,   min_nacp_                         (5u)
,   use_max_nacp_                     (true)
,   max_nacp_                         (5u)
,   use_min_sil_v1_                   (true)
,   min_sil_v1_                       (2u)
,   use_max_sil_v1_                   (true)
,   max_sil_v1_                       (2u)
,   use_min_sil_v2_                   (true)
,   min_sil_v2_                       (4u)
,   use_max_sil_v2_                   (true)
,   max_sil_v2_                       (4u)
,   result_detail_zoom_               (0.02)
,   min_height_filter_layer_          ("")
,   report_skip_no_data_details_      (true)
,   report_split_results_by_mops_     (false)
,   report_split_results_by_aconly_ms_(false)
,   report_author_                    ("")
,   report_abstract_                  ("")
,   report_include_target_details_    (false)
,   report_skip_targets_wo_issues_    (false)
,   report_include_target_tr_details_ (false)
,   show_ok_joined_target_reports_    (false)
,   report_num_max_table_rows_        (1000u)
,   report_num_max_table_col_width_   (18u)
,   report_wait_on_map_loading_       (true)
,   report_run_pdflatex_              (true)
,   report_open_created_pdf_          (false)
,   warning_shown_                    (false)
,   dbcontent_name_ref_               ("RefTraj")
,   dbcontent_name_tst_               ("CAT062")
,   load_timestamp_begin_str_         ("")
,   load_timestamp_end_str_           ("")
{
}

EvaluationManager::EvaluationManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass)
:   Configurable(class_id, instance_id, compass, "eval.json")
,   compass_    (*compass)
,   data_       (*this, compass->dbContentManager())
,   results_gen_(*this, settings_)
,   pdf_gen_    (*this, settings_)
{
    typedef EvaluationManagerSettings Settings;

    registerParameter("dbcontent_name_ref", &settings_.dbcontent_name_ref_, Settings().dbcontent_name_ref_);
    registerParameter("line_id_ref", &settings_.line_id_ref_, Settings().line_id_ref_);
    registerParameter("active_sources_ref", &settings_.active_sources_ref_, Settings().active_sources_ref_);

    data_sources_ref_ = settings_.active_sources_ref_.get<std::map<std::string, std::map<std::string, bool>>>();

     //j.at("foo").get<std::map<std::string, int>>();

    registerParameter("dbcontent_name_tst", &settings_.dbcontent_name_tst_, Settings().dbcontent_name_tst_);
    registerParameter("line_id_tst", &settings_.line_id_tst_, Settings().line_id_tst_);
    registerParameter("active_sources_tst", &settings_.active_sources_tst_, Settings().active_sources_tst_);
    data_sources_tst_ = settings_.active_sources_tst_.get<std::map<std::string, std::map<std::string, bool>>>();

    registerParameter("current_standard", &settings_.current_standard_, Settings().current_standard_);

    registerParameter("use_grp_in_sector", &settings_.use_grp_in_sector_, Settings().use_grp_in_sector_);
    registerParameter("use_requirement", &settings_.use_requirement_, Settings().use_requirement_);

    registerParameter("max_ref_time_diff", &settings_.max_ref_time_diff_, Settings().max_ref_time_diff_);

    // load filter
    registerParameter("use_load_filter", &settings_.use_load_filter_, Settings().use_load_filter_);

    registerParameter("use_timestamp_filter", &settings_.use_timestamp_filter_, Settings().use_timestamp_filter_);
    registerParameter("load_timestamp_begin", &settings_.load_timestamp_begin_str_, Settings().load_timestamp_begin_str_);
    registerParameter("load_timestamp_end", &settings_.load_timestamp_end_str_, Settings().load_timestamp_end_str_);

    if (settings_.load_timestamp_begin_str_.size())
        load_timestamp_begin_ = Time::fromString(settings_.load_timestamp_begin_str_);

    if (settings_.load_timestamp_end_str_.size())
        load_timestamp_end_ = Time::fromString(settings_.load_timestamp_end_str_);

    registerParameter("use_ref_traj_accuracy_filter_", &settings_.use_ref_traj_accuracy_filter_, Settings().use_ref_traj_accuracy_filter_);
    registerParameter("ref_traj_minimum_accuracy", &settings_.ref_traj_minimum_accuracy_, Settings().ref_traj_minimum_accuracy_);

    registerParameter("use_adsb_filter", &settings_.use_adsb_filter_, Settings().use_adsb_filter_);
    registerParameter("use_v0", &settings_.use_v0_, Settings().use_v0_);
    registerParameter("use_v1", &settings_.use_v1_, Settings().use_v1_);
    registerParameter("use_v2", &settings_.use_v2_, Settings().use_v2_);

    // nucp
    registerParameter("use_min_nucp", &settings_.use_min_nucp_, Settings().use_min_nucp_);
    registerParameter("min_nucp", &settings_.min_nucp_, Settings().min_nucp_);

    registerParameter("use_max_nucp", &settings_.use_max_nucp_, Settings().use_max_nucp_);
    registerParameter("max_nucp", &settings_.max_nucp_, Settings().max_nucp_);

    // nic
    registerParameter("use_min_nic", &settings_.use_min_nic_, Settings().use_min_nic_);
    registerParameter("min_nic", &settings_.min_nic_, Settings().min_nic_);

    registerParameter("use_max_nic", &settings_.use_max_nic_, Settings().use_max_nic_);
    registerParameter("max_nic", &settings_.max_nic_, Settings().max_nic_);

    // nacp
    registerParameter("use_min_nacp", &settings_.use_min_nacp_, Settings().use_min_nacp_);
    registerParameter("min_nacp", &settings_.min_nacp_, Settings().min_nacp_);

    registerParameter("use_max_nacp", &settings_.use_max_nacp_, Settings().use_max_nacp_);
    registerParameter("max_nacp", &settings_.max_nacp_, Settings().max_nacp_);

    // sil v1
    registerParameter("use_min_sil_v1", &settings_.use_min_sil_v1_, Settings().use_min_sil_v1_);
    registerParameter("min_sil_v1", &settings_.min_sil_v1_, Settings().min_sil_v1_);

    registerParameter("use_max_sil_v1", &settings_.use_max_sil_v1_, Settings().use_max_sil_v1_);
    registerParameter("max_sil_v1", &settings_.max_sil_v1_, Settings().max_sil_v1_);

    // sil v2
    registerParameter("use_min_sil_v2", &settings_.use_min_sil_v2_, Settings().use_min_sil_v2_);
    registerParameter("min_sil_v2", &settings_.min_sil_v2_, Settings().min_sil_v2_);

    registerParameter("use_max_sil_v2", &settings_.use_max_sil_v2_, Settings().use_max_sil_v2_);
    registerParameter("max_sil_v2", &settings_.max_sil_v2_, Settings().max_sil_v2_);

    registerParameter("result_detail_zoom", &settings_.result_detail_zoom_, Settings().result_detail_zoom_);

    // min height filter
    registerParameter("min_height_filter_layer", &settings_.min_height_filter_layer_, Settings().min_height_filter_layer_);

    // report stuff
    registerParameter("report_skip_no_data_details", &settings_.report_skip_no_data_details_, Settings().report_skip_no_data_details_);
    registerParameter("report_split_results_by_mops", &settings_.report_split_results_by_mops_, Settings().report_split_results_by_mops_);
    registerParameter("report_split_results_by_aconly_ms", &settings_.report_split_results_by_aconly_ms_, Settings().report_split_results_by_aconly_ms_);

    registerParameter("report_author", &settings_.report_author_, Settings().report_author_);

    if (!settings_.report_author_.size())
        settings_.report_author_ = System::getUserName();
    if (!settings_.report_author_.size())
        settings_.report_author_ = "User";

    registerParameter("report_abstract", &settings_.report_abstract_, Settings().report_abstract_);

    registerParameter("report_include_target_details", &settings_.report_include_target_details_, Settings().report_include_target_details_);
    registerParameter("report_skip_targets_wo_issues", &settings_.report_skip_targets_wo_issues_, Settings().report_skip_targets_wo_issues_);
    registerParameter("report_include_target_tr_details", &settings_.report_include_target_tr_details_, Settings().report_include_target_tr_details_);

    registerParameter("show_ok_joined_target_reports", &settings_.show_ok_joined_target_reports_, Settings().show_ok_joined_target_reports_);

    registerParameter("report_num_max_table_rows", &settings_.report_num_max_table_rows_, Settings().report_num_max_table_rows_);
    registerParameter("report_num_max_table_col_width", &settings_.report_num_max_table_col_width_, Settings().report_num_max_table_col_width_);

    registerParameter("report_wait_on_map_loading", &settings_.report_wait_on_map_loading_, Settings().report_wait_on_map_loading_);

    registerParameter("report_run_pdflatex", &settings_.report_run_pdflatex_, Settings().report_run_pdflatex_);

    registerParameter("report_open_created_pdf", &settings_.report_open_created_pdf_, Settings().report_open_created_pdf_);

    bool pdflatex_found = System::exec("which pdflatex").size();

    if (!pdflatex_found)
    {
        settings_.report_run_pdflatex_ = false;
        settings_.report_open_created_pdf_ = false;
    }

    //grid generation
    registerParameter("grid_num_cells_x", &settings_.grid_num_cells_x, Settings().grid_num_cells_x);
    registerParameter("grid_num_cells_y", &settings_.grid_num_cells_y, Settings().grid_num_cells_y);

    //histogram generation
    registerParameter("histogram_num_bins", &settings_.histogram_num_bins, Settings().histogram_num_bins);
    
    registerParameter("warning_shown", &settings_.warning_shown_, Settings().warning_shown_);

    createSubConfigurables();

    init_evaluation_commands();
}

void EvaluationManager::init(QTabWidget* tab_widget)
{
    loginf << "EvaluationManager: init";

    assert (!initialized_);
    assert (tab_widget);

    initialized_ = true;

    if (!COMPASS::instance().hideEvaluation())
        tab_widget->addTab(widget(), "Evaluation");

    widget()->setDisabled(true);

    connect (&COMPASS::instance().dbContentManager(), &DBContentManager::associationStatusChangedSignal,
             this, &EvaluationManager::associationStatusChangedSlot);
}

bool EvaluationManager::canLoadData ()
{
    assert (initialized_);

    return COMPASS::instance().dbContentManager().hasAssociations() && hasCurrentStandard();
}

void EvaluationManager::loadData ()
{
    loginf << "EvaluationManager: loadData";

    assert (initialized_);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // remove previous stuff
    if (viewable_data_cfg_)
    {
        COMPASS::instance().viewManager().unsetCurrentViewPoint();
        viewable_data_cfg_ = nullptr;
    }

    clearLoadedDataAndResults();

    if (widget_)
        widget_->updateButtons();

    emit resultsChangedSignal();

    // actually load

    // load adsb mops versions in a hacky way
    //    if (dbcontent_man.dbContent("CAT021").hasData() && !has_adsb_info_)
    //    {
    //        adsb_info_ = COMPASS::instance().interface().queryADSBInfo();
    //        has_adsb_info_ = true;
    //    }

    QApplication::restoreOverrideCursor();

    FilterManager& fil_man = COMPASS::instance().filterManager();

    // set use filters
    fil_man.useFilters(true);

    // clear data
    data_.clear();

    // set if load for dbos

    std::map<unsigned int, std::set<unsigned int>> ds_ids; // ds_id + line strs

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    std::set<unsigned int> line_ref_set = {settings_.line_id_ref_};

    unsigned int ds_id;

    for (auto& ds_it : data_sources_ref_[settings_.dbcontent_name_ref_])
    {
        ds_id = stoul(ds_it.first);

        loginf << "EvaluationManager: loadData: ref ds_id '" << ds_it.first << "' uint " << ds_id;

        for (auto& line_it : line_ref_set)
               loginf << "EvaluationManager: loadData: ref line " << line_it;

        assert (ds_man.hasDBDataSource(ds_id));

        if (ds_it.second)
            ds_ids.insert(make_pair(ds_id, line_ref_set));
    }

    std::set<unsigned int> line_tst_set = {settings_.line_id_tst_};
    std::set<unsigned int> tst_sources;

    for (auto& ds_it : data_sources_tst_[settings_.dbcontent_name_tst_])
    {
        ds_id = stoul(ds_it.first);

        loginf << "EvaluationManager: loadData: tst ds_id '" << ds_it.first << "' uint " << ds_id;

        for (auto& line_it : line_tst_set)
            loginf << "EvaluationManager: loadData: tst line " << line_it;

        assert (ds_man.hasDBDataSource(ds_id));

        if (ds_it.second)
        {
            if (ds_ids.count(ds_id)) // same ds id
                ds_ids.at(ds_id).insert(line_tst_set.begin(), line_tst_set.end());
            else
                ds_ids.insert(make_pair(ds_id, line_tst_set));

            tst_sources.insert(ds_id);
        }
    }

    updateCompoundCoverage(tst_sources);

    ds_man.setLoadDSTypes(true); // load all ds types
    ds_man.setLoadOnlyDataSources(ds_ids); // limit loaded data sources

    fil_man.disableAllFilters();

    // position data
    if (settings_.load_only_sector_data_ && hasCurrentStandard()
        && sectorsLoaded() && sectorsLayers().size())
    {
        assert (fil_man.hasFilter("Position"));
        DBFilter* pos_fil = fil_man.getFilter("Position");

        json fil_cond;

        bool first = true;
        double lat_min, lat_max, long_min, long_max;
        double tmp_lat_min, tmp_lat_max, tmp_long_min, tmp_long_max;

        EvaluationStandard& standard = currentStandard();

        std::vector<std::shared_ptr<SectorLayer>>& sector_layers = sectorsLayers();
        for (auto& sec_it : sector_layers)
        {
            const string& sector_layer_name = sec_it->name();

            for (auto& req_group_it : standard)
            {
                const string& requirement_group_name = req_group_it->name();

                if (!useGroupInSectorLayer(sector_layer_name, requirement_group_name))
                    continue; // skip if not used

                if (first)
                {
                    tie(lat_min, lat_max) = sec_it->getMinMaxLatitude();
                    tie(long_min, long_max) = sec_it->getMinMaxLongitude();
                    first = false;
                }
                else
                {
                    tie(tmp_lat_min, tmp_lat_max) = sec_it->getMinMaxLatitude();
                    tie(tmp_long_min, tmp_long_max) = sec_it->getMinMaxLongitude();

                    lat_min = min(lat_min, tmp_lat_min);
                    lat_max = max(lat_max, tmp_lat_max);
                    long_min = min(long_min, tmp_long_min);
                    long_max = max(long_max, tmp_long_max);
                }
            }
        }

        //        lat_min = 46.49;
        //        lat_max = 49.16;
        //        long_min = 11.39;
        //        long_max = 17.43;

        if (!first)
        {
            pos_fil->setActive(true);

            latitude_min_ = lat_min-0.2;
            latitude_max_ = lat_max+0.2;
            longitude_min_ = long_min-0.2;
            longitude_max_ = long_max+0.2;
            min_max_pos_set_ = true;

            fil_cond["Position"]["Latitude Maximum"] = to_string(latitude_max_);
            fil_cond["Position"]["Latitude Minimum"] = to_string(latitude_min_);
            fil_cond["Position"]["Longitude Maximum"] = to_string(longitude_max_);
            fil_cond["Position"]["Longitude Minimum"] = to_string(longitude_min_);

            pos_fil->loadViewPointConditions(fil_cond);

        }
        else
        {
            min_max_pos_set_ = false;
        }
    }
    else
        min_max_pos_set_ = false;

    // other filters
    if (settings_.use_load_filter_)
    {
        if (settings_.use_timestamp_filter_)
        {
            assert (fil_man.hasFilter("Timestamp"));
            DBFilter* fil = fil_man.getFilter("Timestamp");

            fil->setActive(true);

            json filter;

            filter["Timestamp"]["Timestamp Minimum"] = Time::toString(load_timestamp_begin_);
            filter["Timestamp"]["Timestamp Maximum"] = Time::toString(load_timestamp_end_);

            fil->loadViewPointConditions(filter);
        }

        if (settings_.use_ref_traj_accuracy_filter_)
        {
            assert (fil_man.hasFilter("RefTraj Accuracy"));
            DBFilter* fil = fil_man.getFilter("RefTraj Accuracy");

            fil->setActive(true);

            json filter;

            filter["RefTraj Accuracy"]["Accuracy Minimum"] = to_string(settings_.ref_traj_minimum_accuracy_);

            fil->loadViewPointConditions(filter);
        }

        if (settings_.use_adsb_filter_)
        {
            assert (fil_man.hasFilter("ADSB Quality"));
            DBFilter* adsb_fil = fil_man.getFilter("ADSB Quality");

            adsb_fil->setActive(true);

            json filter;

            filter["ADSB Quality"]["use_v0"] = settings_.use_v0_;
            filter["ADSB Quality"]["use_v1"] = settings_.use_v1_;
            filter["ADSB Quality"]["use_v2"] = settings_.use_v2_;

            // nucp
            filter["ADSB Quality"]["use_min_nucp"] = settings_.use_min_nucp_;
            filter["ADSB Quality"]["min_nucp"] = settings_.min_nucp_;
            filter["ADSB Quality"]["use_max_nucp"] = settings_.use_max_nucp_;
            filter["ADSB Quality"]["max_nucp"] = settings_.max_nucp_;

            // nic
            filter["ADSB Quality"]["use_min_nic"] = settings_.use_min_nic_;
            filter["ADSB Quality"]["min_nic"] = settings_.min_nic_;
            filter["ADSB Quality"]["use_max_nic"] = settings_.use_max_nic_;
            filter["ADSB Quality"]["max_nic"] = settings_.max_nic_;

            // nacp
            filter["ADSB Quality"]["use_min_nacp"] = settings_.use_min_nacp_;
            filter["ADSB Quality"]["min_nacp"] = settings_.min_nacp_;
            filter["ADSB Quality"]["use_max_nacp"] = settings_.use_max_nacp_;
            filter["ADSB Quality"]["max_nacp"] = settings_.max_nacp_;

            // sil v1
            filter["ADSB Quality"]["use_min_sil_v1"] = settings_.use_min_sil_v1_;
            filter["ADSB Quality"]["min_sil_v1"] = settings_.min_sil_v1_;
            filter["ADSB Quality"]["use_max_sil_v1"] = settings_.use_max_sil_v1_;
            filter["ADSB Quality"]["max_sil_v1"] = settings_.max_sil_v1_;

            // sil v2
            filter["ADSB Quality"]["use_min_sil_v2"] = settings_.use_min_sil_v2_;
            filter["ADSB Quality"]["min_sil_v2"] = settings_.min_sil_v2_;
            filter["ADSB Quality"]["use_max_sil_v2"] = settings_.use_max_sil_v2_;
            filter["ADSB Quality"]["max_sil_v2"] = settings_.max_sil_v2_;

            adsb_fil->loadViewPointConditions(filter);
        }
    }

    COMPASS::instance().viewManager().disableDataDistribution(true);

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    connect(&dbcontent_man, &DBContentManager::loadedDataSignal,
            this, &EvaluationManager::loadedDataDataSlot);
    connect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
            this, &EvaluationManager::loadingDoneSlot);

    needs_additional_variables_ = true;

    dbcontent_man.load();

    needs_additional_variables_ = false;

    if (widget_)
        widget_->updateButtons();
}

bool EvaluationManager::canEvaluate ()
{
    assert (initialized_);

    if (!data_loaded_ || !hasCurrentStandard())
        return false;

    if (!compass_.dbContentManager().hasAssociations())
        return false;

    if (!hasSelectedReferenceDataSources())
        return false;

    if (!hasSelectedTestDataSources())
        return false;

    bool has_anything_to_eval = false;

    if (!sectorsLoaded())
        return false;

    for (auto& sec_it : sectorsLayers())
    {
        const string& sector_layer_name = sec_it->name();

        for (auto& req_group_it : currentStandard())
        {
            const string& requirement_group_name = req_group_it->name();

            if (useGroupInSectorLayer(sector_layer_name, requirement_group_name))
                has_anything_to_eval = true;
        }
    }

    return has_anything_to_eval;
}

std::string EvaluationManager::getCannotEvaluateComment()
{
    assert (!canEvaluate());

    if (!sectors_loaded_)
        return "No Database loaded";

    if (!compass_.dbContentManager().hasAssociations())
        return "Please run target report association";

    // no sector
    if (!sectorsLayers().size())
        return "Please add at least one sector";

    if (!anySectorsWithReq())
        return "Please set requirements for at least one sector";

    if (!hasSelectedReferenceDataSources())
        return "Please select reference data sources";

    if (!hasSelectedTestDataSources())
        return "Please select test data sources";

    if (!data_loaded_)
        return "Please load reference & test data";

    if (!hasCurrentStandard())
        return "Please select a standard";

    // no sector active
    return "Please activate at least one requirement group";
}

void EvaluationManager::databaseOpenedSlot()
{
    loginf << "EvaluationManager: databaseOpenedSlot";

    assert (!sectors_loaded_);
    loadSectors();

    // init with false values if not in cfg
    checkReferenceDataSources();
    checkTestDataSources();
    updateActiveDataSources();

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    if (!dbcont_man.hasAssociations())
        widget()->setDisabled(false);

    if (dbcont_man.hasMinMaxTimestamp())
    {
        std::pair<boost::posix_time::ptime , boost::posix_time::ptime> minmax_ts =  dbcont_man.minMaxTimestamp();
        loadTimestampBegin(get<0>(minmax_ts));
        loadTimestampEnd(get<1>(minmax_ts));

        widget()->updateFilterWidget();
    }

    widget()->updateDataSources();
    widget()->updateSectors();
    widget()->updateButtons();

    emit sectorsChangedSignal();
}

void EvaluationManager::databaseClosedSlot()
{
    loginf << "EvaluationManager: databaseClosedSlot";

    sector_layers_.clear();

    sectors_loaded_ = false;

    data_sources_ref_.clear();
    data_sources_tst_.clear();

    results_gen_.clear();
    data_.clear();

    widget()->updateDataSources();
    widget()->updateSectors();
    widget()->setDisabled(true);

    emit sectorsChangedSignal();

    viewable_data_cfg_ = nullptr;
}

void EvaluationManager::dataSourcesChangedSlot()
{
    checkReferenceDataSources();
    checkTestDataSources();
}

void EvaluationManager::associationStatusChangedSlot()
{
    widget()->setDisabled(!COMPASS::instance().dbContentManager().hasAssociations());
    widget()->updateButtons();
}

void EvaluationManager::loadedDataDataSlot(
        const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
}

void EvaluationManager::loadingDoneSlot()
{
    loginf << "EvaluationManager: loadingDoneSlot";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    disconnect(&dbcontent_man, &DBContentManager::loadedDataSignal,
               this, &EvaluationManager::loadedDataDataSlot);
    disconnect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
               this, &EvaluationManager::loadingDoneSlot);

    COMPASS::instance().viewManager().disableDataDistribution(false);

    loginf << "EvaluationManager: loadingDoneSlot: line ref " << settings_.line_id_ref_
           << " tst " << settings_.line_id_tst_;

    std::map<std::string, std::shared_ptr<Buffer>> data = dbcontent_man.loadedData();
    if (!data.count(settings_.dbcontent_name_ref_))
    {
        QMessageBox m_warning(QMessageBox::Warning, "Loading Data Failed",
                              "No reference data was loaded.",
                              QMessageBox::Ok);
        m_warning.exec();

        if (widget_)
            widget_->updateButtons();

        return;
    }

    data_.setBuffers(data);
    data_.addReferenceData(settings_.dbcontent_name_ref_, settings_.line_id_ref_);
    reference_data_loaded_ = true;

    if (!data.count(settings_.dbcontent_name_tst_))
    {
        QMessageBox m_warning(QMessageBox::Warning, "Loading Data Failed",
                              "No test data was loaded.",
                              QMessageBox::Ok);
        m_warning.exec();

        if (widget_)
            widget_->updateButtons();

        return;
    }
    data_.addTestData(settings_.dbcontent_name_tst_, settings_.line_id_tst_);
    test_data_loaded_ = true;

    dbcontent_man.clearData(); // clear data, has been stored locally

    bool data_loaded_tmp = reference_data_loaded_ && test_data_loaded_;

    loginf << "EvaluationManager: loadingDoneSlot: data loaded " << data_loaded_;

    if (data_loaded_tmp)
    {
        loginf << "EvaluationManager: loadingDoneSlot: finalizing";

        boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

        data_.finalize();

        boost::posix_time::time_duration time_diff =  boost::posix_time::microsec_clock::local_time() - start_time;

        loginf << "EvaluationManager: loadingDoneSlot: finalize done "
                   << String::timeStringFromDouble(time_diff.total_milliseconds() / 1000.0, true);
    }

    data_loaded_ = data_loaded_tmp;

    if (widget_)
        widget_->updateButtons();

}

void EvaluationManager::evaluate()
{
    loginf << "EvaluationManager: evaluate";

    assert (initialized_);
    assert (data_loaded_);
    assert (hasCurrentStandard());

    Projection& projection = ProjectionManager::instance().currentProjection();
    projection.clearCoordinateSystems();
    projection.addAllRadarCoordinateSystems();

    // clean previous
    results_gen_.clear();

    data_.resetModelBegin();
    data_.clearInterestFactors();

    evaluated_ = false;

    if (widget_)
        widget_->updateButtons();

    emit resultsChangedSignal();
    
    // eval
    results_gen_.evaluate(data_, currentStandard());

    data_.resetModelEnd();

    evaluated_ = true;

    if (widget_)
    {
        widget_->updateButtons();
        widget_->expandResults();
        //widget_->showResultId("")
    }

    emit resultsChangedSignal();
}

bool EvaluationManager::canGenerateReport()
{
    assert (initialized_);
    return evaluated_ && hasResults();
}

void EvaluationManager::generateReport()
{
    loginf << "EvaluationManager: generateReport";

    assert (initialized_);
    assert (data_loaded_);
    assert (evaluated_);

    pdf_gen_.dialog().exec();

    if (widget_)
        widget_->updateButtons();
}

void EvaluationManager::close()
{
    initialized_ = false;
}

bool EvaluationManager::needsAdditionalVariables()
{
    return needs_additional_variables_;
}

void EvaluationManager::addVariables (const std::string dbcontent_name, dbContent::VariableSet& read_set)
{
    loginf << "EvaluationManager: addVariables: dbcontent_name " << dbcontent_name;

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    if (!dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_latitude_))
        return;

    // TODO add required variables from standard requirements

    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_));
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ds_id_));
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_));
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_utn_));
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_));

    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_acad_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acad_));

    // flight level
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_));

    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_g_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_g_));

    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_v_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_v_));

    //if (settings_.dbcontent_name_ref_ == dbcontent_name && settings_.dbcontent_name_ref_ == "CAT062")

    // flight level trusted
    if (dbcontent_name == "CAT062")
    {
        read_set.add(dbcontent_man.getVariable("CAT062", DBContent::var_cat062_baro_alt_));
        read_set.add(dbcontent_man.getVariable("CAT062", DBContent::var_cat062_fl_measured_));
    }

    // m3a
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_));

    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_g_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_g_));

    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_v_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_v_));

    // tn
    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_num_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_num_));

    // ground bit
    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ground_bit_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_bit_));

    // speed & track angle
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_));
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_angle_));

    // accs
    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ax_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ax_));
    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ay_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ay_));

    // rocd
    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_rocd_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rocd_));

    // moms
    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mom_long_acc_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mom_long_acc_));
    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mom_trans_acc_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mom_trans_acc_));
    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mom_vert_rate_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mom_vert_rate_));

    if (dbcontent_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_coasting_))
        read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_coasting_));

    //        // for mono sensor + lu sensor

    //        read_set.add(dbcontent.variable("multiple_sources")); // string
    //        read_set.add(dbcontent.variable("track_lu_ds_id")); // int

}

EvaluationManager::~EvaluationManager()
{
    sector_layers_.clear();
}

void EvaluationManager::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "EvaluationStandard")
    {
        EvaluationStandard* standard = new EvaluationStandard(class_id, instance_id, *this);
        logdbg << "EvaluationManager: generateSubConfigurable: adding standard " << standard->name();

        assert(!hasStandard(standard->name()));

        standards_.push_back(std::unique_ptr<EvaluationStandard>(standard));

        // resort by name
        sort(standards_.begin(), standards_.end(),
             [](const std::unique_ptr<EvaluationStandard>& a, const std::unique_ptr<EvaluationStandard>& b) -> bool
        {
            return a->name() > b->name();
        });
    }
    else
        throw std::runtime_error("EvaluationManager: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

EvaluationManagerWidget* EvaluationManager::widget()
{
    if (!widget_)
        widget_.reset(new EvaluationManagerWidget(*this, settings_));

    assert(widget_);
    return widget_.get();
}

void EvaluationManager::checkSubConfigurables()
{
}

bool EvaluationManager::hasSectorLayer(const std::string& layer_name) const
{
    assert (sectors_loaded_);

    auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                             [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});

    return iter != sector_layers_.end();
}

//void EvaluationManager::renameSectorLayer (const std::string& name, const std::string& new_name)
//{
//    // TODO
//}

std::shared_ptr<SectorLayer> EvaluationManager::sectorLayer (const std::string& layer_name) const
{
    assert (sectors_loaded_);
    assert (hasSectorLayer(layer_name));

    auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                             [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});
    assert (iter != sector_layers_.end());

    return *iter;
}

void EvaluationManager::updateMaxSectorID()
{
    for (auto& sec_lay_it : sector_layers_)
    {
        for (auto& sec_it : sec_lay_it->sectors())
            max_sector_id_ = std::max(max_sector_id_, sec_it->id());
    }
}

void EvaluationManager::loadSectors()
{
    loginf << "EvaluationManager: loadSectors";

    assert (!sectors_loaded_);

    if (!COMPASS::instance().interface().ready())
        sectors_loaded_ = false;

    sector_layers_ = COMPASS::instance().interface().loadSectors();

    sectors_loaded_ = true;

    updateMaxSectorID();
    checkMinHeightFilterValid(); // checks if min fl filter sector exists
}

void EvaluationManager::clearLoadedDataAndResults()
{
    loginf << "EvaluationManager: clearLoadedDataAndResults";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    dbcontent_man.clearData(); // clear any previously loaded data

    data_.clear();

    results_gen_.clear();

    reference_data_loaded_ = false;
    test_data_loaded_ = false;
    data_loaded_ = false;

    evaluated_ = false;

    if (widget_)
        widget_->updateButtons();
}

void EvaluationManager::updateSectorLayers()
{
    if (use_fast_sector_inside_check_ && sectors_loaded_)
    {
        for (const auto& layer : sectorsLayers())
            for (const auto& s : layer->sectors())
                s->createFastInsideTest();
    }
}

unsigned int EvaluationManager::getMaxSectorId ()
{
    assert (sectors_loaded_);
    return max_sector_id_;
}

void EvaluationManager::createNewSector (const std::string& name, 
                                         const std::string& layer_name,
                                         bool exclude, 
                                         QColor color, 
                                         std::vector<std::pair<double,double>> points)
{
    loginf << "EvaluationManager: createNewSector: name " << name << " layer_name " << layer_name
           << " num points " << points.size();

    assert (sectors_loaded_);
    assert (!hasSector(name, layer_name));

    ++max_sector_id_; // new max

    shared_ptr<Sector> sector(new Sector(max_sector_id_, name, layer_name, true, exclude, color, points));

    // add to existing sectors
    if (!hasSectorLayer(layer_name))
    {
        sector_layers_.push_back(make_shared<SectorLayer>(layer_name));
    }

    assert (hasSectorLayer(layer_name));

    sectorLayer(layer_name)->addSector(sector);

    assert (hasSector(name, layer_name));
    sector->save();

    if (widget_)
        widget_->updateSectors();

    clearLoadedDataAndResults();

    emit sectorsChangedSignal();
}

bool EvaluationManager::hasSector (const string& name, const string& layer_name)
{
    assert (sectors_loaded_);

    if (!hasSectorLayer(layer_name))
        return false;

    return sectorLayer(layer_name)->hasSector(name);
}

bool EvaluationManager::hasSector (unsigned int id)
{
    assert (sectors_loaded_);

    for (auto& sec_lay_it : sector_layers_)
    {
        auto& sectors = sec_lay_it->sectors();
        auto iter = std::find_if(sectors.begin(), sectors.end(),
                                 [id](const shared_ptr<Sector>& x) { return x->id() == id;});
        if (iter != sectors.end())
            return true;
    }
    return false;
}

std::shared_ptr<Sector> EvaluationManager::sector (const string& name, const string& layer_name)
{
    assert (sectors_loaded_);
    assert (hasSector(name, layer_name));

    return sectorLayer(layer_name)->sector(name);
}

std::shared_ptr<Sector> EvaluationManager::sector (unsigned int id)
{
    assert (sectors_loaded_);
    assert (hasSector(id));

    for (auto& sec_lay_it : sector_layers_)
    {
        auto& sectors = sec_lay_it->sectors();
        auto iter = std::find_if(sectors.begin(), sectors.end(),
                                 [id](const shared_ptr<Sector>& x) { return x->id() == id;});
        if (iter != sectors.end())
            return *iter;
    }

    logerr << "EvaluationManager: sector: id " << id << " not found";
    assert (false);
}

void EvaluationManager::moveSector(unsigned int id, const std::string& old_layer_name, const std::string& new_layer_name)
{
    assert (sectors_loaded_);
    assert (hasSector(id));

    shared_ptr<Sector> tmp_sector = sector(id);

    assert (hasSectorLayer(old_layer_name));
    sectorLayer(old_layer_name)->removeSector(tmp_sector);

    if (!hasSectorLayer(new_layer_name))
        sector_layers_.push_back(make_shared<SectorLayer>(new_layer_name));

    assert (hasSectorLayer(new_layer_name));
    sectorLayer(new_layer_name)->addSector(tmp_sector);

    assert (hasSector(tmp_sector->name(), new_layer_name));
    tmp_sector->save();

    if (widget_)
        widget_->updateSectors();

    clearLoadedDataAndResults();

    emit sectorsChangedSignal();
}

std::vector<std::shared_ptr<SectorLayer>>& EvaluationManager::sectorsLayers()
{
    assert (sectors_loaded_);

    return sector_layers_;
}

void EvaluationManager::saveSector(unsigned int id)
{
    assert (sectors_loaded_);
    assert (hasSector(id));

    saveSector(sector(id));
}

void EvaluationManager::saveSector(std::shared_ptr<Sector> sector)
{
    assert (sectors_loaded_);
    assert (hasSector(sector->name(), sector->layerName()));
    COMPASS::instance().interface().saveSector(sector);
}

void EvaluationManager::deleteSector(shared_ptr<Sector> sector)
{
    assert (sectors_loaded_);
    assert (hasSector(sector->name(), sector->layerName()));

    string layer_name = sector->layerName();

    assert (hasSectorLayer(layer_name));

    sectorLayer(layer_name)->removeSector(sector);

    // remove sector layer if empty
    if (!sectorLayer(layer_name)->size())
    {
        auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                                 [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});

        assert (iter != sector_layers_.end());
        sector_layers_.erase(iter);

        checkMinHeightFilterValid();
    }

    COMPASS::instance().interface().deleteSector(sector);

    if (widget_)
        widget_->updateSectors();

    clearLoadedDataAndResults();

    emit sectorsChangedSignal();
}

void EvaluationManager::deleteAllSectors()
{
    assert (sectors_loaded_);
    sector_layers_.clear();

    COMPASS::instance().interface().deleteAllSectors();

    checkMinHeightFilterValid();

    if (widget_)
        widget_->updateSectors();

    clearLoadedDataAndResults();

    emit sectorsChangedSignal();
}


void EvaluationManager::importSectors(const std::string& filename)
{
    loginf << "EvaluationManager: importSectors: filename '" << filename << "'";

    assert (sectors_loaded_);

    sector_layers_.clear();
    COMPASS::instance().interface().clearSectorsTable();

    std::ifstream input_file(filename, std::ifstream::in);

    try
    {
        json j = json::parse(input_file);

        if (!j.contains("sectors"))
        {
            logerr << "EvaluationManager: importSectors: file does not contain sectors";
            return;
        }

        json& sectors = j["sectors"];

        if (!sectors.is_array())
        {
            logerr << "EvaluationManager: importSectors: file sectors is not an array";
            return;
        }

        unsigned int id;
        string name;
        string layer_name;
        string json_str;

        for (auto& j_sec_it : sectors.get<json::array_t>())
        {
            if (!j_sec_it.contains("id")
                    || !j_sec_it.contains("name")
                    || !j_sec_it.contains("layer_name")
                    || !j_sec_it.contains("points"))
            {
                logerr << "EvaluationManager: importSectors: ill-defined sectors skipped, json '" << j_sec_it.dump(4)
                       << "'";
                continue;
            }

            id = j_sec_it.at("id");
            name = j_sec_it.at("name");
            layer_name = j_sec_it.at("layer_name");

            auto eval_sector = new Sector(id, name, layer_name, true);
            eval_sector->readJSON(j_sec_it.dump());

            if (!hasSectorLayer(layer_name))
                sector_layers_.push_back(make_shared<SectorLayer>(layer_name));

            sectorLayer(layer_name)->addSector(shared_ptr<Sector>(eval_sector));

            assert (hasSector(name, layer_name));

            eval_sector->save();

            loginf << "EvaluationManager: importSectors: loaded sector '" << name << "' in layer '"
                   << layer_name << "' num points " << sector(name, layer_name)->size();
        }
    }
    catch (json::exception& e)
    {
        logerr << "EvaluationManager: importSectors: could not load file '"
               << filename << "'";
        throw e;
    }

    checkMinHeightFilterValid();

    clearLoadedDataAndResults();

    if (widget_)
        widget_->updateSectors();

    emit sectorsChangedSignal();
}

void EvaluationManager::exportSectors (const std::string& filename)
{
    loginf << "EvaluationManager: exportSectors: filename '" << filename << "'";

    assert (sectors_loaded_);

    json j;

    j["sectors"] = json::array();
    json& sectors = j["sectors"];

    unsigned int cnt = 0;

    for (auto& sec_lay_it : sector_layers_)
    {
        for (auto& sec_it : sec_lay_it->sectors())
        {
            sectors[cnt] = sec_it->jsonData();
            ++cnt;
        }
    }

    std::ofstream output_file;
    output_file.open(filename, std::ios_base::out);

    output_file << j.dump(4);
}

bool EvaluationManager::importAirSpace(const AirSpace& air_space,
                                       const boost::optional<std::set<std::string>>& sectors_to_import)
{
    auto layers = air_space.layers();
    if (layers.empty())
        return false;

    std::vector<std::shared_ptr<SectorLayer>> new_layers;

    for (auto l : layers)
    {
        std::vector<std::shared_ptr<Sector>> sectors;
        for (auto s : l->sectors())
        {
            if (sectors_to_import.has_value() && sectors_to_import->find(s->name()) == sectors_to_import->end())
                continue;

            sectors.push_back(s);
        }

        if (!sectors.empty())
        {
            l->clearSectors();
            for (auto s : sectors)
            {
                //serialize from now on
                s->serializeSector(true);

                l->addSector(s);
            }
            new_layers.push_back(l);
        }
    }

    if (new_layers.empty())
        return false;

    sector_layers_.insert(sector_layers_.begin(), new_layers.begin(), new_layers.end());

    for (auto& sec_lay_it : new_layers)
        for (auto& sec_it : sec_lay_it->sectors())
            sec_it->save();


    updateMaxSectorID();

    if (widget_)
        widget_->updateSectors();

    emit sectorsChangedSignal();

    return true;
}

bool EvaluationManager::filterMinimumHeight() const
{
    return !settings_.min_height_filter_layer_.empty();
}   

const std::string& EvaluationManager::minHeightFilterLayerName() const
{
    return settings_.min_height_filter_layer_;
}

void EvaluationManager::minHeightFilterLayerName(const std::string& layer_name)
{
    assert(layer_name.empty() || hasSectorLayer(layer_name));

    loginf << "EvaluationManager: minHeightFilterLayerName: layer changed to "
           << (layer_name.empty() ? "null" : "'" + layer_name + "'");

    settings_.min_height_filter_layer_= layer_name;
}

std::shared_ptr<SectorLayer> EvaluationManager::minHeightFilterLayer() const
{
    if (!filterMinimumHeight())
        return {};

    //!will assert on non-existing layer name!
    return sectorLayer(settings_.min_height_filter_layer_);
}

/**
 * Called every time a layer is removed from the eval manager.
 * Checks if the selected min height filter layer is still present and resets it needed.
 */
void EvaluationManager::checkMinHeightFilterValid()
{
    if (!settings_.min_height_filter_layer_.empty() && !hasSectorLayer(settings_.min_height_filter_layer_))
    {
        logerr << "EvaluationManager: checkMinHeightFilterValid: Layer '" << settings_.min_height_filter_layer_ << "'"
               << " not present, resetting min height filter";
        
        settings_.min_height_filter_layer_ = "";
    }
}

std::string EvaluationManager::dbContentNameRef() const
{
    return settings_.dbcontent_name_ref_;
}

void EvaluationManager::dbContentNameRef(const std::string& name)
{
    loginf << "EvaluationManager: dbContentNameRef: name " << name;

    settings_.dbcontent_name_ref_ = name;

    checkReferenceDataSources();

    widget()->updateButtons();
}

bool EvaluationManager::hasValidReferenceDBContent ()
{
    if (!settings_.dbcontent_name_ref_.size())
        return false;

    return COMPASS::instance().dbContentManager().existsDBContent(settings_.dbcontent_name_ref_);
}

set<unsigned int> EvaluationManager::activeDataSourcesRef()
{
    set<unsigned int> srcs;

    for (auto& ds_it : data_sources_ref_[settings_.dbcontent_name_ref_])
        if (ds_it.second)
            srcs.insert(stoul(ds_it.first));

    return srcs;
}

EvaluationManager::EvaluationDSInfo EvaluationManager::activeDataSourceInfoRef() const
{
    EvaluationDSInfo ds_info;
    ds_info.dbcontent = settings_.dbcontent_name_ref_;

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    for (auto& ds_it : data_sources_ref_.at(settings_.dbcontent_name_ref_))
    {
        if (!ds_it.second)
            continue;

        unsigned int ds_id = stoul(ds_it.first);
        assert (ds_man.hasDBDataSource(ds_id));

        const auto& name = ds_man.dbDataSource(ds_id).name();

        ds_info.data_sources.push_back({ name, ds_id });
    }

    return ds_info;
}

std::string EvaluationManager::dbContentNameTst() const
{
    return settings_.dbcontent_name_tst_;
}

void EvaluationManager::dbContentNameTst(const std::string& name)
{
    loginf << "EvaluationManager: dbContentNameTst: name " << name;

    settings_.dbcontent_name_tst_ = name;

    checkTestDataSources();

    widget()->updateButtons();
}

bool EvaluationManager::hasValidTestDBContent ()
{
    if (!settings_.dbcontent_name_tst_.size())
        return false;

    return COMPASS::instance().dbContentManager().existsDBContent(settings_.dbcontent_name_tst_);
}

set<unsigned int> EvaluationManager::activeDataSourcesTst()
{
    set<unsigned int> srcs;

    for (auto& ds_it : data_sources_tst_[settings_.dbcontent_name_tst_])
        if (ds_it.second)
            srcs.insert(stoul(ds_it.first));

    return srcs;
}

EvaluationManager::EvaluationDSInfo EvaluationManager::activeDataSourceInfoTst() const
{
    EvaluationDSInfo ds_info;
    ds_info.dbcontent = settings_.dbcontent_name_tst_;

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    for (auto& ds_it : data_sources_tst_.at(settings_.dbcontent_name_tst_))
    {
        if (!ds_it.second)
            continue;

        unsigned int ds_id = stoul(ds_it.first);
        assert (ds_man.hasDBDataSource(ds_id));

        const auto& name = ds_man.dbDataSource(ds_id).name();

        ds_info.data_sources.push_back({ name, ds_id });
    }

    return ds_info;
}

bool EvaluationManager::dataLoaded() const
{
    return data_loaded_;
}

bool EvaluationManager::evaluated() const
{
    return evaluated_;
}

EvaluationData& EvaluationManager::getData()
{
    return data_;
}

bool EvaluationManager::hasCurrentStandard()
{
    return settings_.current_standard_.size() && hasStandard(settings_.current_standard_);
}

std::string EvaluationManager::currentStandardName() const
{
    return settings_.current_standard_;
}

void EvaluationManager::currentStandardName(const std::string& current_standard)
{
    settings_.current_standard_ = current_standard;

    if (settings_.current_standard_.size())
        assert (hasStandard(settings_.current_standard_));

    emit currentStandardChangedSignal();

    if (widget_)
        widget_->updateButtons();
}

void EvaluationManager::renameCurrentStandard (const std::string& new_name)
{
    loginf << "EvaluationManager: renameCurrentStandard: new name '" << new_name << "'";

    assert (hasCurrentStandard());
    assert (!hasStandard(new_name));

    currentStandard().name(new_name);
    settings_.current_standard_ = new_name;

    emit standardsChangedSignal();
    emit currentStandardChangedSignal();
}

void EvaluationManager::copyCurrentStandard (const std::string& new_name)
{
    loginf << "EvaluationManager: renameCurrentStandard: new name '" << new_name << "'";

    assert (hasCurrentStandard());
    assert (!hasStandard(new_name));

    nlohmann::json data;
    data["parameters"]["name"] = new_name;

    Configurable::generateSubConfigurableFromJSON(currentStandard(), data, "EvaluationStandard");

    settings_.current_standard_ = new_name;

    emit standardsChangedSignal();
    emit currentStandardChangedSignal();
}

EvaluationStandard& EvaluationManager::currentStandard()
{
    assert (hasCurrentStandard());

    string name = settings_.current_standard_;

    auto iter = std::find_if(standards_.begin(), standards_.end(),
                             [&name](const unique_ptr<EvaluationStandard>& x) { return x->name() == name;});

    assert (iter != standards_.end());

    return *iter->get();
}

bool EvaluationManager::hasStandard(const std::string& name)
{
    auto iter = std::find_if(standards_.begin(), standards_.end(),
                             [&name](const unique_ptr<EvaluationStandard>& x) { return x->name() == name;});

    return iter != standards_.end();
}

void EvaluationManager::addStandard(const std::string& name)
{
    loginf << "EvaluationManager: addStandard: name " << name;

    assert (!hasStandard(name));

    std::string instance = "EvaluationStandard" + name + "0";

    auto config = Configuration::create("EvaluationStandard", instance);
    config->addParameter<std::string>("name", name);

    generateSubConfigurableFromConfig(std::move(config));

    emit standardsChangedSignal();

    currentStandardName(name);
}

void EvaluationManager::deleteCurrentStandard()
{
    loginf << "EvaluationManager: deleteCurrentStandard: name " << settings_.current_standard_;

    assert (hasCurrentStandard());

    string name = settings_.current_standard_;

    auto iter = std::find_if(standards_.begin(), standards_.end(),
                             [&name](const unique_ptr<EvaluationStandard>& x) { return x->name() == name;});

    assert (iter != standards_.end());

    standards_.erase(iter);

    emit standardsChangedSignal();

    currentStandardName("");
}

std::vector<std::string> EvaluationManager::currentRequirementNames()
{
    std::vector<std::string> names;

    if (hasCurrentStandard())
    {
        for (auto& req_grp_it : currentStandard())
        {
            for (auto& req_it : *req_grp_it)
            {
                if (find(names.begin(), names.end(), req_it->name()) == names.end())
                    names.push_back(req_it->name());
            }
        }
    }

    return names;
}

EvaluationResultsGenerator& EvaluationManager::resultsGenerator()
{
    return results_gen_;
}

bool EvaluationManager::sectorsLoaded() const
{
    return sectors_loaded_;
}

bool EvaluationManager::anySectorsWithReq()
{
    if (!sectors_loaded_)
        return false;

    bool any = false;

    if (hasCurrentStandard())
    {
        EvaluationStandard& standard = currentStandard();

        std::vector<std::shared_ptr<SectorLayer>>& sector_layers = sectorsLayers();
        for (auto& sec_it : sector_layers)
        {
            const string& sector_layer_name = sec_it->name();

            for (auto& req_group_it : standard)
            {
                const string& requirement_group_name = req_group_it->name();

                if (useGroupInSectorLayer(sector_layer_name, requirement_group_name))
                {
                    any = true;
                    break;
                }
            }
        }
    }

    return any;
}

void EvaluationManager::checkReferenceDataSources()
{
    loginf << "EvaluationManager: checkReferenceDataSources";

    if (!hasValidReferenceDBContent())
        return;

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    // clear out old ds_ids
    auto ds_copy = data_sources_ref_[settings_.dbcontent_name_ref_];

    unsigned int ds_id;
    for (auto& ds_it : ds_copy)
    {
        ds_id = stoul(ds_it.first);

        if (!ds_man.hasDBDataSource(ds_id))
            data_sources_ref_[settings_.dbcontent_name_ref_].erase(ds_it.first);
    }

    // init non-existing ones with false
    if (ds_man.hasDataSourcesOfDBContent(settings_.dbcontent_name_ref_))
    {
        for (auto& ds_it : ds_man.dbDataSources())
        {
            if (!ds_it->hasNumInserted(settings_.dbcontent_name_ref_))
                continue;

            string ds_id_str = to_string(ds_it->id());

            if (!data_sources_ref_[settings_.dbcontent_name_ref_].count(ds_id_str))
                data_sources_ref_[settings_.dbcontent_name_ref_][ds_id_str] = false; // init with default false
        }
    }
}

void EvaluationManager::checkTestDataSources()
{
    loginf << "EvaluationManager: checkTestDataSources";

    if (!hasValidTestDBContent())
        return;

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    // clear out old ds_ids
    auto ds_copy = data_sources_tst_[settings_.dbcontent_name_tst_];

    unsigned int ds_id;
    for (auto& ds_it : ds_copy)
    {
        ds_id = stoul(ds_it.first);

        if (!ds_man.hasDBDataSource(ds_id))
            data_sources_tst_[settings_.dbcontent_name_tst_].erase(ds_it.first);
    }

    // init non-existing ones with false
    if (ds_man.hasDataSourcesOfDBContent(settings_.dbcontent_name_tst_))
    {
        for (auto& ds_it : ds_man.dbDataSources())
        {
            if (!ds_it->hasNumInserted(settings_.dbcontent_name_tst_))
                continue;

            string ds_id_str = to_string(ds_it->id());

            if (!data_sources_tst_[settings_.dbcontent_name_tst_].count(ds_id_str))
                data_sources_tst_[settings_.dbcontent_name_tst_][ds_id_str] = false; // init with default false
        }
    }
}

bool EvaluationManager::hasSelectedReferenceDataSources()
{
    if (!hasValidReferenceDBContent())
        return false;

    for (auto& ds_it : data_sources_ref_[settings_.dbcontent_name_ref_])
        if (ds_it.second)
            return true;

    return false;
}

bool EvaluationManager::hasSelectedTestDataSources()
{
    if (!hasValidTestDBContent())
        return false;

    for (auto& ds_it : data_sources_tst_[settings_.dbcontent_name_tst_])
        if (ds_it.second)
            return true;

    return false;
}

const dbContent::DataSourceCompoundCoverage& EvaluationManager::tstSrcsCoverage() const
{
    return tst_srcs_coverage_;
}

void EvaluationManager::setViewableDataConfig (const nlohmann::json::object_t& data)
{
    viewable_data_cfg_.reset(new ViewableDataConfig(data));

    COMPASS::instance().viewManager().setCurrentViewPoint(viewable_data_cfg_.get());
}

void EvaluationManager::showUTN (unsigned int utn)
{
    loginf << "EvaluationManager: showUTN: utn " << utn;

    nlohmann::json data = getBaseViewableDataConfig();
    data[ViewPoint::VP_FILTERS_KEY]["UTNs"]["utns"] = to_string(utn);

    loginf << "EvaluationManager: showUTN: showing";
    setViewableDataConfig(data);
}

std::unique_ptr<nlohmann::json::object_t> EvaluationManager::getViewableForUTN (unsigned int utn)
{
    nlohmann::json::object_t data = getBaseViewableDataConfig();
    data[ViewPoint::VP_FILTERS_KEY]["UTNs"]["utns"] = to_string(utn);

    return std::unique_ptr<nlohmann::json::object_t>{new nlohmann::json::object_t(move(data))};
}

std::unique_ptr<nlohmann::json::object_t> EvaluationManager::getViewableForEvaluation (
        const std::string& req_grp_id, const std::string& result_id)
{
    nlohmann::json::object_t data = getBaseViewableNoDataConfig();

    return std::unique_ptr<nlohmann::json::object_t>{new nlohmann::json::object_t(move(data))};
}

std::unique_ptr<nlohmann::json::object_t> EvaluationManager::getViewableForEvaluation (
        unsigned int utn, const std::string& req_grp_id, const std::string& result_id)
{
    nlohmann::json::object_t data = getBaseViewableDataConfig();
    data[ViewPoint::VP_FILTERS_KEY]["UTNs"]["utns"] = to_string(utn);

    return std::unique_ptr<nlohmann::json::object_t>{new nlohmann::json::object_t(move(data))};
}

void EvaluationManager::showResultId (const std::string& id, 
                                      bool select_tab,
                                      bool show_figure)
{
    loginf << "EvaluationManager: showResultId: id '" << id << "'";

    assert (widget_);
    widget_->showResultId(id, select_tab, show_figure);
}

EvaluationManager::ResultIterator EvaluationManager::begin()
{
    return results_gen_.begin();
}
EvaluationManager::ResultIterator EvaluationManager::end()
{
    return results_gen_.end();
}

bool EvaluationManager::hasResults()
{
    return results_gen_.results().size();
}
const std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>&
EvaluationManager::results() const
{
    return results_gen_.results(); }
;

void EvaluationManager::updateResultsToChanges ()
{
    if (evaluated_)
    {
        results_gen_.updateToChanges();

        if (widget_)
        {
            widget_->expandResults();
            widget_->reshowLastResultId();
        }
    }
}

void EvaluationManager::showFullUTN (unsigned int utn)
{
    nlohmann::json::object_t data;
    data[ViewPoint::VP_FILTERS_KEY]["UTNs"]["utns"] = to_string(utn);

    setViewableDataConfig(data);
}

void EvaluationManager::showSurroundingData (unsigned int utn)
{
    nlohmann::json::object_t data;

    assert (data_.hasTargetData(utn));

    const EvaluationTargetData& target_data = data_.targetData(utn);

    ptime time_begin = target_data.timeBegin();
    time_begin -= seconds(60);

    ptime time_end = target_data.timeEnd();
    time_end += seconds(60);

    //    "Timestamp": {
    //    "Timestamp Maximum": "05:56:32.297",
    //    "Timestamp Minimum": "05:44:58.445"
    //    },

    // TODO_TIMESTAMP
    data[ViewPoint::VP_FILTERS_KEY]["Timestamp"]["Timestamp Maximum"] = Time::toString(time_end);
    data[ViewPoint::VP_FILTERS_KEY]["Timestamp"]["Timestamp Minimum"] = Time::toString(time_begin);

    //    "Aircraft Address": {
    //    "Aircraft Address Values": "FEFE10"
    //    },
    if (target_data.acads().size())
        data[ViewPoint::VP_FILTERS_KEY]["Aircraft Address"]["Aircraft Address Values"] = target_data.acadsStr()+",NULL";

    //    "Mode 3/A Code": {
    //    "Mode 3/A Code Values": "7000"
    //    }

    if (target_data.modeACodes().size())
        data[ViewPoint::VP_FILTERS_KEY]["Mode 3/A Codes"]["Mode 3/A Codes Values"] = target_data.modeACodesStr()+",NULL";

    //    VP_FILTERS_KEY: {
    //    "Barometric Altitude": {
    //    "Barometric Altitude Maximum": "43000",
    //    "Barometric Altitude Minimum": "500"
    //    "Barometric Altitude NULL": false
    //    },

    if (target_data.hasModeC())
    {
        float alt_min = target_data.modeCMin();
        alt_min -= 300;
        float alt_max = target_data.modeCMax();
        alt_max += 300;

        data[ViewPoint::VP_FILTERS_KEY]["Barometric Altitude"]["Barometric Altitude Maximum"] = alt_max;
        data[ViewPoint::VP_FILTERS_KEY]["Barometric Altitude"]["Barometric Altitude Minimum"] = alt_min;
        data[ViewPoint::VP_FILTERS_KEY]["Barometric Altitude"]["Barometric Altitude NULL"] = true;
    }

    //    "Position": {
    //    "Latitude Maximum": "50.78493920733",
    //    "Latitude Minimum": "44.31547147615",
    //    "Longitude Maximum": "20.76559892354",
    //    "Longitude Minimum": "8.5801592186"
    //    }

    if (target_data.hasPos())
    {
        double lat_eps = (target_data.latitudeMax() - target_data.latitudeMin()) / 10.0;
        lat_eps = min(lat_eps, 0.1); // 10% or 0.1 at max
        double lon_eps = (target_data.longitudeMax() - target_data.longitudeMin()) / 10.0; // 10%
        lon_eps = min(lon_eps, 0.1); // 10% or 0.1 at max

        data[ViewPoint::VP_FILTERS_KEY]["Position"]["Latitude Maximum"] = to_string(target_data.latitudeMax()+lat_eps);
        data[ViewPoint::VP_FILTERS_KEY]["Position"]["Latitude Minimum"] = to_string(target_data.latitudeMin()-lat_eps);
        data[ViewPoint::VP_FILTERS_KEY]["Position"]["Longitude Maximum"] = to_string(target_data.longitudeMax()+lon_eps);
        data[ViewPoint::VP_FILTERS_KEY]["Position"]["Longitude Minimum"] = to_string(target_data.longitudeMin()-lon_eps);
    }

    setViewableDataConfig(data);
}

json::boolean_t& EvaluationManager::useGroupInSectorLayer(const std::string& sector_layer_name,
                                                          const std::string& group_name)
{
    assert (hasCurrentStandard());

    // standard_name->sector_layer_name->req_grp_name->bool use
    if (!settings_.use_grp_in_sector_.contains(settings_.current_standard_)
            || !settings_.use_grp_in_sector_.at(settings_.current_standard_).contains(sector_layer_name)
            || !settings_.use_grp_in_sector_.at(settings_.current_standard_).at(sector_layer_name).contains(group_name))
        settings_.use_grp_in_sector_[settings_.current_standard_][sector_layer_name][group_name] = true;

    return settings_.use_grp_in_sector_[settings_.current_standard_][sector_layer_name][group_name].get_ref<json::boolean_t&>();
}

void EvaluationManager::useGroupInSectorLayer(const std::string& sector_layer_name,
                                              const std::string& group_name, bool value)
{
    assert (hasCurrentStandard());

    loginf << "EvaluationManager: useGroupInSector: standard_name " << settings_.current_standard_
           << " sector_layer_name " << sector_layer_name << " group_name " << group_name << " value " << value;

    settings_.use_grp_in_sector_[settings_.current_standard_][sector_layer_name][group_name] = value;

    if (widget_)
        widget_->updateButtons();
}

json::boolean_t& EvaluationManager::useRequirement(const std::string& standard_name, const std::string& group_name,
                                                   const std::string& req_name)
{
    // standard_name->req_grp_name->req_grp_name->bool use
    if (!settings_.use_requirement_.contains(standard_name)
            || !settings_.use_requirement_.at(standard_name).contains(group_name)
            || !settings_.use_requirement_.at(standard_name).at(group_name).contains(req_name))
        settings_.use_requirement_[standard_name][group_name][req_name] = true;

    return settings_.use_requirement_[standard_name][group_name][req_name].get_ref<json::boolean_t&>();
}

EvaluationResultsReport::PDFGenerator& EvaluationManager::pdfGenerator()
{
    return pdf_gen_;
}

nlohmann::json::object_t EvaluationManager::getBaseViewableDataConfig ()
{
    nlohmann::json data;

    // set data sources

    std::map<unsigned int, std::set<unsigned int>> data_sources;

    for (auto& src_it : data_sources_ref_[settings_.dbcontent_name_ref_])
        if (src_it.second)
            data_sources[stoul(src_it.first)].insert(settings_.line_id_ref_);

    for (auto& src_it : data_sources_tst_[settings_.dbcontent_name_tst_])
        if (src_it.second)
            data_sources[stoul(src_it.first)].insert(settings_.line_id_tst_);

    data["data_sources"] = data_sources;

    if (settings_.load_only_sector_data_ && min_max_pos_set_)
    {
        data[ViewPoint::VP_FILTERS_KEY]["Position"]["Latitude Maximum"] = to_string(latitude_max_);
        data[ViewPoint::VP_FILTERS_KEY]["Position"]["Latitude Minimum"] = to_string(latitude_min_);
        data[ViewPoint::VP_FILTERS_KEY]["Position"]["Longitude Maximum"] = to_string(longitude_max_);
        data[ViewPoint::VP_FILTERS_KEY]["Position"]["Longitude Minimum"] = to_string(longitude_min_);
    }

    if (settings_.use_load_filter_)
    {
        if (settings_.use_timestamp_filter_)
        {
            data[ViewPoint::VP_FILTERS_KEY]["Timestamp"]["Timestamp Minimum"] = Time::toString(load_timestamp_begin_);
            data[ViewPoint::VP_FILTERS_KEY]["Timestamp"]["Timestamp Maximum"] = Time::toString(load_timestamp_end_);
        }

        if (settings_.use_ref_traj_accuracy_filter_)
        {
            data[ViewPoint::VP_FILTERS_KEY]["RefTraj Accuracy"]["Accuracy Minimum"] = to_string(settings_.ref_traj_minimum_accuracy_);
        }

        if (settings_.use_adsb_filter_)
        {
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_v0"] = settings_.use_v0_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_v1"] = settings_.use_v1_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_v2"] = settings_.use_v2_;

            // nucp
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_min_nucp"] = settings_.use_min_nucp_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["min_nucp"] = settings_.min_nucp_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_max_nucp"] = settings_.use_max_nucp_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["max_nucp"] = settings_.max_nucp_;

            // nic
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_min_nic"] = settings_.use_min_nic_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["min_nic"] = settings_.min_nic_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_max_nic"] = settings_.use_max_nic_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["max_nic"] = settings_.max_nic_;

            // nacp
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_min_nacp"] = settings_.use_min_nacp_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["min_nacp"] = settings_.min_nacp_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_max_nacp"] = settings_.use_max_nacp_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["max_nacp"] = settings_.max_nacp_;

            // sil v1
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_min_sil_v1"] = settings_.use_min_sil_v1_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["min_sil_v1"] = settings_.min_sil_v1_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_max_sil_v1"] = settings_.use_max_sil_v1_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["max_sil_v1"] = settings_.max_sil_v1_;

            // sil v2
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_min_sil_v2"] = settings_.use_min_sil_v2_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["min_sil_v2"] = settings_.min_sil_v2_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["use_max_sil_v2"] = settings_.use_max_sil_v2_;
            data[ViewPoint::VP_FILTERS_KEY]["ADSB Quality"]["max_sil_v2"] = settings_.max_sil_v2_;
        }
    }

    return data;
}

nlohmann::json::object_t EvaluationManager::getBaseViewableNoDataConfig ()
{
    nlohmann::json data;

    data["data_sources"] = std::map<unsigned int, std::set<unsigned int>>{};

    return data;
}

boost::posix_time::ptime EvaluationManager::loadTimestampBegin() const
{
    return load_timestamp_begin_;
}

void EvaluationManager::loadTimestampBegin(boost::posix_time::ptime value)
{
    loginf << "EvaluationManager: loadTimeBegin: value " << Time::toString(value);

    load_timestamp_begin_ = value;
    settings_.load_timestamp_begin_str_ = Time::toString(load_timestamp_begin_);
}

boost::posix_time::ptime EvaluationManager::loadTimestampEnd() const
{
    return load_timestamp_end_;
}

void EvaluationManager::loadTimestampEnd(boost::posix_time::ptime value)
{
    loginf << "EvaluationManager: loadTimeEnd: value " << Time::toString(value);

    load_timestamp_end_ = value;
    settings_.load_timestamp_end_str_ = Time::toString(load_timestamp_end_);
}

void EvaluationManager::updateActiveDataSources() // save to config var
{
    settings_.active_sources_ref_ = data_sources_ref_;
    settings_.active_sources_tst_ = data_sources_tst_;

    widget_->updateButtons();
}

void EvaluationManager::updateCompoundCoverage(std::set<unsigned int> tst_sources)
{
    loginf << "EvaluationManager: updateCompoundCoverage";

    tst_srcs_coverage_.clear();

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    for (auto ds_id : tst_sources)
    {
        assert (ds_man.hasDBDataSource(ds_id));

        dbContent::DBDataSource& ds = ds_man.dbDataSource(ds_id);

        if (ds.hasRadarRanges())
        {
            bool range_max_set = false;
            double range_max = 0;

            for (auto range_it : ds.radarRanges())
            {
                if (range_max_set)
                    range_max = max(range_max, range_it.second);
                else
                {
                    range_max = range_it.second;
                    range_max_set = true;
                }
            }

            if (range_max_set && ds.hasPosition())
            {
                loginf << "EvaluationManager: updateCompoundCoverage: adding src " << ds.name()
                       << " range " << range_max * NM2M;

                tst_srcs_coverage_.addRangeCircle(ds_id, ds.latitude(), ds.longitude(), range_max * NM2M);
            }
        }
    }

    tst_srcs_coverage_.finalize();
}

/**
*/
void EvaluationManager::onConfigurationChanged(const std::vector<std::string>& changed_params)
{
    assert(widget_);

    widget_->updateFromSettings();
}
