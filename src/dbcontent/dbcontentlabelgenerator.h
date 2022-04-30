#ifndef DBCONTENTLABELGENERATOR_H
#define DBCONTENTLABELGENERATOR_H

#include "configurable.h"
#include "json.hpp"

#include <QObject>

#include <set>

class DBContentManager;
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

    void registerLeafItemLabel (GeometryLeafItemLabels& item_label);
    void unregisterLeafItemLabel (GeometryLeafItemLabels& item_label);

    unsigned int currentLOD() const;
    void currentLOD(unsigned int current_lod);

protected:

    DBContentManager& dbcont_manager_;

    bool auto_label_ {true};
    bool auto_lod_ {true};
    unsigned int current_lod_ {3}; // 1, 2, 3

    nlohmann::json label_config_;

    std::set<GeometryLeafItemLabels*> item_labels_;

    virtual void checkSubConfigurables();
};

#endif // DBCONTENTLABELGENERATOR_H
