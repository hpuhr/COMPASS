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
#include "evaluationdatawidget.h"
#include "evaluationstandard.h"
#include "evaluationstandardwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/baseconfig.h"
#include "compass.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "datasourcesloadwidget.h"
#include "datasourcemanager.h"
#include "evaluationsector.h"
#include "sectorlayer.h"
#include "airspace.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/target/target.h"
#include "buffer.h"
#include "filtermanager.h"
#include "dbfilter.h"
#include "dbfilterwidget.h"
#include "viewabledataconfig.h"
#include "viewmanager.h"
#include "stringconv.h"
#include "dbcontent/variable/variableorderedset.h"
#include "sqliteconnection.h"
#include "stringconv.h"
#include "util/timeconv.h"

#include "json.hpp"

#include <QTabWidget>
#include <QApplication>
#include <QCoreApplication>
#include <QThread>
#include <QMessageBox>

#include "boost/date_time/posix_time/posix_time.hpp"

#include <memory>
#include <fstream>
#include <cstdlib>
#include <system.h>

using namespace Utils;
using namespace std;
using namespace nlohmann;
using namespace boost::posix_time;

const std::string EvaluationManager::AirSpaceLayerName = "air_space_sectors";

EvaluationManager::EvaluationManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass)
:   Configurable(class_id, instance_id, compass, "eval.json")
,   compass_    (*compass)
,   air_space_  (new AirSpace)
,   data_       (*this, compass->dbContentManager())
,   results_gen_(*this)
,   pdf_gen_    (*this)
{
    registerParameter("dbcontent_name_ref", &dbcontent_name_ref_, "RefTraj");
    registerParameter("line_id_ref", &line_id_ref_, 0);
    registerParameter("active_sources_ref", &active_sources_ref_, json::object());

    data_sources_ref_ = active_sources_ref_.get<std::map<std::string, std::map<std::string, bool>>>();

     //j.at("foo").get<std::map<std::string, int>>();

    registerParameter("dbcontent_name_tst", &dbcontent_name_tst_, "CAT062");
    registerParameter("line_id_tst", &line_id_tst_, 0);
    registerParameter("active_sources_tst", &active_sources_tst_, json::object());
    data_sources_tst_ = active_sources_tst_.get<std::map<std::string, std::map<std::string, bool>>>();

    registerParameter("current_standard", &current_standard_, "");

    registerParameter("use_grp_in_sector", &use_grp_in_sector_, json::object());
    registerParameter("use_requirement", &use_requirement_, json::object());

    registerParameter("max_ref_time_diff", &max_ref_time_diff_, 4.0);

    // load filter
    registerParameter("use_load_filter", &use_load_filter_, false);

    registerParameter("use_timestamp_filter", &use_timestamp_filter_, false);
    registerParameter("load_timestamp_begin", &load_timestamp_begin_str_, "");
    registerParameter("load_timestamp_end", &load_timestamp_end_str_, "");

    if (load_timestamp_begin_str_.size())
        load_timestamp_begin_ = Time::fromString(load_timestamp_begin_str_);

    if (load_timestamp_end_str_.size())
        load_timestamp_end_ = Time::fromString(load_timestamp_end_str_);

    registerParameter("use_adsb_filter", &use_adsb_filter_, false);
    registerParameter("use_v0", &use_v0_, true);
    registerParameter("use_v1", &use_v1_, true);
    registerParameter("use_v2", &use_v2_, true);

    // nucp
    registerParameter("use_min_nucp", &use_min_nucp_, true);
    registerParameter("min_nucp", &min_nucp_, 4);

    registerParameter("use_max_nucp", &use_max_nucp_, true);
    registerParameter("max_nucp", &max_nucp_, 4);

    // nic
    registerParameter("use_min_nic", &use_min_nic_, true);
    registerParameter("min_nic", &min_nic_, 5);

    registerParameter("use_max_nic", &use_max_nic_, true);
    registerParameter("max_nic", &max_nic_, 5);

    // nacp
    registerParameter("use_min_nacp", &use_min_nacp_, true);
    registerParameter("min_nacp", &min_nacp_, 5);

    registerParameter("use_max_nacp", &use_max_nacp_, true);
    registerParameter("max_nacp", &max_nacp_, 5);

    // sil v1
    registerParameter("use_min_sil_v1", &use_min_sil_v1_, true);
    registerParameter("min_sil_v1", &min_sil_v1_, 2);

    registerParameter("use_max_sil_v1", &use_max_sil_v1_, true);
    registerParameter("max_sil_v1", &max_sil_v1_, 2);

    // sil v2
    registerParameter("use_min_sil_v2", &use_min_sil_v2_, true);
    registerParameter("min_sil_v2", &min_sil_v2_, 4);

    registerParameter("use_max_sil_v2", &use_max_sil_v2_, true);
    registerParameter("max_sil_v2", &max_sil_v2_, 4);

    registerParameter("result_detail_zoom", &result_detail_zoom_, 0.02);

    // report stuff

    registerParameter("report_skip_no_data_details", &report_skip_no_data_details_, true);
    registerParameter("report_split_results_by_mops", &report_split_results_by_mops_, false);
    registerParameter("report_show_adsb_info", &report_show_adsb_info_, false);

    registerParameter("report_author", &report_author_, "");

    if (!report_author_.size())
        report_author_ = System::getUserName();
    if (!report_author_.size())
        report_author_ = "User";

    registerParameter("report_abstract", &report_abstract_, "");

    registerParameter("report_include_target_details", &report_include_target_details_, false);
    registerParameter("report_skip_targets_wo_issues", &report_skip_targets_wo_issues_, false);
    registerParameter("report_include_target_tr_details", &report_include_target_tr_details_, false);

    registerParameter("report_num_max_table_rows", &report_num_max_table_rows_, 1000);
    registerParameter("report_num_max_table_col_width", &report_num_max_table_col_width_, 18);

    registerParameter("report_wait_on_map_loading", &report_wait_on_map_loading_, true);

    registerParameter("report_run_pdflatex", &report_run_pdflatex_, true);

    registerParameter("report_open_created_pdf", &report_open_created_pdf_, false);

    bool pdflatex_found = System::exec("which pdflatex").size();

    if (!pdflatex_found)
    {
        report_run_pdflatex_ = false;
        report_open_created_pdf_ = false;
    }

    registerParameter("warning_shown", &warning_shown_, false);

    createSubConfigurables();
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

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    dbcontent_man.clearData(); // clear any previously loaded data

    results_gen_.clear();

    reference_data_loaded_ = false;
    test_data_loaded_ = false;
    data_loaded_ = false;

    evaluated_ = false;

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

    std::set<unsigned int> line_ref_set = {line_id_ref_};

    unsigned int ds_id;

    for (auto& ds_it : data_sources_ref_[dbcontent_name_ref_])
    {
        ds_id = stoul(ds_it.first);

        loginf << "EvaluationManager: loadData: ref ds_id '" << ds_it.first << "' uint " << ds_id;

        for (auto& line_it : line_ref_set)
               loginf << " line " << line_it;

        assert (ds_man.hasDBDataSource(ds_id));

        if (ds_it.second)
            ds_ids.insert(make_pair(ds_id, line_ref_set));
    }

    std::set<unsigned int> line_tst_set = {line_id_tst_};

    for (auto& ds_it : data_sources_tst_[dbcontent_name_tst_])
    {
        ds_id = stoul(ds_it.first);

        loginf << "EvaluationManager: loadData: tst ds_id '" << ds_it.first << "' uint " << ds_id;

        for (auto& line_it : line_tst_set)
            loginf << " line " << line_it;

        assert (ds_man.hasDBDataSource(ds_id));

        if (ds_it.second)
        {
            if (ds_ids.count(ds_id)) // same ds id
                ds_ids.at(ds_id).insert(line_tst_set.begin(), line_tst_set.end());
            else
                ds_ids.insert(make_pair(ds_id, line_tst_set));
        }
    }

    ds_man.setLoadDSTypes(true); // load all ds types
    ds_man.setLoadOnlyDataSources(ds_ids); // limit loaded data sources

    fil_man.disableAllFilters();

    // position data
    if (load_only_sector_data_ && hasCurrentStandard() && sectorsLayers().size())
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
    if (use_load_filter_)
    {
        if (use_timestamp_filter_)
        {
            assert (fil_man.hasFilter("Timestamp"));
            DBFilter* fil = fil_man.getFilter("Timestamp");

            fil->setActive(true);

            json filter;

            filter["Timestamp"]["Timestamp Minimum"] = Time::toString(load_timestamp_begin_);
            filter["Timestamp"]["Timestamp Maximum"] = Time::toString(load_timestamp_end_);

            fil->loadViewPointConditions(filter);
        }


        if (use_adsb_filter_)
        {
            assert (fil_man.hasFilter("ADSB Quality"));
            DBFilter* adsb_fil = fil_man.getFilter("ADSB Quality");

            adsb_fil->setActive(true);

            json filter;

            filter["ADSB Quality"]["use_v0"] = use_v0_;
            filter["ADSB Quality"]["use_v1"] = use_v1_;
            filter["ADSB Quality"]["use_v2"] = use_v2_;

            // nucp
            filter["ADSB Quality"]["use_min_nucp"] = use_min_nucp_;
            filter["ADSB Quality"]["min_nucp"] = min_nucp_;
            filter["ADSB Quality"]["use_max_nucp"] = use_max_nucp_;
            filter["ADSB Quality"]["max_nucp"] = max_nucp_;

            // nic
            filter["ADSB Quality"]["use_min_nic"] = use_min_nic_;
            filter["ADSB Quality"]["min_nic"] = min_nic_;
            filter["ADSB Quality"]["use_max_nic"] = use_max_nic_;
            filter["ADSB Quality"]["max_nic"] = max_nic_;

            // nacp
            filter["ADSB Quality"]["use_min_nacp"] = use_min_nacp_;
            filter["ADSB Quality"]["min_nacp"] = min_nacp_;
            filter["ADSB Quality"]["use_max_nacp"] = use_max_nacp_;
            filter["ADSB Quality"]["max_nacp"] = max_nacp_;

            // sil v1
            filter["ADSB Quality"]["use_min_sil_v1"] = use_min_sil_v1_;
            filter["ADSB Quality"]["min_sil_v1"] = min_sil_v1_;
            filter["ADSB Quality"]["use_max_sil_v1"] = use_max_sil_v1_;
            filter["ADSB Quality"]["max_sil_v1"] = max_sil_v1_;

            // sil v2
            filter["ADSB Quality"]["use_min_sil_v2"] = use_min_sil_v2_;
            filter["ADSB Quality"]["min_sil_v2"] = min_sil_v2_;
            filter["ADSB Quality"]["use_max_sil_v2"] = use_max_sil_v2_;
            filter["ADSB Quality"]["max_sil_v2"] = max_sil_v2_;

            adsb_fil->loadViewPointConditions(filter);
        }
    }

    COMPASS::instance().viewManager().disableDataDistribution(true);

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

    widget()->updateDataSources();
    widget()->updateSectors();
    widget()->setDisabled(true);

    emit sectorsChangedSignal();
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

    loginf << "EvaluationManager: loadingDoneSlot: line ref " << line_id_ref_ << " tst " << line_id_tst_;

    std::map<std::string, std::shared_ptr<Buffer>> data = dbcontent_man.loadedData();
    if (!data.count(dbcontent_name_ref_))
    {
        QMessageBox m_warning(QMessageBox::Warning, "Loading Data Failed",
                              "No reference data was loaded.",
                              QMessageBox::Ok);
        m_warning.exec();

        if (widget_)
            widget_->updateButtons();

        return;
    }

    data_.addReferenceData(dbcontent_man.dbContent(dbcontent_name_ref_), line_id_ref_, data.at(dbcontent_name_ref_));
    reference_data_loaded_ = true;

    if (!data.count(dbcontent_name_tst_))
    {
        QMessageBox m_warning(QMessageBox::Warning, "Loading Data Failed",
                              "No test data was loaded.",
                              QMessageBox::Ok);
        m_warning.exec();

        if (widget_)
            widget_->updateButtons();

        return;
    }
    data_.addTestData(dbcontent_man.dbContent(dbcontent_name_tst_), line_id_tst_, data.at(dbcontent_name_tst_));
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

void EvaluationManager::evaluate ()
{
    loginf << "EvaluationManager: evaluate";

    assert (initialized_);
    assert (data_loaded_);
    assert (hasCurrentStandard());

    // clean previous
    results_gen_.clear();

    evaluated_ = false;

    if (widget_)
        widget_->updateButtons();

    emit resultsChangedSignal();
    
    // eval
    results_gen_.evaluate(data_, currentStandard());

    evaluated_ = true;

    if (widget_)
    {
        widget_->updateButtons();
        widget_->expandResults();
        //widget_->showResultId("")
    }

    emit resultsChangedSignal();
}

bool EvaluationManager::canGenerateReport ()
{
    assert (initialized_);
    return evaluated_ && hasResults();
}

void EvaluationManager::generateReport ()
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

bool EvaluationManager::needsAdditionalVariables ()
{
    return needs_additional_variables_;
}

void EvaluationManager::addVariables (const std::string dbcontent_name, dbContent::VariableSet& read_set)
{
    loginf << "EvaluationManager: addVariables: dbcontent_name " << dbcontent_name;

    // TODO add required variables from standard requirements

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name));
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_datasource_id_.name()).getFor(dbcontent_name));
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_line_id_.name()).getFor(dbcontent_name));
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_associations_.name()).getFor(dbcontent_name));
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_timestamp_.name()).getFor(dbcontent_name));
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_latitude_.name()).getFor(dbcontent_name));
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_longitude_.name()).getFor(dbcontent_name));

    if (dbcontent_man.metaVariable(DBContent::meta_var_ta_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_ta_.name()).getFor(dbcontent_name));

    // flight level
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_mc_.name()).getFor(dbcontent_name));

    if (dbcontent_man.metaVariable(DBContent::meta_var_mc_g_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_mc_g_.name()).getFor(dbcontent_name));

    if (dbcontent_man.metaVariable(DBContent::meta_var_mc_v_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_mc_v_.name()).getFor(dbcontent_name));

    if (dbcontent_name_ref_ == dbcontent_name && dbcontent_name_ref_ == "CAT062")
        read_set.add(dbcontent_man.dbContent("CAT062").variable(DBContent::var_cat062_baro_alt_.name()));

    // flight level trusted
    if (dbcontent_name == "CAT062")
        read_set.add(dbcontent_man.dbContent("CAT062").variable(DBContent::var_cat062_fl_measured_.name()));

    // m3a
    read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_m3a_.name()).getFor(dbcontent_name));

    if (dbcontent_man.metaVariable(DBContent::meta_var_m3a_g_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_m3a_g_.name()).getFor(dbcontent_name));

    if (dbcontent_man.metaVariable(DBContent::meta_var_m3a_v_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_m3a_v_.name()).getFor(dbcontent_name));

    // tn
    if (dbcontent_man.metaVariable(DBContent::meta_var_track_num_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_track_num_.name()).getFor(dbcontent_name));

    // ground bit
    if (dbcontent_man.metaVariable(DBContent::meta_var_ground_bit_.name()).existsIn(dbcontent_name))
        read_set.add(dbcontent_man.metaVariable(DBContent::meta_var_ground_bit_.name()).getFor(dbcontent_name));

    //    if (dbcontent_name == "ADSB")
    //    {
    //        read_set.add(dbcontent.variable("ground_bit"));
    //        //            read_set.add(obj.variable("nac_p"));
    //        //            read_set.add(obj.variable("nucp_nic"));
    //        //            read_set.add(obj.variable("sil"));
    //    }

    // speed & track angle
  
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_));
    read_set.add(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_angle_));

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
        widget_.reset(new EvaluationManagerWidget(*this));

    assert(widget_);
    return widget_.get();
}

