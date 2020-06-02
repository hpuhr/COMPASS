#ifndef VIEWPOINTSREPORTGENERATOR_H
#define VIEWPOINTSREPORTGENERATOR_H

#include "configurable.h"

class ViewManager;

class ViewPointsReportGenerator : public Configurable
{
public:
    ViewPointsReportGenerator(const std::string& class_id, const std::string& instance_id,
                              ViewManager& view_manager);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

protected:
    ViewManager& view_manager_;

    virtual void checkSubConfigurables();
};

#endif // VIEWPOINTSREPORTGENERATOR_H
