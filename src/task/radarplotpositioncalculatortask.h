#ifndef RADARPLOTPOSITIONCALCULATOR_H_
#define RADARPLOTPOSITIONCALCULATOR_H_

#include "configurable.h"
#include "dbodatasource.h"

#include <QObject>
#include <memory>

class Buffer;
class DBObject;
class DBOVariable;
class RadarPlotPositionCalculatorTaskWidget;
class TaskManager;

class RadarPlotPositionCalculatorTask : public QObject, public Configurable
{
    Q_OBJECT

public slots:
    //    void readJobIntermediateSlot (std::shared_ptr<Buffer> buffer);
    //    void readJobObsoleteSlot ();
    //    void readJobDoneSlot();
    void newDataSlot (DBObject &object);
    void loadingDoneSlot (DBObject &object);

public:
    RadarPlotPositionCalculatorTask(const std::string &class_id, const std::string &instance_id, TaskManager* task_manager);
    virtual ~RadarPlotPositionCalculatorTask();

    void calculate ();

    bool isCalculating ();
    unsigned int getNumLoaded () { return num_loaded_; }

    RadarPlotPositionCalculatorTaskWidget* widget();

    std::string dbObjectStr() const;
    void dbObjectStr(const std::string &db_object_str);

    std::string keyVarStr() const;
    void keyVarStr(const std::string &key_var_str);

    std::string datasourceVarStr() const;
    void datasourceVarStr(const std::string &datasource_var_str);

    std::string rangeVarStr() const;
    void rangeVarStr(const std::string &range_var_str);

    std::string azimuthVarStr() const;
    void azimuthVarStr(const std::string &azimuth_var_str);

    std::string altitudeVarStr() const;
    void altitudeVarStr(const std::string &altitude_var_str);

    std::string latitudeVarStr() const;
    void latitudeVarStr(const std::string &latitude_var_str);

    std::string longitudeVarStr() const;
    void longitudeVarStr(const std::string &longitude_var_str);

protected:
    std::string db_object_str_;
    DBObject* db_object_{nullptr};

    std::string key_var_str_;
    DBOVariable* key_var_{nullptr};

    std::string datasource_var_str_;
    DBOVariable* datasource_var_{nullptr};

    std::string range_var_str_;
    DBOVariable* range_var_{nullptr};

    std::string azimuth_var_str_;
    DBOVariable* azimuth_var_{nullptr};

    std::string altitude_var_str_;
    DBOVariable* altitude_var_{nullptr};

    std::string latitude_var_str_;
    DBOVariable* latitude_var_{nullptr};

    std::string longitude_var_str_;
    DBOVariable* longitude_var_{nullptr};

    std::map<int, DBODataSource> data_sources_;

    bool calculating_ {false};

    unsigned int num_loaded_ {0};

    RadarPlotPositionCalculatorTaskWidget* widget_ {nullptr};

    void checkAndSetVariable (std::string &name_str, DBOVariable** var);
};

#endif /* RADARPLOTPOSITIONCALCULATOR_H_ */
