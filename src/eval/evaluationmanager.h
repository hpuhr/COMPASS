#ifndef EVALUATIONMANAGER_H
#define EVALUATIONMANAGER_H

#include <QObject>

#include "json.hpp"

#include "configurable.h"
#include "sectorlayer.h"
#include "activedatasource.h"

class ATSDB;
class EvaluationManagerWidget;

class QWidget;
class QTabWidget;

class EvaluationManager : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void sectorsChangedSignal();

public slots:

public:
    EvaluationManager(const std::string& class_id, const std::string& instance_id, ATSDB* atsdb);
    virtual ~EvaluationManager();

    void init(QTabWidget* tab_widget);

    void loadData ();
    void evaluate ();
    void generateReport ();

    void close();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    EvaluationManagerWidget* widget();

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

    std::string dboNameTst() const;
    void dboNameTst(const std::string& name);
    bool hasValidTestDBO ();
    std::map<int, ActiveDataSource>& dataSourcesTst() { return data_sources_tst_; }

    bool dataLoaded() const;
    bool evaluated() const;

protected:
    ATSDB& atsdb_;

    bool initialized_ {false};
    bool data_loaded_ {false};
    bool evaluated_ {false};

    std::string dbo_name_ref_;
    std::map<int, ActiveDataSource> data_sources_ref_;
    nlohmann::json active_sources_ref_;

    std::string dbo_name_tst_;
    std::map<int, ActiveDataSource> data_sources_tst_;
    nlohmann::json active_sources_tst_;

    std::unique_ptr<EvaluationManagerWidget> widget_{nullptr};

    std::vector<std::shared_ptr<SectorLayer>> sector_layers_;

    virtual void checkSubConfigurables();

    void loadSectors();

    unsigned int getMaxSectorId ();

    void updateReferenceDBO();
    void updateReferenceDataSources();
    void updateReferenceDataSourcesActive();

    void updateTestDBO();
    void updateTestDataSources();
    void updateTestDataSourcesActive();
};

#endif // EVALUATIONMANAGER_H
