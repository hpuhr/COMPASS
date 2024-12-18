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

#pragma once

#include "configurable.h"
#include "evaluationdata.h"
#include "evaluationresultsgenerator.h"
#include "viewabledataconfig.h"
#include "evaluationmanagerwidget.h"
#include "eval/results/report/pdfgenerator.h"
#include "datasourcecompoundcoverage.h"
#include "sector.h"

#include <QObject>

#include "json.hpp"

class COMPASS;
class EvaluationStandard;
class DBContent;
class SectorLayer;
class AirSpace;

namespace dbContent {

class VariableSet;

}

class QWidget;
class QTabWidget;

struct EvaluationManagerSettings
{
    EvaluationManagerSettings();

    unsigned int line_id_ref_;
    nlohmann::json active_sources_ref_; // config var for data_sources_ref_

    unsigned int line_id_tst_;
    nlohmann::json active_sources_tst_; // config var for active_sources_tst_

    std::string current_standard_;
    //std::string current_config_name_;

    nlohmann::json use_grp_in_sector_; //standard_name->sector_layer_name->req_grp_name->bool use
    nlohmann::json use_requirement_; // standard_name->req_grp_name->req_grp_name->bool use

    float max_ref_time_diff_ {0};

    // load filter
    bool use_load_filter_ {false};

    bool use_timestamp_filter_ {false};

    bool use_ref_traj_accuracy_filter_ {false};
    float ref_traj_minimum_accuracy_ {30};

    bool use_adsb_filter_ {false};
    bool use_v0_ {false};
    bool use_v1_ {false};
    bool use_v2_ {false};

    // nucp
    bool use_min_nucp_ {false};
    unsigned int min_nucp_ {0};

    bool use_max_nucp_ {false};
    unsigned int max_nucp_ {0};

    // nic
    bool use_min_nic_ {false};
    unsigned int min_nic_ {0};

    bool use_max_nic_ {false};
    unsigned int max_nic_ {0};

    // nacp
    bool use_min_nacp_ {false};
    unsigned int min_nacp_ {0};

    bool use_max_nacp_ {false};
    unsigned int max_nacp_ {0};

    // sil v1
    bool use_min_sil_v1_ {false};
    unsigned int min_sil_v1_ {0};

    bool use_max_sil_v1_ {false};
    unsigned int max_sil_v1_ {0};

    // sil v2
    bool use_min_sil_v2_ {false};
    unsigned int min_sil_v2_ {0};

    bool use_max_sil_v2_ {false};
    unsigned int max_sil_v2_ {0};

    double result_detail_zoom_ {0.0}; // in WGS84 deg

    std::string min_height_filter_layer_; //layer used as minimum height filter

    // report stuff
    bool report_skip_no_data_details_ {true};
    bool report_split_results_by_mops_ {false};
    bool report_split_results_by_aconly_ms_ {false};

    std::string report_author_;
    std::string report_abstract_;

    bool report_include_target_details_ {false};
    bool report_skip_targets_wo_issues_ {false};
    bool report_include_target_tr_details_ {false};

    bool show_ok_joined_target_reports_ {false};

    unsigned int report_num_max_table_rows_ {1000};
    unsigned int report_num_max_table_col_width_ {18};

    bool report_wait_on_map_loading_ {true};

    bool report_run_pdflatex_ {true};
    bool report_open_created_pdf_ {false};

    bool warning_shown_ {false};

    //grid generation
    unsigned int grid_num_cells_x     = 512;
    unsigned int grid_num_cells_y     = 512;

    //histogram generation
    unsigned int histogram_num_bins = 20;

    //not written to config
    bool load_only_sector_data_ {true};

private:
    friend class EvaluationManager;

    // private since specific setter functions
    std::string dbcontent_name_ref_;
    std::string dbcontent_name_tst_;

    std::string load_timestamp_begin_str_;
    std::string load_timestamp_end_str_;
};

class EvaluationManager : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void sectorsChangedSignal();
    void standardsChangedSignal(); // emitted if standard was added or deleted
    void currentStandardChangedSignal(); // emitted if current standard was changed

    void resultsChangedSignal();

public slots:
    void databaseOpenedSlot();
    void databaseClosedSlot();
    void dataSourcesChangedSlot();
    void associationStatusChangedSlot();

    void loadedDataDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSlot();

