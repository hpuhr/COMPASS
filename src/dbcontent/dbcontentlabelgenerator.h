#ifndef DBCONTENTLABELGENERATOR_H
#define DBCONTENTLABELGENERATOR_H

#include "configurable.h"
#include "json.hpp"

#include <osg/Matrixd>

#include <QObject>

#include <set>

class Buffer;
class DBContentManager;
class DBContentLabelGeneratorWidget;
class GeometryLeafItemLabels;

enum LabelDirection
{
    LEFT_UP=0,
    RIGHT_UP,
    LEFT_DOWN,
    RIGHT_DOWN
};

class DBContentLabelGenerator : public QObject, public Configurable
{
    Q_OBJECT

public:
    DBContentLabelGenerator(const std::string& class_id, const std::string& instance_id,
                            DBContentManager& manager);
    virtual ~DBContentLabelGenerator();

    virtual void generateSubConfigurable(const std::string& class_id, const std::string& instance_id);

    std::vector<std::string> getLabelTexts(const std::string& dbcontent_name, unsigned int buffer_index);

    bool autoLabel() const;
    void autoLabel(bool auto_label);

//    void registerLeafItemLabel (GeometryLeafItemLabels& item_label);
//    void unregisterLeafItemLabel (GeometryLeafItemLabels& item_label);

    void autoAdustCurrentLOD(unsigned int num_labels_on_screen);

    unsigned int currentLOD() const;
    void currentLOD(unsigned int current_lod);

    DBContentLabelGeneratorWidget& widget();

    bool autoLOD() const;
    void autoLOD(bool auto_lod);

    void addLabelDSID(unsigned int ds_id);
    void removeLabelDSID(unsigned int ds_id);
    const std::set<unsigned int>& labelDSIDs() const;
    bool labelWanted(unsigned int ds_id);
    bool labelWanted(std::shared_ptr<Buffer> buffer, unsigned int index);

    bool filterMode3aActive() const;
    void filterMode3aActive(bool filter_active);
    std::string filterMode3aValues() const;
    void filterMode3aValues(const std::string& filter_values);

    bool filterTIActive() const;
    void filterTIActive(bool filter_active);
    std::string filterTIValues() const;
    void filterTIValues(const std::string& filter_values);

    bool filterTAActive() const;
    void filterTAActive(bool filter_active);
    std::string filterTAValues() const;
    void filterTAValues(const std::string& filter_values);

    bool filterModecMinActive() const;
    void filterModecMinActive(bool value);

    float filterModecMinValue() const;
    void filterModecMinValue(float value);

    bool filterModecMaxActive() const;
    void filterModecMaxActive(bool value);

    float filterModecMaxValue() const;
    void filterModecMaxValue(float value);

    bool filterModecNullWanted() const;
    void filterModecNullWanted(bool value);

    LabelDirection labelDirection (unsigned int ds_id);
    void labelDirection (unsigned int ds_id, LabelDirection direction);

protected:
    DBContentManager& dbcont_manager_;

    std::unique_ptr<DBContentLabelGeneratorWidget> widget_;

    bool auto_label_ {true};
    bool auto_lod_ {true};
    unsigned int current_lod_ {3}; // 1, 2, 3

    nlohmann::json label_directions_;

    nlohmann::json label_config_;

    std::set<unsigned int> label_ds_ids_;

    bool filter_mode3a_active_;
    std::string filter_mode3a_values_;
    std::set<unsigned int> filter_m3a_values_set_; // dec
    bool filter_m3a_null_wanted_ {false};

    bool filter_modec_min_active_;
    float filter_modec_min_value_ {0};
    bool filter_modec_max_active_;
    float filter_modec_max_value_ {0};
    bool filter_modec_null_wanted_ {false};

    bool filter_ti_active_;
    std::string filter_ti_values_;
    std::set<std::string> filter_ti_values_set_;
    bool filter_ti_null_wanted_ {false};

    bool filter_ta_active_;
    std::string filter_ta_values_;
    std::set<unsigned int> filter_ta_values_set_; // dec
    bool filter_ta_null_wanted_ {false};

    //std::set<GeometryLeafItemLabels*> item_labels_;

    virtual void checkSubConfigurables();
    bool updateM3AValuesFromStr(const std::string& values);
    bool updateTIValuesFromStr(const std::string& values);
    bool updateTAValuesFromStr(const std::string& values);
};

#endif // DBCONTENTLABELGENERATOR_H