void EvaluationManager::checkSubConfigurables()
{
}

bool EvaluationManager::hasSectorLayer (const std::string& layer_name)
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

std::shared_ptr<SectorLayer> EvaluationManager::sectorLayer (const std::string& layer_name)
{
    assert (sectors_loaded_);
    assert (hasSectorLayer(layer_name));

    auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                             [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});
    assert (iter != sector_layers_.end());

    return *iter;
}

void EvaluationManager::loadSectors()
{
    loginf << "EvaluationManager: loadSectors";

    assert (!sectors_loaded_);

    if (!COMPASS::instance().interface().ready())
        sectors_loaded_ = false;

    sector_layers_ = COMPASS::instance().interface().loadSectors();

    for (auto& sec_lay_it : sector_layers_)
    {
        for (auto& sec_it : sec_lay_it->sectors())
            max_sector_id_ = std::max(max_sector_id_, sec_it->id());
    }

    sectors_loaded_ = true;
}

void EvaluationManager::updateSectorLayers()
{
    if (use_fast_sector_inside_check_)
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

    shared_ptr<Sector> sector(new EvaluationSector(max_sector_id_, name, layer_name, exclude, color, points));

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
    }

    COMPASS::instance().interface().deleteSector(sector);

    if (widget_)
        widget_->updateSectors();

    emit sectorsChangedSignal();
}

