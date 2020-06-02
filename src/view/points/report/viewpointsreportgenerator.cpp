#include "viewpointsreportgenerator.h"
#include "viewpointsreportgeneratordialog.h"
#include "viewmanager.h"

ViewPointsReportGenerator::ViewPointsReportGenerator(const std::string& class_id, const std::string& instance_id,
                                                     ViewManager& view_manager)
    : Configurable(class_id, instance_id, &view_manager), view_manager_(view_manager)
{

}


void ViewPointsReportGenerator::generateSubConfigurable(const std::string& class_id,
                                                        const std::string& instance_id)
{
    throw std::runtime_error("ViewPointsReportGenerator: generateSubConfigurable: unknown class_id " +
                             class_id);
}

void ViewPointsReportGenerator::checkSubConfigurables()
{
    // move along sir
}


ViewPointsReportGeneratorDialog& ViewPointsReportGenerator::dialog()
{
    if (!dialog_)
        dialog_.reset(new ViewPointsReportGeneratorDialog(*this));

    return *dialog_;
}
