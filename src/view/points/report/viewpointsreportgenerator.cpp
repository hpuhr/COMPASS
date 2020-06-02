#include "viewpointsreportgenerator.h"
#include "viewmanager.h"

ViewPointsReportGenerator::ViewPointsReportGenerator(const std::string& class_id, const std::string& instance_id,
                                                     ViewManager& view_manager)
    : Configurable(class_id, instance_id, &view_manager), view_manager_(view_manager)
{

}


void ViewPointsReportGenerator::generateSubConfigurable(const std::string& class_id,
                                                        const std::string& instance_id)
{

}

void ViewPointsReportGenerator::checkSubConfigurables()
{

}