void EvaluationManager::deleteAllSectors()
{
    assert (sectors_loaded_);
    sector_layers_.clear();

    COMPASS::instance().interface().deleteAllSectors();

    if (widget_)
        widget_->updateSectors();

    emit sectorsChangedSignal();
}


void EvaluationManager::importSectors (const std::string& filename)
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

            auto eval_sector = new EvaluationSector(id, name, layer_name);
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

bool EvaluationManager::importAirSpace(const std::string& filename)
{
    assert(air_space_);
    bool ok = air_space_->readJSON(filename);

    if (widget_)
        widget_->updateSectors();

    emit airSpaceChangedSignal();

    return ok;
}

const SectorLayer* EvaluationManager::airSpaceSectors() const
{
    assert(air_space_);
    return air_space_->layer();
}

void EvaluationManager::clearAirSpace()
{
    assert(air_space_);
    air_space_->clear();
}

std::string EvaluationManager::dbContentNameRef() const
{
    return dbcontent_name_ref_;
}

void EvaluationManager::dbContentNameRef(const std::string& name)
{
    loginf << "EvaluationManager: dbContentNameRef: name " << name;

    dbcontent_name_ref_ = name;

    checkReferenceDataSources();

    widget()->updateButtons();
}

bool EvaluationManager::hasValidReferenceDBContent ()
{
    if (!dbcontent_name_ref_.size())
        return false;

    return COMPASS::instance().dbContentManager().existsDBContent(dbcontent_name_ref_);
}

