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

protected:
    DBContentManager& dbcont_manager_;

    std::unique_ptr<DBContentLabelGeneratorWidget> widget_;

    bool auto_label_ {true};
    bool auto_lod_ {true};
    unsigned int current_lod_ {3}; // 1, 2, 3

    nlohmann::json label_config_;

    std::set<unsigned int> label_ds_ids_;

    //std::set<GeometryLeafItemLabels*> item_labels_;

    virtual void checkSubConfigurables();
};

#endif // DBCONTENTLABELGENERATOR_H
