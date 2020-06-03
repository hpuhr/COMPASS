#ifndef VIEWPOINTSREPORTGENERATOR_H
#define VIEWPOINTSREPORTGENERATOR_H

#include "configurable.h"

class ViewManager;
class ViewPointsReportGeneratorDialog;

class ViewPointsReportGenerator : public Configurable
{
public:
    ViewPointsReportGenerator(const std::string& class_id, const std::string& instance_id,
                              ViewManager& view_manager);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    ViewPointsReportGeneratorDialog& dialog();

    void run ();
    void cancel ();

    std::string reportPath() const;
    void reportPath(const std::string& path);

    std::string reportFilename() const;
    void reportFilename(const std::string& filename);

protected:
    ViewManager& view_manager_;

    std::unique_ptr<ViewPointsReportGeneratorDialog> dialog_;

    std::string report_path_;
    std::string report_filename_; // without path

    bool running_ {false};
    bool cancel_ {false};

    virtual void checkSubConfigurables();
};

#endif // VIEWPOINTSREPORTGENERATOR_H