set<unsigned int> EvaluationManager::activeDataSourcesRef()
{
    set<unsigned int> srcs;

    for (auto& ds_it : data_sources_ref_[dbcontent_name_ref_])
        if (ds_it.second)
            srcs.insert(stoul(ds_it.first));

    return srcs;

}

std::string EvaluationManager::dbContentNameTst() const
{
    return dbcontent_name_tst_;
}

void EvaluationManager::dbContentNameTst(const std::string& name)
{
    loginf << "EvaluationManager: dbContentNameTst: name " << name;

    dbcontent_name_tst_ = name;

    checkTestDataSources();

    widget()->updateButtons();
}

bool EvaluationManager::hasValidTestDBContent ()
{
    if (!dbcontent_name_tst_.size())
        return false;

    return COMPASS::instance().dbContentManager().existsDBContent(dbcontent_name_tst_);
}

set<unsigned int> EvaluationManager::activeDataSourcesTst()
{
    set<unsigned int> srcs;

    for (auto& ds_it : data_sources_tst_[dbcontent_name_tst_])
        if (ds_it.second)
            srcs.insert(stoul(ds_it.first));

    return srcs;

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
    return current_standard_.size() && hasStandard(current_standard_);
}

std::string EvaluationManager::currentStandardName() const
{
    return current_standard_;
}

void EvaluationManager::currentStandardName(const std::string& current_standard)
{
    current_standard_ = current_standard;

    if (current_standard_.size())
        assert (hasStandard(current_standard_));

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
    current_standard_ = new_name;

    emit standardsChangedSignal();
    emit currentStandardChangedSignal();
}

void EvaluationManager::copyCurrentStandard (const std::string& new_name)
{
    loginf << "EvaluationManager: renameCurrentStandard: new name '" << new_name << "'";

    assert (hasCurrentStandard());
    assert (!hasStandard(new_name));

    //Configuration new_config = currentStandard().configuration();

    nlohmann::json current_json_cfg;
    currentStandard().configuration().generateJSON(current_json_cfg);
    current_json_cfg["parameters"]["name"] = new_name;

    Configuration& config = addNewSubConfiguration("EvaluationStandard");
    config.parseJSONConfig(current_json_cfg);
    //config.addParameterString("name", new_name);
    generateSubConfigurable("EvaluationStandard", config.getInstanceId());

    current_standard_ = new_name;

    emit standardsChangedSignal();
    emit currentStandardChangedSignal();
}

EvaluationStandard& EvaluationManager::currentStandard()
{
    assert (hasCurrentStandard());

    string name = current_standard_;

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

    Configuration& config = addNewSubConfiguration("EvaluationStandard", instance);
    config.addParameterString("name", name);

    generateSubConfigurable("EvaluationStandard", instance);

    emit standardsChangedSignal();

    currentStandardName(name);
}

