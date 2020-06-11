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

    void reportPathAndFilename(const std::string& str);

    bool isRunning() const;

    void showDone(bool show_done);

    std::string author() const;
    void author(const std::string& author);

    std::string abstract() const;
    void abstract(const std::string& abstract);

    bool exportAllUnsorted() const;
    void exportAllUnsorted(bool value);

    bool runPDFLatex() const;
    void runPDFLatex(bool value);

    bool pdfLatexFound() const;

    bool openCreatedPDF() const;
    void openCreatedPDF(bool value);

    unsigned int timeBeforeScreenshot() const;
    void timeBeforeScreenshot(unsigned int value_ms);

    bool groupByType() const;
    void groupByType(bool value);

protected:
    ViewManager& view_manager_;

    std::unique_ptr<ViewPointsReportGeneratorDialog> dialog_;

    std::string report_path_;
    std::string report_filename_; // without path

    std::string author_;
    std::string abstract_;

    unsigned int time_before_screenshot_ms_;

    bool group_by_type_ {true};
    bool export_all_unsorted_ {false};
    bool run_pdflatex_ {true};
    bool pdflatex_found_ {false};

    bool open_created_pdf_ {false};

    bool running_ {false};
    bool cancel_ {false};
    bool show_done_ {true};

    bool pdf_created_ {false};

    virtual void checkSubConfigurables();
};

#endif // VIEWPOINTSREPORTGENERATOR_H
