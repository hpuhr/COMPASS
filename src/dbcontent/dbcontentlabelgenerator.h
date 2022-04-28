#ifndef DBCONTENTLABELGENERATOR_H
#define DBCONTENTLABELGENERATOR_H

#include "configurable.h"
#include "json.hpp"

#include <QObject>

class DBContentManager;

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

protected:

    DBContentManager& dbcont_manager_;

    bool auto_label_ {true};

    nlohmann::json label_config_;

    virtual void checkSubConfigurables();
};

#endif // DBCONTENTLABELGENERATOR_H