void EvaluationManager::deleteCurrentStandard()
{
    loginf << "EvaluationManager: deleteCurrentStandard: name " << current_standard_;

    assert (hasCurrentStandard());

    string name = current_standard_;

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
    auto ds_copy = data_sources_ref_[dbcontent_name_ref_];

    unsigned int ds_id;
    for (auto& ds_it : ds_copy)
    {
        ds_id = stoul(ds_it.first);

        if (!ds_man.hasDBDataSource(ds_id))
            data_sources_ref_[dbcontent_name_ref_].erase(ds_it.first);
    }

    // init non-existing ones with false
    if (ds_man.hasDataSourcesOfDBContent(dbcontent_name_ref_))
    {
        for (auto& ds_it : ds_man.dbDataSources())
        {
            if (!ds_it->hasNumInserted(dbcontent_name_ref_))
                continue;

            string ds_id_str = to_string(ds_it->id());

            if (!data_sources_ref_[dbcontent_name_ref_].count(ds_id_str))
                data_sources_ref_[dbcontent_name_ref_][ds_id_str] = false; // init with default false
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
    auto ds_copy = data_sources_tst_[dbcontent_name_tst_];

    unsigned int ds_id;
    for (auto& ds_it : ds_copy)
    {
        ds_id = stoul(ds_it.first);

        if (!ds_man.hasDBDataSource(ds_id))
            data_sources_tst_[dbcontent_name_tst_].erase(ds_it.first);
    }

    // init non-existing ones with false
    if (ds_man.hasDataSourcesOfDBContent(dbcontent_name_tst_))
    {
        for (auto& ds_it : ds_man.dbDataSources())
        {
            if (!ds_it->hasNumInserted(dbcontent_name_tst_))
                continue;

            string ds_id_str = to_string(ds_it->id());

            if (!data_sources_tst_[dbcontent_name_tst_].count(ds_id_str))
                data_sources_tst_[dbcontent_name_tst_][ds_id_str] = false; // init with default false
        }
    }
}

bool EvaluationManager::hasSelectedReferenceDataSources()
{
    if (!hasValidReferenceDBContent())
        return false;

    for (auto& ds_it : data_sources_ref_[dbcontent_name_ref_])
        if (ds_it.second)
            return true;

    return false;
}

bool EvaluationManager::hasSelectedTestDataSources()
{
    if (!hasValidTestDBContent())
        return false;

    for (auto& ds_it : data_sources_tst_[dbcontent_name_tst_])
        if (ds_it.second)
            return true;

    return false;
}

bool EvaluationManager::reportSkipTargetsWoIssues() const
{
    return report_skip_targets_wo_issues_;
}

void EvaluationManager::reportSkipTargetsWoIssues(bool value)
{
    loginf << "EvaluationManager: reportSkipTargetsWoIssues: value " << value;

    report_skip_targets_wo_issues_ = value;
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
    data[VP_FILTERS_KEY]["UTNs"]["utns"] = to_string(utn);

    loginf << "EvaluationManager: showUTN: showing";
    setViewableDataConfig(data);
}

std::unique_ptr<nlohmann::json::object_t> EvaluationManager::getViewableForUTN (unsigned int utn)
{
    nlohmann::json::object_t data = getBaseViewableDataConfig();
    data[VP_FILTERS_KEY]["UTNs"]["utns"] = to_string(utn);

    return std::unique_ptr<nlohmann::json::object_t>{new nlohmann::json::object_t(move(data))};
}

std::unique_ptr<nlohmann::json::object_t> EvaluationManager::getViewableForEvaluation (
        const std::string& req_grp_id, const std::string& result_id)
{
    nlohmann::json::object_t data = getBaseViewableNoDataConfig();

    data[VP_EVAL_KEY][VP_EVAL_SHOW_RES_KEY] = true;
    data[VP_EVAL_KEY][VP_EVAL_REQGRP_ID_KEY] = req_grp_id;
    data[VP_EVAL_KEY][VP_EVAL_RES_ID_KEY] = result_id;

    data[VP_SHOWSEC_KEY] = vector<string>({String::split(req_grp_id, ':').at(0)});

    return std::unique_ptr<nlohmann::json::object_t>{new nlohmann::json::object_t(move(data))};
}

std::unique_ptr<nlohmann::json::object_t> EvaluationManager::getViewableForEvaluation (
        unsigned int utn, const std::string& req_grp_id, const std::string& result_id)
{
    nlohmann::json::object_t data = getBaseViewableDataConfig();
    data[VP_FILTERS_KEY]["UTNs"]["utns"] = to_string(utn);

    data[VP_EVAL_KEY][VP_EVAL_SHOW_RES_KEY] = true;
    data[VP_EVAL_KEY][VP_EVAL_REQGRP_ID_KEY] = req_grp_id;
    data[VP_EVAL_KEY][VP_EVAL_RES_ID_KEY] = result_id;

    data[VP_SHOWSEC_KEY] = vector<string>({String::split(req_grp_id, ':').at(0)});

    return std::unique_ptr<nlohmann::json::object_t>{new nlohmann::json::object_t(move(data))};
}

void EvaluationManager::showResultId (const std::string& id)
{
    loginf << "EvaluationManager: showResultId: id '" << id << "'";

    assert (widget_);
    widget_->showResultId(id);
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
    data[VP_FILTERS_KEY]["UTNs"]["utns"] = to_string(utn);

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
    data[VP_FILTERS_KEY]["Timestamp"]["Timestamp Maximum"] = Time::toString(time_end);
    data[VP_FILTERS_KEY]["Timestamp"]["Timestamp Minimum"] = Time::toString(time_begin);

    //    "Aircraft Address": {
    //    "Aircraft Address Values": "FEFE10"
    //    },
    if (target_data.targetAddresses().size())
        data[VP_FILTERS_KEY]["Aircraft Address"]["Aircraft Address Values"] = target_data.targetAddressesStr()+",NULL";

    //    "Mode 3/A Code": {
    //    "Mode 3/A Code Values": "7000"
    //    }

    if (target_data.modeACodes().size())
        data[VP_FILTERS_KEY]["Mode 3/A Codes"]["Mode 3/A Codes Values"] = target_data.modeACodesStr()+",NULL";

    //    VP_FILTERS_KEY: {
    //    "Barometric Altitude": {
    //    "Barometric Altitude Maximum": "43000",
    //    "Barometric Altitude Minimum": "500"
    //    },

    //    if (target_data.hasModeC())
    //    {
    //        float alt_min = target_data.modeCMin();
    //        alt_min -= 300;
    //        float alt_max = target_data.modeCMax();
    //        alt_max += 300;
    //    }

    //    "Position": {
    //    "Latitude Maximum": "50.78493920733",
    //    "Latitude Minimum": "44.31547147615",
    //    "Longitude Maximum": "20.76559892354",
    //    "Longitude Minimum": "8.5801592186"
    //    }

    if (target_data.hasPos())
    {
        data[VP_FILTERS_KEY]["Position"]["Latitude Maximum"] = to_string(target_data.latitudeMax()+0.2);
        data[VP_FILTERS_KEY]["Position"]["Latitude Minimum"] = to_string(target_data.latitudeMin()-0.2);
        data[VP_FILTERS_KEY]["Position"]["Longitude Maximum"] = to_string(target_data.longitudeMax()+0.2);
        data[VP_FILTERS_KEY]["Position"]["Longitude Minimum"] = to_string(target_data.longitudeMin()-0.2);
    }

    setViewableDataConfig(data);
}

json::boolean_t& EvaluationManager::useGroupInSectorLayer(const std::string& sector_layer_name,
                                                          const std::string& group_name)
{
    assert (hasCurrentStandard());

    // standard_name->sector_layer_name->req_grp_name->bool use
    if (!use_grp_in_sector_.contains(current_standard_)
            || !use_grp_in_sector_.at(current_standard_).contains(sector_layer_name)
            || !use_grp_in_sector_.at(current_standard_).at(sector_layer_name).contains(group_name))
        use_grp_in_sector_[current_standard_][sector_layer_name][group_name] = true;

    return use_grp_in_sector_[current_standard_][sector_layer_name][group_name].get_ref<json::boolean_t&>();
}

void EvaluationManager::useGroupInSectorLayer(const std::string& sector_layer_name,
                                              const std::string& group_name, bool value)
{
    assert (hasCurrentStandard());

    loginf << "EvaluationManager: useGroupInSector: standard_name " << current_standard_
           << " sector_layer_name " << sector_layer_name << " group_name " << group_name << " value " << value;

    use_grp_in_sector_[current_standard_][sector_layer_name][group_name] = value;

    if (widget_)
        widget_->updateButtons();
}

json::boolean_t& EvaluationManager::useRequirement(const std::string& standard_name, const std::string& group_name,
                                                   const std::string& req_name)
{
    // standard_name->req_grp_name->req_grp_name->bool use
    if (!use_requirement_.contains(standard_name)
            || !use_requirement_.at(standard_name).contains(group_name)
            || !use_requirement_.at(standard_name).at(group_name).contains(req_name))
        use_requirement_[standard_name][group_name][req_name] = true;

    return use_requirement_[standard_name][group_name][req_name].get_ref<json::boolean_t&>();
}

EvaluationResultsReport::PDFGenerator& EvaluationManager::pdfGenerator()
{
    return pdf_gen_;
}

bool EvaluationManager::loadOnlySectorData() const
{
    return load_only_sector_data_;
}

void EvaluationManager::loadOnlySectorData(bool value)
{
    load_only_sector_data_ = value;
}

nlohmann::json::object_t EvaluationManager::getBaseViewableDataConfig ()
{
    nlohmann::json data;

    // set data sources

    std::map<unsigned int, std::set<unsigned int>> data_sources;

    for (auto& src_it : data_sources_ref_[dbcontent_name_ref_])
        if (src_it.second)
            data_sources[stoul(src_it.first)].insert(line_id_ref_);

    for (auto& src_it : data_sources_tst_[dbcontent_name_tst_])
        if (src_it.second)
            data_sources[stoul(src_it.first)].insert(line_id_tst_);

    data["data_sources"] = data_sources;

    if (load_only_sector_data_ && min_max_pos_set_)
    {
        data[VP_FILTERS_KEY]["Position"]["Latitude Maximum"] = to_string(latitude_max_);
        data[VP_FILTERS_KEY]["Position"]["Latitude Minimum"] = to_string(latitude_min_);
        data[VP_FILTERS_KEY]["Position"]["Longitude Maximum"] = to_string(longitude_max_);
        data[VP_FILTERS_KEY]["Position"]["Longitude Minimum"] = to_string(longitude_min_);
    }

    return data;
}

nlohmann::json::object_t EvaluationManager::getBaseViewableNoDataConfig ()
{
    nlohmann::json data;

    data["data_sources"] = std::map<unsigned int, std::set<unsigned int>>{};

    return data;
}

bool EvaluationManager::useV0() const
{
    return use_v0_;
}

void EvaluationManager::useV0(bool value)
{
    loginf << "EvaluationManager: useV0: value " << value;
    use_v0_ = value;
}

bool EvaluationManager::useV1() const
{
    return use_v1_;
}

void EvaluationManager::useV1(bool value)
{
    loginf << "EvaluationManager: useV1: value " << value;
    use_v1_ = value;
}

bool EvaluationManager::useV2() const
{
    return use_v2_;
}

void EvaluationManager::useV2(bool value)
{
    loginf << "EvaluationManager: useV2: value " << value;
    use_v2_ = value;
}

bool EvaluationManager::useMinNUCP() const
{
    return use_min_nucp_;
}

void EvaluationManager::useMinNUCP(bool value)
{
    loginf << "EvaluationManager: useMinNUCP: value " << value;
    use_min_nucp_ = value;
}

unsigned int EvaluationManager::minNUCP() const
{
    return min_nucp_;
}

void EvaluationManager::minNUCP(unsigned int value)
{
    loginf << "EvaluationManager: minNUCP: value " << value;
    min_nucp_ = value;
}

bool EvaluationManager::useMinNIC() const
{
    return use_min_nic_;
}

void EvaluationManager::useMinNIC(bool value)
{
    loginf << "EvaluationManager: useMinNIC: value " << value;
    use_min_nic_ = value;
}

unsigned int EvaluationManager::minNIC() const
{
    return min_nic_;
}

void EvaluationManager::minNIC(unsigned int value)
{
    loginf << "EvaluationManager: minNIC: value " << value;
    min_nic_ = value;
}

bool EvaluationManager::useMinNACp() const
{
    return use_min_nacp_;
}

void EvaluationManager::useMinNACp(bool value)
{
    loginf << "EvaluationManager: useMinNACp: value " << value;
    use_min_nacp_ = value;
}

unsigned int EvaluationManager::minNACp() const
{
    return min_nacp_;
}

void EvaluationManager::minNACp(unsigned int value)
{
    loginf << "EvaluationManager: minNACp: value " << value;
    min_nacp_ = value;
}

bool EvaluationManager::useMinSILv1() const
{
    return use_min_sil_v1_;
}

void EvaluationManager::useMinSILv1(bool value)
{
    loginf << "EvaluationManager: useMinSILv1: value " << value;
    use_min_sil_v1_ = value;
}

unsigned int EvaluationManager::minSILv1() const
{
    return min_sil_v1_;
}

void EvaluationManager::minSILv1(unsigned int value)
{
    loginf << "EvaluationManager: minSILv1: value " << value;
    min_sil_v1_ = value;
}

bool EvaluationManager::useMinSILv2() const
{
    return use_min_sil_v2_;
}

void EvaluationManager::useMinSILv2(bool value)
{
    loginf << "EvaluationManager: useMinSILv2: value " << value;
    use_min_sil_v2_ = value;
}

unsigned int EvaluationManager::minSILv2() const
{
    return min_sil_v2_;
}

void EvaluationManager::minSILv2(unsigned int value)
{
    loginf << "EvaluationManager: minSILv2: value " << value;
    min_sil_v2_ = value;
}

bool EvaluationManager::useMaxNUCP() const
{
    return use_max_nucp_;
}

void EvaluationManager::useMaxNUCP(bool value)
{
    loginf << "EvaluationManager: useMaxNUCP: value " << value;
    use_max_nucp_ = value;
}

unsigned int EvaluationManager::maxNUCP() const
{
    return max_nucp_;
}

void EvaluationManager::maxNUCP(unsigned int value)
{
    loginf << "EvaluationManager: maxNUCP: value " << value;
    max_nucp_ = value;
}

bool EvaluationManager::useMaxNIC() const
{
    return use_max_nic_;
}

void EvaluationManager::useMaxNIC(bool value)
{
    loginf << "EvaluationManager: useMaxNIC: value " << value;
    use_max_nic_ = value;
}

unsigned int EvaluationManager::maxNIC() const
{
    return max_nic_;
}

void EvaluationManager::maxNIC(unsigned int value)
{
    loginf << "EvaluationManager: maxNIC: value " << value;
    max_nic_ = value;
}

bool EvaluationManager::useMaxNACp() const
{
    return use_max_nacp_;
}

void EvaluationManager::useMaxNACp(bool value)
{
    loginf << "EvaluationManager: useMaxNACp: value " << value;
    use_max_nacp_ = value;
}

unsigned int EvaluationManager::maxNACp() const
{
    return max_nacp_;
}

void EvaluationManager::maxNACp(unsigned int value)
{
    loginf << "EvaluationManager: maxNACp: value " << value;
    max_nacp_ = value;
}

bool EvaluationManager::useMaxSILv1() const
{
    return use_max_sil_v1_;
}

void EvaluationManager::useMaxSILv1(bool value)
{
    loginf << "EvaluationManager: useMaxSILv1: value " << value;
    use_max_sil_v1_ = value;
}

unsigned int EvaluationManager::maxSILv1() const
{
    return max_sil_v1_;
}

void EvaluationManager::maxSILv1(unsigned int value)
{
    loginf << "EvaluationManager: maxSILv1: value " << value;
    max_sil_v1_ = value;
}

bool EvaluationManager::useMaxSILv2() const
{
    return use_max_sil_v2_;
}

void EvaluationManager::useMaxSILv2(bool value)
{
    loginf << "EvaluationManager: useMaxSILv2: value " << value;
    use_max_sil_v2_ = value;
}

unsigned int EvaluationManager::maxSILv2() const
{
    return max_sil_v2_;
}

void EvaluationManager::maxSILv2(unsigned int value)
{
    loginf << "EvaluationManager: maxSILv2: value " << value;
    max_sil_v2_ = value;
}

bool EvaluationManager::useLoadFilter() const
{
    return use_load_filter_;
}

void EvaluationManager::useLoadFilter(bool value)
{
    loginf << "EvaluationManager: useLoadFilter: value " << value;
    use_load_filter_ = value;
}

bool EvaluationManager::useTimestampFilter() const
{
    return use_timestamp_filter_;
}

void EvaluationManager::useTimestampFilter(bool value)
{
    loginf << "EvaluationManager: useTimeFilter: value " << value;
    use_timestamp_filter_ = value;
}

boost::posix_time::ptime EvaluationManager::loadTimestampBegin() const
{
    return load_timestamp_begin_;
}

void EvaluationManager::loadTimestampBegin(boost::posix_time::ptime value)
{
    loginf << "EvaluationManager: loadTimeBegin: value " << Time::toString(value);

    load_timestamp_begin_ = value;
    load_timestamp_begin_str_ = Time::toString(load_timestamp_begin_);
}

boost::posix_time::ptime EvaluationManager::loadTimestampEnd() const
{
    return load_timestamp_end_;
}

void EvaluationManager::loadTimestampEnd(boost::posix_time::ptime value)
{
    loginf << "EvaluationManager: loadTimeEnd: value " << Time::toString(value);

    load_timestamp_end_ = value;
    load_timestamp_end_str_ = Time::toString(load_timestamp_end_);
}

bool EvaluationManager::useASDBFilter() const
{
    return use_adsb_filter_;
}

void EvaluationManager::useASDBFilter(bool value)
{
    loginf << "EvaluationManager: useASDBFilter: value " << value;
    use_adsb_filter_ = value;
}

float EvaluationManager::maxRefTimeDiff() const
{
    return max_ref_time_diff_;
}

void EvaluationManager::maxRefTimeDiff(float value)
{
    loginf << "EvaluationManager: maxRefTimeDiff: value " << value;

    max_ref_time_diff_ = value;
}

bool EvaluationManager::warningShown() const
{
    return warning_shown_;
}

void EvaluationManager::warningShown(bool warning_shown)
{
    warning_shown_ = warning_shown;
}

double EvaluationManager::resultDetailZoom() const
{
    return result_detail_zoom_;
}

void EvaluationManager::resultDetailZoom(double result_detail_zoom)
{
    result_detail_zoom_ = result_detail_zoom;
}

bool EvaluationManager::reportSkipNoDataDetails() const
{
    return report_skip_no_data_details_;
}

void EvaluationManager::reportSkipNoDataDetails(bool value)
{
    report_skip_no_data_details_ = value;
}

bool EvaluationManager::reportSplitResultsByMOPS() const
{
    return report_split_results_by_mops_;
}

void EvaluationManager::reportSplitResultsByMOPS(bool value)
{
    report_split_results_by_mops_ = value;
}

bool EvaluationManager::reportShowAdsbInfo() const
{
    return report_show_adsb_info_;
}

void EvaluationManager::reportShowAdsbInfo(bool value)
{
    report_show_adsb_info_ = value;
}

std::string EvaluationManager::reportAuthor() const
{
    return report_author_;
}

void EvaluationManager::reportAuthor(const std::string& author)
{
    report_author_ = author;
}

std::string EvaluationManager::reportAbstract() const
{
    return report_abstract_;
}

void EvaluationManager::reportAbstract(const std::string& abstract)
{
    report_abstract_ = abstract;
}

bool EvaluationManager::reportRunPDFLatex() const
{
    return report_run_pdflatex_;
}

void EvaluationManager::reportRunPDFLatex(bool value)
{
    report_run_pdflatex_ = value;
}


bool EvaluationManager::reportOpenCreatedPDF() const
{
    return report_open_created_pdf_;
}

void EvaluationManager::reportOpenCreatedPDF(bool value)
{
    report_open_created_pdf_ = value;
}

bool EvaluationManager::reportWaitOnMapLoading() const
{
    return report_wait_on_map_loading_;
}

void EvaluationManager::reportWaitOnMapLoading(bool value)
{
    report_wait_on_map_loading_ = value;
}

bool EvaluationManager::reportIncludeTargetDetails() const
{
    return report_include_target_details_;
}

void EvaluationManager::reportIncludeTargetDetails(bool value)
{
    report_include_target_details_ = value;
}

bool EvaluationManager::reportIncludeTargetTRDetails() const
{
    return report_include_target_tr_details_;
}

void EvaluationManager::reportIncludeTargetTRDetails(bool value)
{
    report_include_target_tr_details_ = value;
}

unsigned int EvaluationManager::reportNumMaxTableRows() const
{
    return report_num_max_table_rows_;
}

void EvaluationManager::reportNumMaxTableRows(unsigned int value)
{
    report_num_max_table_rows_ = value;
}

unsigned int EvaluationManager::reportNumMaxTableColWidth() const
{
    return report_num_max_table_col_width_;
}

void EvaluationManager::reportNumMaxTableColWidth(unsigned int value)
{
    report_num_max_table_col_width_ = value;
}

void EvaluationManager::updateActiveDataSources() // save to config var
{
    active_sources_ref_ = data_sources_ref_;
    active_sources_tst_ = data_sources_tst_;

    widget_->updateButtons();
}

unsigned int EvaluationManager::lineIDRef() const
{
    return line_id_ref_;
}

void EvaluationManager::lineIDRef(unsigned int line_id_ref)
{
    line_id_ref_ = line_id_ref;
}

unsigned int EvaluationManager::lineIDTst() const
{
    return line_id_tst_;
}

void EvaluationManager::lineIDTst(unsigned int line_id_tst)
{
    line_id_tst_ = line_id_tst;
}

