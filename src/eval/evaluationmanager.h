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
#include "activedatasource.h"
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
class DBObject;
class DBOVariableSet;

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
    void newDataSlot(DBObject& object);
    void loadingDoneSlot(DBObject& object);

public:
    EvaluationManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass);
    virtual ~EvaluationManager();

    void init(QTabWidget* tab_widget);

    bool canLoadData ();
    void loadData ();
    bool canEvaluate ();
    void evaluate ();
    bool canGenerateReport ();
    void generateReport ();

    void close();

    bool needsAdditionalVariables ();
    void addVariables (const std::string dbo_name, DBOVariableSet& read_set);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    EvaluationManagerWidget* widget();

    bool sectorsLoaded() const;
    void loadSectors();

    bool hasSectorLayer (const std::string& layer_name);
    //void renameSectorLayer (const std::string& name, const std::string& new_name);
    std::shared_ptr<SectorLayer> sectorLayer (const std::string& layer_name);

    void createNewSector (const std::string& name, const std::string& layer_name,
                          std::vector<std::pair<double,double>> points);
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

    std::string dboNameRef() const;
    void dboNameRef(const std::string& name);
    bool hasValidReferenceDBO ();
    std::map<int, ActiveDataSource>& dataSourcesRef() { return data_sources_ref_; }
    std::set<int> activeDataSourcesRef();

    std::string dboNameTst() const;
    void dboNameTst(const std::string& name);
    bool hasValidTestDBO ();
    std::map<int, ActiveDataSource>& dataSourcesTst() { return data_sources_tst_; }
    std::set<int> activeDataSourcesTst();

    bool dataLoaded() const;
    bool evaluated() const;

    EvaluationData& getData();

    bool hasCurrentStandard();
    std::string currentStandardName() const; // can return empty string, indicating no standard
    void currentStandardName(const std::string& current_standard);
    EvaluationStandard& currentStandard();

    bool hasStandard(const std::string& name);
    void addStandard(const std::string& name);
    void deleteCurrentStandard();

    using EvaluationStandardIterator = typename std::map<std::string, std::unique_ptr<EvaluationStandard>>::iterator;
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

    ResultIterator begin() { return results_gen_.begin(); }
    ResultIterator end() { return results_gen_.end(); }

    bool hasResults() { return results_gen_.results().size(); }
    const std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>& results()
    const { return results_gen_.results(); } ;

    void setUseTargetData (unsigned int utn, bool value);
    void updateResultsToUseChangeOf (unsigned int utn);
    void showFullUTN (unsigned int utn);
    void showSurroundingData (unsigned int utn);

    nlohmann::json::boolean_t& useGroupInSectorLayer(const std::string& sector_layer_name,
                                                     const std::string& group_name);
    void useGroupInSectorLayer(const std::string& sector_layer_name, const std::string& group_name, bool value);

    nlohmann::json::boolean_t& useRequirement(const std::string& standard_name, const std::string& group_name,
                                              const std::string& req_name);

    EvaluationResultsReport::PDFGenerator& pdfGenerator() const;

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
    std::map<int, ActiveDataSource> data_sources_ref_;
    nlohmann::json active_sources_ref_;

    std::string dbo_name_tst_;
    std::map<int, ActiveDataSource> data_sources_tst_;
    nlohmann::json active_sources_tst_;

    std::string current_standard_;

    std::unique_ptr<EvaluationManagerWidget> widget_{nullptr};

    std::vector<std::shared_ptr<SectorLayer>> sector_layers_;
    std::map<std::string, std::unique_ptr<EvaluationStandard>> standards_;

    nlohmann::json use_grp_in_sector_; //standard_name->sector_layer_name->req_grp_name->bool use
    nlohmann::json use_requirement_; // standard_name->req_grp_name->req_grp_name->bool use

    EvaluationData data_;
    EvaluationResultsGenerator results_gen_;
    std::unique_ptr<EvaluationResultsReport::PDFGenerator> pdf_gen_;

    std::unique_ptr<ViewableDataConfig> viewable_data_cfg_;

    virtual void checkSubConfigurables() override;

    unsigned int getMaxSectorId ();

    void updateReferenceDBO();
    void updateReferenceDataSources();
    void updateReferenceDataSourcesActive();

    void updateTestDBO();
    void updateTestDataSources();
    void updateTestDataSourcesActive();

    nlohmann::json::object_t getBaseViewableDataConfig ();
    nlohmann::json::object_t getBaseViewableNoDataConfig ();
};

#endif // EVALUATIONMANAGER_H