public:
    struct EvaluationDS
    {
        std::string  name;
        unsigned int id;
    };

    struct EvaluationDSInfo
    {
        std::string               dbcontent;
        std::vector<EvaluationDS> data_sources;
    };

    typedef std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>> ResultMap;
    typedef ResultMap::const_iterator ResultIterator;

    EvaluationManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass);
    virtual ~EvaluationManager();

    void init(QTabWidget* tab_widget);

    bool canLoadData ();
    void loadData ();

    bool canEvaluate ();
    std::string getCannotEvaluateComment();
    void evaluate ();
    bool canGenerateReport ();
    void generateReport ();

    void close();

    bool needsAdditionalVariables ();
    void addVariables (const std::string dbcontent_name, dbContent::VariableSet& read_set);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    EvaluationManagerWidget* widget();

    bool sectorsLoaded() const;
    bool anySectorsWithReq();

    bool hasSectorLayer (const std::string& layer_name) const;
    //void renameSectorLayer (const std::string& name, const std::string& new_name);
    std::shared_ptr<SectorLayer> sectorLayer(const std::string& layer_name) const;

    void createNewSector (const std::string& name, const std::string& layer_name,
                          bool exclude, QColor color, std::vector<std::pair<double,double>> points);
    bool hasSector (const std::string& name, const std::string& layer_name);
    bool hasSector (unsigned int id);
    std::shared_ptr<Sector> sector (const std::string& name, const std::string& layer_name);
    std::shared_ptr<Sector> sector (unsigned int id);
    void moveSector(unsigned int id, const std::string& old_layer_name, const std::string& new_layer_name); // moves sector to new layer, saves
    void saveSector(unsigned int id); // write to db and add
    void saveSector(std::shared_ptr<Sector> sector); // write to db and add
    std::vector<std::shared_ptr<SectorLayer>>& sectorsLayers();
    void deleteSector(std::shared_ptr<Sector> sector);
    void deleteAllSectors();
    void importSectors (const std::string& filename);
    void exportSectors (const std::string& filename);
    unsigned int getMaxSectorId ();
    void updateSectorLayers();

    bool importAirSpace(const AirSpace& air_space, 
                        const boost::optional<std::set<std::string>>& sectors_to_import = {});
    bool filterMinimumHeight() const;
    const std::string& minHeightFilterLayerName() const;
    void minHeightFilterLayerName(const std::string& layer_name);
    std::shared_ptr<SectorLayer> minHeightFilterLayer() const;

    std::string dbContentNameRef() const;
    void dbContentNameRef(const std::string& name);

    bool hasValidReferenceDBContent ();
    std::map<std::string, bool>& dataSourcesRef() { return data_sources_ref_[settings_.dbcontent_name_ref_]; } // can be used to set active bool
    std::set<unsigned int> activeDataSourcesRef();
    EvaluationDSInfo activeDataSourceInfoRef() const;

    std::string dbContentNameTst() const;
    void dbContentNameTst(const std::string& name);

    bool hasValidTestDBContent ();
    std::map<std::string, bool>& dataSourcesTst() { return data_sources_tst_[settings_.dbcontent_name_tst_]; } // can be used to set active bool
    std::set<unsigned int> activeDataSourcesTst();
    EvaluationDSInfo activeDataSourceInfoTst() const;

    bool dataLoaded() const;
    bool evaluated() const;

    EvaluationData& getData();

    bool hasCurrentStandard();
    std::string currentStandardName() const; // can return empty string, indicating no standard
    void currentStandardName(const std::string& current_standard);
    void renameCurrentStandard (const std::string& new_name);
    void copyCurrentStandard (const std::string& new_name);
    EvaluationStandard& currentStandard();

    bool hasStandard(const std::string& name);
    void addStandard(const std::string& name);
    void deleteCurrentStandard();

    using EvaluationStandardIterator = typename std::vector<std::unique_ptr<EvaluationStandard>>::iterator;
    EvaluationStandardIterator standardsBegin() { return standards_.begin(); }
    EvaluationStandardIterator standardsEnd() { return standards_.end(); }
    unsigned int standardsSize () { return standards_.size(); };

    std::vector<std::string> currentRequirementNames();

    EvaluationResultsGenerator& resultsGenerator();

    void setViewableDataConfig (const nlohmann::json::object_t& data);
    void showUTN (unsigned int utn);
    std::unique_ptr<nlohmann::json::object_t> getViewableForUTN (unsigned int utn);
    std::unique_ptr<nlohmann::json::object_t> getViewableForEvaluation (
            const std::string& req_grp_id, const std::string& result_id); // empty load
    std::unique_ptr<nlohmann::json::object_t> getViewableForEvaluation (
            unsigned int utn, const std::string& req_grp_id, const std::string& result_id); // with data
    void showResultId (const std::string& id, 
                       bool select_tab = false,
                       bool show_figure = false);

    ResultIterator begin();
    ResultIterator end();

    bool hasResults();
    const ResultMap& results() const;

    void updateResultsToChanges ();
    void showFullUTN (unsigned int utn);
    void showSurroundingData (unsigned int utn);

    nlohmann::json::boolean_t& useGroupInSectorLayer(const std::string& sector_layer_name,
                                                     const std::string& group_name);
    void useGroupInSectorLayer(const std::string& sector_layer_name, const std::string& group_name, bool value);

    nlohmann::json::boolean_t& useRequirement(const std::string& standard_name, const std::string& group_name,
                                              const std::string& req_name);

    EvaluationResultsReport::PDFGenerator& pdfGenerator();

    boost::posix_time::ptime loadTimestampBegin() const;
    void loadTimestampBegin(boost::posix_time::ptime value);

    boost::posix_time::ptime loadTimestampEnd() const;
    void loadTimestampEnd(boost::posix_time::ptime value);

    const EvaluationManagerSettings& settings() const { return settings_; }

    // updaters
    void updateActiveDataSources(); // save to config var

    bool hasSelectedReferenceDataSources();
    bool hasSelectedTestDataSources();

    const dbContent::DataSourceCompoundCoverage& tstSrcsCoverage() const;

    void clearLoadedDataAndResults();

