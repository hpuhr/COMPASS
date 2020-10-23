#include "eval/results/report/pdfgenerator.h"
#include "eval/results/report/pdfgeneratordialog.h"
#include "evaluationmanager.h"
#include "atsdb.h"
#include "global.h"
#include "dbinterface.h"
#include "logger.h"
#include "files.h"
#include "stringconv.h"
#include "sqliteconnection.h"
#include "mysqlppconnection.h"
#include "latexdocument.h"
#include "system.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include "osgview.h"
#endif

#include <QCoreApplication>
#include <QApplication>
#include <QMessageBox>
#include <QUrl>
#include <QDesktopServices>

#include "boost/date_time/posix_time/posix_time.hpp"

using namespace std;
using namespace Utils;

namespace EvaluationResultsReport
{

    PDFGenerator::PDFGenerator(const std::string& class_id, const std::string& instance_id,
                               EvaluationManager& eval_manager)
        : Configurable(class_id, instance_id, &eval_manager), eval_man_(eval_manager)
    {
        registerParameter("author", &author_, "");
        registerParameter("abstract", &abstract_, "");

        SQLiteConnection* sql_con = dynamic_cast<SQLiteConnection*>(&ATSDB::instance().interface().connection());

        if (sql_con)
        {
            report_path_ = Files::getDirectoryFromPath(sql_con->lastFilename())+"/eval_report_"
                    + Files::getFilenameFromPath(sql_con->lastFilename()) + "/";
        }
        else
        {
            MySQLppConnection* mysql_con = dynamic_cast<MySQLppConnection*>(&ATSDB::instance().interface().connection());
            assert (mysql_con);
            report_path_ = HOME_PATH+"/eval_report_"+mysql_con->usedDatabase() + "/";
        }

        report_filename_ = "report.tex";

        loginf << "EvaluationResultsReportPDFGenerator: constructor: report path '" << report_path_ << "'"
               << " filename '"  << report_filename_ << "'";

        registerParameter("wait_on_map_loading", &wait_on_map_loading_, true);

        registerParameter("run_pdflatex", &run_pdflatex_, true);
        registerParameter("open_created_pdf", &open_created_pdf_, false);

        pdflatex_found_ = System::exec("which pdflatex").size(); // empty if none

        if (!pdflatex_found_)
        {
            run_pdflatex_ = false;
            open_created_pdf_ = false;
        }
    }

    void PDFGenerator::generateSubConfigurable(const std::string& class_id,
                                               const std::string& instance_id)
    {
        throw std::runtime_error("PDFGenerator: generateSubConfigurable: unknown class_id " +
                                 class_id);
    }

    void PDFGenerator::checkSubConfigurables()
    {
        // move along sir
    }

    PDFGeneratorDialog& PDFGenerator::dialog()
    {
        if (!dialog_)
            dialog_.reset(new PDFGeneratorDialog(*this));

        return *dialog_;
    }

    void PDFGenerator::run ()
    {
        loginf << "EvaluationResultsReportPDFGenerator: run";

        assert (dialog_);
        dialog_->setRunning(true);

        LatexDocument doc (report_path_, report_filename_);
        doc.title("ATSDB View Points Report");

        if (author_.size())
            doc.author(author_);

        if (abstract_.size())
            doc.abstract(abstract_);

        //LatexVisitor visitor (doc, group_by_type_, add_overview_table_, add_overview_screenshot_, wait_on_map_loading_);

        cancel_ = false;
        running_ = true;
        pdf_created_ = false;

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        boost::posix_time::ptime start_time;
        boost::posix_time::ptime stop_time;
        boost::posix_time::time_duration time_diff;

    }

    void PDFGenerator::cancel ()
    {
        loginf << "PDFGenerator: cancel";

        cancel_ = true;

        if (!running_)
            dialog_->close();
    }

    std::string PDFGenerator::reportPath() const
    {
        return report_path_;
    }

    void PDFGenerator::reportPath(const std::string& path)
    {
        loginf << "PDFGenerator: reportPath: '" << path << "'";
        report_path_ = path;

        if (dialog_)
            dialog_->updateFileInfo();
    }

    std::string PDFGenerator::reportFilename() const
    {
        return report_filename_;
    }

    void PDFGenerator::reportFilename(const std::string& filename)
    {
        loginf << "PDFGenerator: reportFilename: '" << filename << "'";
        report_filename_ = filename;

        if (dialog_)
            dialog_->updateFileInfo();
    }

    void PDFGenerator::reportPathAndFilename(const std::string& str)
    {
        report_path_ = Files::getDirectoryFromPath(str) + "/";
        report_filename_ = Files::getFilenameFromPath(str);

        loginf << "PDFGenerator: reportPathAndFilename: path '" << report_path_
               << "' filename '" << report_filename_ << "'";

        if (dialog_)
            dialog_->updateFileInfo();
    }

    bool PDFGenerator::isRunning() const
    {
        return running_;
    }

    void PDFGenerator::showDone(bool show_done)
    {
        show_done_ = show_done;
    }

    std::string PDFGenerator::author() const
    {
        return author_;
    }

    void PDFGenerator::author(const std::string& author)
    {
        author_ = author;
    }

    std::string PDFGenerator::abstract() const
    {
        return abstract_;
    }

    void PDFGenerator::abstract(const std::string& abstract)
    {
        abstract_ = abstract;
    }

    bool PDFGenerator::runPDFLatex() const
    {
        return run_pdflatex_;
    }

    void PDFGenerator::runPDFLatex(bool value)
    {
        run_pdflatex_ = value;
    }

    bool PDFGenerator::pdfLatexFound() const
    {
        return pdflatex_found_;
    }

    bool PDFGenerator::openCreatedPDF() const
    {
        return open_created_pdf_;
    }

    void PDFGenerator::openCreatedPDF(bool value)
    {
        open_created_pdf_ = value;
    }

    bool PDFGenerator::waitOnMapLoading() const
    {
        return wait_on_map_loading_;
    }

    void PDFGenerator::waitOnMapLoading(bool value)
    {
        wait_on_map_loading_ = value;
    }

}
