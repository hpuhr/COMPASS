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

#pragma once

class EvaluationCalculator;
class LatexDocument;

#include <string>
#include <memory>

namespace EvaluationResultsReport
{
    class PDFGeneratorDialog;

    class PDFGenerator
    {
    public:
        PDFGenerator(EvaluationCalculator& calculator);

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

        bool pdfLatexFound() const;

    protected:
        EvaluationCalculator& calculator_;

        std::unique_ptr<PDFGeneratorDialog> dialog_;

        std::string report_path_;
        std::string report_filename_; // without path

        bool pdflatex_found_ {false};

        bool running_ {false};
        bool cancel_ {false};
        bool show_done_ {true};

        bool pdf_created_ {false};
    };

}
