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
#include "dbinterface.h"
#include "sqliteconnection.h"
#include "mysqlppconnection.h"
#include "files.h"
#include "latexdocument.h"
#include "latexvisitor.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include "osgview.h"
#endif

#include "boost/date_time/posix_time/posix_time.hpp"

#include <QCoreApplication>
#include <QMessageBox>

using namespace std;
using namespace Utils;

ViewPointsReportGenerator::ViewPointsReportGenerator(const std::string& class_id, const std::string& instance_id,
                                                     ViewManager& view_manager)
    : Configurable(class_id, instance_id, &view_manager), view_manager_(view_manager)
{

    SQLiteConnection* sql_con = dynamic_cast<SQLiteConnection*>(&ATSDB::instance().interface().connection());

    if (sql_con)
    {
        report_path_ = Files::getDirectoryFromPath(sql_con->lastFilename())+"/report_"
                + Files::getFilenameFromPath(sql_con->lastFilename()) + "/";
    }
    else
    {
        MySQLppConnection* mysql_con = dynamic_cast<MySQLppConnection*>(&ATSDB::instance().interface().connection());
        assert (mysql_con);
        report_path_ = HOME_PATH+"/report_"+mysql_con->usedDatabase() + "/";
    }

    report_filename_ = "report.tex";


    loginf << "ViewPointsReportGenerator: constructor: report path '" << report_path_ << "'"
           << " filename '"  << report_filename_ << "'";
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

    LatexDocument doc (report_path_, report_filename_);
    doc.title("ATSDB View Points Report");

    LatexVisitor visitor (doc);

    cancel_ = false;
    running_ = true;

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
        if (cancel_)
        {
            loginf << "ViewPointsReportGenerator: run: cancel";
            break;
        }

        loginf << "ViewPointsReportGenerator: run: setting vp " << vp_it.first;
        view_manager_.setCurrentViewPoint(vp_it.first);

        while (obj_man.loadInProgress() || QCoreApplication::hasPendingEvents())
            QCoreApplication::processEvents();

        // do stuff
        vp_it.second.accept(visitor);


        // update status
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

    if (cancel_)
    {
        dialog_->setProgress(0, vp_size, 0);
        dialog_->setStatus("Writing view points cancelled");
        dialog_->setRemainingTime(String::timeStringFromDouble(0, false));
    }
    else // proceed
    {
        dialog_->setProgress(0, vp_size, vp_size);
        dialog_->setStatus("Writing view points done");
        dialog_->setRemainingTime(String::timeStringFromDouble(0, false));

        doc.write();
    }

    dialog_->close();

#if USE_EXPERIMENTAL_SOURCE == true
    OSGView::instant_display_ = false;
#endif

    QMessageBox msgBox;
    if (cancel_)
        msgBox.setText("Export View Points as PDF Cancelled");
    else
        msgBox.setText("Export View Points as PDF Done");

    msgBox.exec();

    running_ = false;
}

void ViewPointsReportGenerator::cancel ()
{
    loginf << "ViewPointsReportGenerator: cancel";

    cancel_ = true;

    if (!running_)
        dialog_->close();
}

std::string ViewPointsReportGenerator::reportPath() const
{
    return report_path_;
}

void ViewPointsReportGenerator::reportPath(const std::string& path)
{
    loginf << "ViewPointsReportGenerator: reportPath: '" << path << "'";
    report_path_ = path;
}

std::string ViewPointsReportGenerator::reportFilename() const
{
    return report_filename_;
}

void ViewPointsReportGenerator::reportFilename(const std::string& filename)
{
    loginf << "ViewPointsReportGenerator: reportFilename: '" << filename << "'";
    report_filename_ = filename;
}
