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

#ifndef EVALUATIONMANAGER_H
#define EVALUATIONMANAGER_H

#include "configurable.h"
#include "sectorlayer.h"
#include "evaluationdata.h"
#include "evaluationresultsgenerator.h"
#include "viewabledataconfig.h"
#include "evaluationmanagerwidget.h"
#include "eval/results/report/pdfgenerator.h"
#include "eval/results/report/pdfgeneratordialog.h"

#include <QObject>

#include "json.hpp"

class COMPASS;
class EvaluationStandard;
class DBContent;

namespace dbContent {

class VariableSet;

}

class QWidget;
class QTabWidget;

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
    void associationStatusChangedSlot();

    void loadedDataDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSlot();

//    void newDataSlot(DBContent& object);
//    void loadingDoneSlot(DBContent& object);

public:
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
    void addVariables (const std::string dbo_name, dbContent::VariableSet& read_set);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    EvaluationManagerWidget* widget();

    bool sectorsLoaded() const;

    bool hasSectorLayer (const std::string& layer_name);
    //void renameSectorLayer (const std::string& name, const std::string& new_name);
    std::shared_ptr<SectorLayer> sectorLayer (const std::string& layer_name);

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

    std::string dboNameRef() const;
    void dboNameRef(const std::string& name);

    unsigned int lineIDRef() const;
    void lineIDRef(unsigned int line_id_ref);

    bool hasValidReferenceDBO ();
    std::map<unsigned int, bool>& dataSourcesRef() { return data_sources_ref_; } // can be used to set active bool
    std::set<unsigned int> activeDataSourcesRef();

    std::string dboNameTst() const;
    void dboNameTst(const std::string& name);

    unsigned int lineIDTst() const;
    void lineIDTst(unsigned int line_id_tst);

    bool hasValidTestDBO ();
    std::map<unsigned int, bool>& dataSourcesTst() { return data_sources_tst_; } // can be used to set active bool
    std::set<unsigned int> activeDataSourcesTst();

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
    void showResultId (const std::string& id);

    typedef std::map<std::string,
      std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>::const_iterator ResultIterator;

    ResultIterator begin();
    ResultIterator end();

    bool hasResults();
    const std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>& results()
    const;

    //void setUseTargetData (unsigned int utn, bool value);
    void updateResultsToChanges ();
    void showFullUTN (unsigned int utn);
    void showSurroundingData (unsigned int utn);

    nlohmann::json::boolean_t& useGroupInSectorLayer(const std::string& sector_layer_name,
                                                     const std::string& group_name);
    void useGroupInSectorLayer(const std::string& sector_layer_name, const std::string& group_name, bool value);

    nlohmann::json::boolean_t& useRequirement(const std::string& standard_name, const std::string& group_name,
                                              const std::string& req_name);

    EvaluationResultsReport::PDFGenerator& pdfGenerator() const;

    bool useUTN (unsigned int utn);
    void useUTN (unsigned int utn, bool value, bool update_td, bool update_res=true); // update target data
    void useAllUTNs (bool value);
    void clearUTNComments ();
    void filterUTNs ();

    std::string utnComment (unsigned int utn);
    void utnComment (unsigned int utn, std::string value, bool update_td); // update target data

    bool removeShortTargets() const;
    void removeShortTargets(bool value);

    unsigned int removeShortTargetsMinUpdates() const;
    void removeShortTargetsMinUpdates(unsigned int value);

    double removeShortTargetsMinDuration() const;
    void removeShortTargetsMinDuration(double value);

    bool removePsrOnlyTargets() const;
    void removePsrOnlyTargets(bool value);

    bool removeModeACodes() const;
    void removeModeACodes(bool value);

    bool removeModeCValues() const;
    void removeModeCValues(bool value);

    float removeModeCMinValue() const;
    void removeModeCMinValue(float value);

    std::string removeModeACodeValues() const;
    std::set<std::pair<int,int>> removeModeACodeData() const; // single ma,-1 or range ma1,ma2
    void removeModeACodeValues(const std::string& value);

    bool removeTargetAddresses() const;
    void removeTargetAddresses(bool value);

    std::string removeTargetAddressValues() const;
    std::set<unsigned int> removeTargetAddressData() const;
    void removeTargetAddressValues(const std::string& value);

    bool removeModeACOnlys() const;
    void removeModeACOnlys(bool value);

    bool removeNotDetectedDBOs() const;
    void removeNotDetectedDBOs(bool value);

    bool removeNotDetectedDBO(const std::string& dbo_name) const;
    void removeNotDetectedDBOs(const std::string& dbo_name, bool value);

    bool hasADSBInfo() const;
    bool hasADSBInfo(unsigned int ta) const;
    std::tuple<std::set<unsigned int>, std::tuple<bool, unsigned int, unsigned int>,
            std::tuple<bool, unsigned int, unsigned int>> adsbInfo(unsigned int ta) const;

    bool loadOnlySectorData() const;
    void loadOnlySectorData(bool value);

    bool useV0() const;
    void useV0(bool value);

    bool useV1() const;
    void useV1(bool value);

    bool useV2() const;
    void useV2(bool value);

    // nucp
    bool useMinNUCP() const;
    void useMinNUCP(bool value);

    unsigned int minNUCP() const;
    void minNUCP(unsigned int value);

    bool useMaxNUCP() const;
    void useMaxNUCP(bool value);

    unsigned int maxNUCP() const;
    void maxNUCP(unsigned int value);

    // nic
    bool useMinNIC() const;
    void useMinNIC(bool value);

    unsigned int minNIC() const;
    void minNIC(unsigned int value);

    bool useMaxNIC() const;
    void useMaxNIC(bool value);

    unsigned int maxNIC() const;
    void maxNIC(unsigned int value);

    // nacp
    bool useMinNACp() const;
    void useMinNACp(bool value);

    unsigned int minNACp() const;
    void minNACp(unsigned int value);

    bool useMaxNACp() const;
    void useMaxNACp(bool value);

    unsigned int maxNACp() const;
    void maxNACp(unsigned int value);

    // sil v1
    bool useMinSILv1() const;
    void useMinSILv1(bool value);

    unsigned int minSILv1() const;
    void minSILv1(unsigned int value);

    bool useMaxSILv1() const;
    void useMaxSILv1(bool value);

    unsigned int maxSILv1() const;
    void maxSILv1(unsigned int value);

    // sil v2
    bool useMinSILv2() const;
    void useMinSILv2(bool value);

    unsigned int minSILv2() const;
    void minSILv2(unsigned int value);

    bool useMaxSILv2() const;
    void useMaxSILv2(bool value);

    unsigned int maxSILv2() const;
    void maxSILv2(unsigned int value);

    // other
    bool useLoadFilter() const;
    void useLoadFilter(bool value);

    bool useTimeFilter() const;
    void useTimeFilter(bool value);

    float loadTimeBegin() const;
    void loadTimeBegin(float value);

    float loadTimeEnd() const;
    void loadTimeEnd(float value);

    bool useASDBFilter() const;
    void useASDBFilter(bool value);

    float maxRefTimeDiff() const;
    void maxRefTimeDiff(float value);

    bool warningShown() const;
    void warningShown(bool warning_shown);

    double resultDetailZoom() const;
    void resultDetailZoom(double result_detail_zoom);