protected:
    virtual void checkSubConfigurables() override;

    void loadSectors();

    void checkReferenceDataSources();
    void checkTestDataSources();

    void updateMaxSectorID();

    void checkMinHeightFilterValid();

    nlohmann::json::object_t getBaseViewableDataConfig ();
    nlohmann::json::object_t getBaseViewableNoDataConfig ();

    void updateCompoundCoverage(std::set<unsigned int> tst_sources);

    virtual void onConfigurationChanged(const std::vector<std::string>& changed_params) override;

    COMPASS& compass_;

    EvaluationManagerSettings settings_;

    std::map<std::string, std::map<std::string, bool>> data_sources_ref_ ; // db_content -> ds_id -> active flag
    std::map<std::string, std::map<std::string, bool>> data_sources_tst_; // db_content -> ds_id -> active flag

    boost::posix_time::ptime load_timestamp_begin_;
    boost::posix_time::ptime load_timestamp_end_;

    bool min_max_pos_set_ {false};
    double latitude_min_ {0};
    double latitude_max_ {0};
    double longitude_min_ {0};
    double longitude_max_ {0};

    bool sectors_loaded_ {false};
    bool initialized_ {false};

    bool needs_additional_variables_ {false}; // indicates if variables should be added during loading
    bool data_loaded_ {false};

    bool reference_data_loaded_ {false};
    bool test_data_loaded_ {false};

    bool evaluated_ {false};

    std::unique_ptr<EvaluationManagerWidget> widget_{nullptr};

    std::vector<std::shared_ptr<SectorLayer>> sector_layers_;
    unsigned int max_sector_id_ {0};
    std::vector<std::unique_ptr<EvaluationStandard>> standards_;

    EvaluationData data_;
    EvaluationResultsGenerator results_gen_;
    EvaluationResultsReport::PDFGenerator pdf_gen_;

    std::unique_ptr<ViewableDataConfig> viewable_data_cfg_;

    bool has_adsb_info_ {false};
    std::map<unsigned int, std::tuple<std::set<unsigned int>, std::tuple<bool, unsigned int, unsigned int>,
        std::tuple<bool, unsigned int, unsigned int>>> adsb_info_;

    bool use_fast_sector_inside_check_ = true;

    dbContent::DataSourceCompoundCoverage tst_srcs_coverage_;
};
