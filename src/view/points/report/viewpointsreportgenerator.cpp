#include "viewpointsreportgenerator.h"
#include "viewpointsreportgeneratordialog.h"
#include "viewmanager.h"
#include "viewpointswidget.h"
#include "viewpointstablemodel.h"
#include "viewpoint.h"
#include "logger.h"
#include "stringconv.h"
#include "dbobjectmanager.h"
#include "atsdb.h"
#include "global.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include "osgview.h"
#endif

#include "boost/date_time/posix_time/posix_time.hpp"

#include <QCoreApplication>

using namespace std;
using namespace Utils;

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

void ViewPointsReportGenerator::run ()
{
    loginf << "ViewPointsReportGenerator: run";

    boost::posix_time::ptime start_time;
    boost::posix_time::ptime stop_time;
    boost::posix_time::time_duration time_diff;

    start_time = boost::posix_time::microsec_clock::local_time();

    ViewPointsWidget* vp_widget = view_manager_.viewPointsWidget();
    assert (vp_widget);
    ViewPointsTableModel* table_model = vp_widget->tableModel();
    assert (table_model);
    std::map<unsigned int, ViewPoint> view_points = table_model->viewPoints();

    string status_str, elapsed_time_str, remaining_time_str;
    DBObjectManager& obj_man = ATSDB::instance().objectManager();

    unsigned int vp_cnt = 0;
    unsigned int vp_size = view_points.size();
    double ms;
    double ms_per_vp;

#if USE_EXPERIMENTAL_SOURCE == true
    OSGView::instant_display_ = true;
#endif

    for (auto& vp_it : view_points)
    {
        loginf << "ViewPointsReportGenerator: run: setting vp " << vp_it.first;
        view_manager_.setCurrentViewPoint(vp_it.first);

        while (obj_man.loadInProgress() || QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        stop_time = boost::posix_time::microsec_clock::local_time();

        time_diff = stop_time - start_time;
        ms = time_diff.total_milliseconds();
        elapsed_time_str =
            String::timeStringFromDouble(ms / 1000.0, false);

        status_str = "Writing view point "+to_string(vp_cnt+1)+"/"+to_string(vp_size);

        if (vp_cnt && ms > 0)
        {
            ms_per_vp = ms/(double)vp_cnt;

            remaining_time_str = String::timeStringFromDouble((vp_size-vp_cnt) * ms_per_vp / 1000.0, false);

            loginf << "ViewPointsReportGenerator: run: setting vp " << vp_it.first
                   << " done after " << elapsed_time_str << " remaining " << remaining_time_str;
        }
        else
        {
            loginf << "ViewPointsReportGenerator: run: setting vp " << vp_it.first
                   << " done after " << elapsed_time_str;

        }

        dialog_->setElapsedTime(elapsed_time_str);
        dialog_->setProgress(0, vp_size, vp_cnt);
        dialog_->setStatus(status_str);
        dialog_->setRemainingTime(remaining_time_str);

        ++vp_cnt;
    }

    dialog_->setProgress(0, vp_size, vp_size);
    dialog_->setStatus("Writing view points done");
    dialog_->setRemainingTime(String::timeStringFromDouble(0, false));


#if USE_EXPERIMENTAL_SOURCE == true
    OSGView::instant_display_ = false;
#endif

}
