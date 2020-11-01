/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "eval/results/report/pdfgenerator.h"
#include "eval/results/report/pdfgeneratordialog.h"
#include "evaluationmanager.h"
#include "compass.h"
#include "global.h"
#include "dbinterface.h"
#include "logger.h"
#include "files.h"
#include "stringconv.h"
#include "sqliteconnection.h"
#include "mysqlppconnection.h"
#include "latexdocument.h"
#include "latexvisitor.h"
#include "system.h"

#include "eval/results/report/section.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/evaluationresultsgenerator.h"

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

        report_filename_ = "report.tex";

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
        if (!report_path_.size())
        {
            SQLiteConnection* sql_con = dynamic_cast<SQLiteConnection*>(&COMPASS::instance().interface().connection());

            if (sql_con)
            {
                report_path_ = Files::getDirectoryFromPath(sql_con->lastFilename())+"/eval_report_"
                        + Files::getFilenameFromPath(sql_con->lastFilename()) + "/";
            }
            else
            {
                MySQLppConnection* mysql_con =
                        dynamic_cast<MySQLppConnection*>(&COMPASS::instance().interface().connection());
                assert (mysql_con);
                report_path_ = HOME_PATH+"/eval_report_"+mysql_con->usedDatabase() + "/";
            }
            loginf << "PDFGenerator: dialog: report path '" << report_path_ << "'"
                   << " filename '"  << report_filename_ << "'";
        }

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
        doc.title("COMPASS Evaluation Report");

        if (author_.size())
            doc.author(author_);

        if (abstract_.size())
            doc.abstract(abstract_);

        LatexVisitor visitor (doc, false, false, false, wait_on_map_loading_);

        cancel_ = false;
        running_ = true;
        pdf_created_ = false;

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        boost::posix_time::ptime start_time;
        boost::posix_time::ptime stop_time;
        boost::posix_time::time_duration time_diff;

        //boost::posix_time::ptime vp_start_time;

        start_time = boost::posix_time::microsec_clock::local_time();

        assert (eval_man_.hasResults());
        std::shared_ptr<Section> root_section = eval_man_.resultsGenerator().resultsModel().rootItem()->rootSection();

        string status_str, elapsed_time_str, remaining_time_str;

        // create sections
        vector<shared_ptr<Section>> sections;
        sections.push_back(root_section);
        root_section->addSectionsFlat(sections);

        unsigned int num_sections = sections.size();

        unsigned int vp_cnt = 0;
        double ms;
        double ms_per_sec;

#if USE_EXPERIMENTAL_SOURCE == true
        OSGView::instant_display_ = true;
#endif

        for (auto& sec_it : sections)
        {
            while (QCoreApplication::hasPendingEvents())
                QCoreApplication::processEvents();

            if (cancel_)
            {
                loginf << "EvaluationResultsReportPDFGenerator: run: cancel";
                break;
            }

            sec_it->accept(visitor);

            // update status
            stop_time = boost::posix_time::microsec_clock::local_time();

            time_diff = stop_time - start_time;
            ms = time_diff.total_milliseconds();
            elapsed_time_str =
                    String::timeStringFromDouble(ms / 1000.0, false);

            status_str = "Writing section "+to_string(vp_cnt+1)+"/"+to_string(num_sections);

            if (vp_cnt && ms > 0)
            {
                ms_per_sec = ms/(double)vp_cnt;

                remaining_time_str = String::timeStringFromDouble((num_sections-vp_cnt) * ms_per_sec / 1000.0, false);

                loginf << "EvaluationResultsReportPDFGenerator: run: section " << sec_it->heading()
                       << " done after " << elapsed_time_str << " remaining " << remaining_time_str;
            }
            else
            {
                loginf << "EvaluationResultsReportPDFGenerator: run: section " << sec_it->heading()
                       << " done after " << elapsed_time_str;

            }

            dialog_->setElapsedTime(elapsed_time_str);
            dialog_->setProgress(0, num_sections, vp_cnt);
            dialog_->setStatus(status_str);
            dialog_->setRemainingTime(remaining_time_str);

            ++vp_cnt;
        }

        if (cancel_)
        {
            dialog_->setProgress(0, num_sections, 0);
            dialog_->setStatus("Writing section cancelled");
            dialog_->setRemainingTime(String::timeStringFromDouble(0, false));

            while (QCoreApplication::hasPendingEvents())
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        else // proceed
        {
            dialog_->setProgress(0, num_sections, num_sections);
            dialog_->setStatus("Writing sections done");
            dialog_->setRemainingTime(String::timeStringFromDouble(0, false));

            while (QCoreApplication::hasPendingEvents())
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

            doc.write();

            if (run_pdflatex_)
            {
                std::string command_out;
                std::string command = "cd "+report_path_+" && pdflatex --interaction=nonstopmode "+report_filename_
                        +" | awk 'BEGIN{IGNORECASE = 1}/warning|!/,/^$/;'";

                loginf << "ViewPointsReportGenerator: run: running pdflatex";
                dialog_->setStatus("Running pdflatex");
                dialog_->setRemainingTime("");

                //while (QCoreApplication::hasPendingEvents())
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

                logdbg << "ViewPointsReportGenerator: run: cmd '" << command << "'";

                command_out = System::exec(command);

                logdbg << "ViewPointsReportGenerator: run: cmd done";

                // update status
                stop_time = boost::posix_time::microsec_clock::local_time();

                time_diff = stop_time - start_time;
                ms = time_diff.total_milliseconds();
                elapsed_time_str = String::timeStringFromDouble(ms / 1000.0, false);
                dialog_->setElapsedTime(elapsed_time_str);

                while (command_out.find("Rerun to get outlines right") != std::string::npos
                       || command_out.find("Rerun to get cross-references right") != std::string::npos)
                {
                    loginf << "ViewPointsReportGenerator: run: re-running pdflatex";
                    dialog_->setStatus("Re-running pdflatex");

                    //                while (QCoreApplication::hasPendingEvents())
                    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

                    command_out = System::exec(command);

                    time_diff = stop_time - start_time;
                    ms = time_diff.total_milliseconds();
                    elapsed_time_str = String::timeStringFromDouble(ms / 1000.0, false);
                    dialog_->setElapsedTime(elapsed_time_str);

                    logdbg << "ViewPointsReportGenerator: run: re-run done";
                }

                loginf << "ViewPointsReportGenerator: run: result '" << command_out << "'";

                if (!command_out.size()) // no warnings
                {
                    pdf_created_ = true;

                    dialog_->setStatus("Running pdflatex done");

                    if (open_created_pdf_)
                    {
                        std::string fullpath = report_path_+report_filename_;

                        if (String::hasEnding(fullpath, ".tex"))
                        {
                            String::replace(fullpath, ".tex", ".pdf");

                            loginf << "ViewPointsReportGenerator: run: opening '" << fullpath << "'";

                            QDesktopServices::openUrl(QUrl(fullpath.c_str()));
                        }
                        else
                            logerr << "ViewPointsReportGenerator: run: opening not possible since wrong file ending";
                    }
                }
                else // show warnings
                {
                    QMessageBox msgBox;
                    msgBox.setText("PDF Latex failed with warnings:\n\n"+QString(command_out.c_str()));
                    msgBox.exec();
                }
            }
        }
        dialog_->setRunning(false);
        dialog_->close();

        QApplication::restoreOverrideCursor();

        running_ = false;
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
