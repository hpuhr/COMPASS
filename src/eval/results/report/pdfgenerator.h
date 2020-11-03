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

#ifndef EVALUATIONRESULTSREPORTPDFGENERATOR_H
#define EVALUATIONRESULTSREPORTPDFGENERATOR_H

#include "configurable.h"

class EvaluationManager;

namespace EvaluationResultsReport
{

    class PDFGeneratorDialog;

    class PDFGenerator : public Configurable
    {
    public:
        PDFGenerator(const std::string& class_id, const std::string& instance_id,
                     EvaluationManager& eval_manager);

        virtual void generateSubConfigurable(const std::string& class_id,
                                             const std::string& instance_id);

        PDFGeneratorDialog& dialog();

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

        bool runPDFLatex() const;
        void runPDFLatex(bool value);

        bool pdfLatexFound() const;

        bool openCreatedPDF() const;
        void openCreatedPDF(bool value);

        bool waitOnMapLoading() const;
        void waitOnMapLoading(bool value);

        bool includeTargetDetails() const;
        void includeTargetDetails(bool value);

        bool includeTargetTRDetails() const;
        void includeTargetTRDetails(bool value);

    protected:
        EvaluationManager& eval_man_;

        std::unique_ptr<PDFGeneratorDialog> dialog_;

        std::string report_path_;
        std::string report_filename_; // without path

        std::string author_;
        std::string abstract_;

        bool include_target_details_ {false};
        bool include_target_tr_details_ {false};

        bool wait_on_map_loading_ {true};

        bool run_pdflatex_ {true};
        bool pdflatex_found_ {false};

        bool open_created_pdf_ {false};

        bool running_ {false};
        bool cancel_ {false};
        bool show_done_ {true};

        bool pdf_created_ {false};

        virtual void checkSubConfigurables();
    };

}

#endif // EVALUATIONRESULTSREPORTPDFGENERATOR_H