protected:
    COMPASS& compass_;

    bool sectors_loaded_ {false};
    bool initialized_ {false};

    bool needs_additional_variables_ {false}; // indicates if variables should be added during loading
    bool data_loaded_ {false};

    bool reference_data_loaded_ {false};
    bool test_data_loaded_ {false};

    bool evaluated_ {false};

    std::string dbo_name_ref_;
    unsigned int line_id_ref_;
    std::map<unsigned int, bool> data_sources_ref_; // ds_id -> active flag
    nlohmann::json active_sources_ref_; // config var for data_sources_ref_

    std::string dbo_name_tst_;
    unsigned int line_id_tst_;
    std::map<unsigned int, bool> data_sources_tst_; // ds_id -> active flag
    nlohmann::json active_sources_tst_; // config var for active_sources_tst_

    std::string current_standard_;
    nlohmann::json configs_;
    std::string current_config_name_;

    float max_ref_time_diff_ {0};

    // utn filter stuff
    bool update_results_ {true}; // to supress updating of results during bulk operations

    bool remove_short_targets_ {true};
    unsigned int remove_short_targets_min_updates_ {10};
    double remove_short_targets_min_duration_ {60.0};

    bool remove_psr_only_targets_ {true};
    bool remove_modeac_onlys_ {false};

    bool remove_mode_a_codes_{false};
    std::string remove_mode_a_code_values_;

    bool remove_mode_c_values_{false};
    float remove_mode_c_min_value_;

    bool remove_target_addresses_{false};
    std::string remove_target_address_values_;

    bool remove_not_detected_dbos_{false};
    nlohmann::json remove_not_detected_dbo_values_;

    bool load_only_sector_data_ {true};

    bool min_max_pos_set_ {false};
    double latitude_min_ {0};
    double latitude_max_ {0};
    double longitude_min_ {0};
    double longitude_max_ {0};

    // load filter
    bool use_load_filter_ {false};

    bool use_time_filter_ {false};
    float load_time_begin_ {0};
    float load_time_end_ {0};

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

    bool warning_shown_ {false};

    std::unique_ptr<EvaluationManagerWidget> widget_{nullptr};

    std::vector<std::shared_ptr<SectorLayer>> sector_layers_;
    unsigned int max_sector_id_ {0};
    std::vector<std::unique_ptr<EvaluationStandard>> standards_;

    nlohmann::json use_grp_in_sector_; //standard_name->sector_layer_name->req_grp_name->bool use
    nlohmann::json use_requirement_; // standard_name->req_grp_name->req_grp_name->bool use

    EvaluationData data_;
    std::unique_ptr<EvaluationResultsGenerator> results_gen_;
    std::unique_ptr<EvaluationResultsReport::PDFGenerator> pdf_gen_;

    std::unique_ptr<ViewableDataConfig> viewable_data_cfg_;

    bool has_adsb_info_ {false};
    std::map<unsigned int, std::tuple<std::set<unsigned int>, std::tuple<bool, unsigned int, unsigned int>,
        std::tuple<bool, unsigned int, unsigned int>>> adsb_info_;

    virtual void checkSubConfigurables() override;

    void loadSectors();

    void updateReferenceDBO();
    void updateReferenceDataSources();

    void updateTestDBO();
    void updateTestDataSources();

    nlohmann::json::object_t getBaseViewableDataConfig ();
    nlohmann::json::object_t getBaseViewableNoDataConfig ();
};

#endif // EVALUATIONMANAGER_H
