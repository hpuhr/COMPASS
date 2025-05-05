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
#include "evaluationcalculator.h"
#include "viewabledataconfig.h"
#include "evaluationmanagerwidget.h"
#include "eval/results/report/pdfgenerator.h"
#include "datasourcecompoundcoverage.h"
#include "sector.h"
#include "result.h"

#include <QObject>

#include "json.hpp"

class COMPASS;
class EvaluationStandard;
class DBContent;
class SectorLayer;
class AirSpace;
class EvaluationSettings;

namespace dbContent 
{
    class VariableSet;
}

class QWidget;

/**
 */
class EvaluationManager : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void sectorsChangedSignal();
    void standardsChangedSignal(); // emitted if standard was added or deleted
    void currentStandardChangedSignal(); // emitted if current standard was changed
    void resultsChangedSignal();
    void hasNewData();

public slots:
    void databaseOpenedSlot();
    void databaseClosedSlot();
    void dataSourcesChangedSlot();
    void associationStatusChangedSlot();

public:
    EvaluationManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass);
    virtual ~EvaluationManager();

    void init();
    void close();
    void clearData();

    bool evaluated() const;
    Result canEvaluate() const;
    void evaluate(bool show_dialog);

    bool canGenerateReport() const;
    void generateReport();

    // sectors
    bool sectorsLoaded() const;

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

    bool importAirSpace(const AirSpace& air_space, 
                        const boost::optional<std::set<std::string>>& sectors_to_import = {});

    //data loading
    bool needsAdditionalVariables() const;
    void addVariables(const std::string dbcontent_name, dbContent::VariableSet& read_set);

    //EvaluationResultsReport::PDFGenerator& pdfGenerator();

    void setViewableDataConfig (const nlohmann::json::object_t& data);
    void resetViewableDataConfig(bool reset_view_point);

    EvaluationManagerWidget* widget();

    const EvaluationCalculator& calculator() const;
    EvaluationCalculator& calculator();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

protected:
    friend class EvaluationCalculator;

    virtual void checkSubConfigurables() override;
    virtual void onConfigurationChanged(const std::vector<std::string>& changed_params) override;

    void updateSectorLayers();

    void loadData(const EvaluationCalculator& calculator);
    std::map<std::string, std::shared_ptr<Buffer>> fetchData();

    COMPASS& compass() { return compass_; }

private:
    void loadSectors();
    void clearSectors();
    void updateMaxSectorID();

    void configureLoadFilters(const EvaluationCalculator& calculator);
    void loadingDone();

    COMPASS& compass_;

    bool sectors_loaded_ {false};
    bool initialized_ {false};

    bool needs_additional_variables_ {false}; // indicates if variables should be added during loading

    std::vector<std::shared_ptr<SectorLayer>> sector_layers_;
    unsigned int max_sector_id_ {0};

    std::unique_ptr<EvaluationCalculator> calculator_; // sub-configurable
    //EvaluationResultsReport::PDFGenerator pdf_gen_;

    std::unique_ptr<ViewableDataConfig>            viewable_data_cfg_;
    std::map<std::string, std::shared_ptr<Buffer>> raw_data_;

    std::unique_ptr<EvaluationManagerWidget> widget_{nullptr};
};
