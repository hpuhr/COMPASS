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

#include "task/result/report/reportexporterlatex.h"

#include "task/result/report/section.h"
#include "task/result/report/sectioncontent.h"
#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/sectioncontenttable.h"
#include "task/result/report/sectioncontenttext.h"
#include "task/result/taskresult.h"

#include "latexdocument.h"
#include "latexsection.h"
#include "lateximage.h"
#include "latextable.h"

#include "files.h"
#include "stringconv.h"
#include "util/system.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

namespace ResultReport
{

/**
 */
ReportExporterLatex::ReportExporterLatex(const ReportExport* report_export,
                                         const std::string& export_fn,
                                         const std::string& export_resource_dir,
                                         bool interaction_mode,
                                         bool write_pdf)
:   ReportExporter(report_export, export_fn, export_resource_dir, interaction_mode)
,   write_pdf_    (write_pdf)
{
}

/**
 */
ReportExporterLatex::~ReportExporterLatex()
{
}

/**
 */
Result ReportExporterLatex::initExport_impl(TaskResult& result)
{
    bool pdflatex_found = Utils::System::exec("which pdflatex").size();
    if (write_pdf_ && !pdflatex_found)
        return Result::failed("Cannot generate PDF: pdflatex not installed");

    std::string report_fn = boost::filesystem::path(exportFilename()).stem().string() + ".tex";

    latex_doc_.reset(new LatexDocument(exportResourceDir(), report_fn));

    latex_doc_->title("OpenATS COMPASS " + result.name() + " Report");

    const auto& s = settings();

    if (s.author.size())
        latex_doc_->author(s.author);

    if (s.abstract.size())
        latex_doc_->abstract(s.abstract);

    return Result::succeeded();
}

/**
 */
ResultT<nlohmann::json> ReportExporterLatex::finalizeExport_impl(TaskResult& result)
{
    setStatus("Writing report file");

    latex_doc_->write();

    if (write_pdf_)
    {
        setStatus("Converting to PDF");

        auto res_pdf = writePDF();
        if (!res_pdf.ok())
            return res_pdf;
    }

    return ResultT<nlohmann::json>::succeeded(nlohmann::json());
}

/**
 */
Result ReportExporterLatex::exportSection_impl(Section& section)
{
    auto heading      = section.compoundResultsHeading();
    auto section_name = Utils::String::latexString(heading);

    // ignore if top "Results"
    if (section_name.empty())
        return Result::succeeded();

    LatexSection& latex_section = latex_doc_->getSection(section_name);
    latex_section.label("sec:" + heading);

    latex_sections_[ &section ] = section_name;

    return Result::succeeded();
}

/**
 */
Result ReportExporterLatex::exportFigure_impl(SectionContentFigure& figure)
{
    auto it = latex_sections_.find(figure.parentSection());
    if (it == latex_sections_.end())
        return Result::failed("Content '" + figure.name() + "' obtains no parent section in report"); 

    //obtain image data
    auto images = figure.obtainImages(&exportResourceDir());
    if (!images.ok())
        return images;

    auto& latex_section = latex_doc_->getSection(it->second);

    for (const auto& img : images.result())
        latex_section.addImage(img.path, img.name);

    return Result::succeeded();
}

/**
 */
Result ReportExporterLatex::exportTable_impl(SectionContentTable& table)
{
    auto it = latex_sections_.find(table.parentSection());
    if (it == latex_sections_.end())
        return Result::failed("Content '" + table.name() + "' obtains no parent section in report"); 

    const auto& s = settings();

    auto& latex_section = latex_doc_->getSection(it->second);

    std::vector<std::string> headings = table.headings();
    unsigned int num_cols = headings.size();

    for (unsigned int cnt=0; cnt < num_cols; ++cnt) // latexify headings
        headings[ cnt ] = Utils::String::latexString(headings[ cnt ]);

    assert (num_cols);

    //configure wide table settings
    bool wide_table = false;
    if (s.latex_table_min_cols_wide >= 0 && headings.size() >= (size_t)s.latex_table_min_cols_wide)
        wide_table = true;

    //determine max row count
    //table > settings
    bool has_max_row_override = table.maxRowCount().has_value();

    int max_row_count;
    if (has_max_row_override) // from internal override
        max_row_count = table.maxRowCount().value();
    else // from settings
        max_row_count = s.latex_table_max_rows;

    unsigned int num_rows = table.filteredRowCount();
    std::vector<std::string> row_strings;
    std::string ref;

    // split all too long tables with internal max row override into subtables
    bool split_tables = has_max_row_override; 

    //apply a hard limit to maximum rows when splitting tables
    const int HardLimit = 500;
    max_row_count = split_tables ? std::max(max_row_count, HardLimit) : max_row_count;

    LatexTable*  current_table    = nullptr;
    unsigned int current_rows     = 0;
    unsigned int num_tables       = 0;

    for (unsigned int row=0; row < num_rows; ++row, ++current_rows)
    {
        //create initial table or change table on hitting max row count when in split mode
        if (!current_table || (split_tables && current_rows > (unsigned int)max_row_count))
        {
            std::string table_name_cur = table.name();

            //split mode => keep track of tables
            if (split_tables)
            {
                current_rows = 0;
                ++num_tables;
                table_name_cur += std::to_string(num_tables);
            }

            //create new table
            assert (!latex_section.hasTable(table_name_cur));

            latex_section.addTable(table_name_cur, num_cols, headings, "", false);
            current_table = &latex_section.getTable(table_name_cur);

            current_table->setMaxRowCount(max_row_count);
            current_table->setWideTable(wide_table);
        }

        row_strings = table.sortedRowStrings(row);
        assert (row_strings.size() == num_cols);

        if (wide_table) // truncate texts
        {
            for (unsigned int cnt=0; cnt < num_cols; ++cnt)
            {
                if (cnt > 2 && 
                    s.latex_table_max_col_width >= 0 &&  
                    row_strings[cnt].size() > (size_t)s.latex_table_max_col_width)
                {
                    std::string::size_type space_pos = row_strings[cnt].rfind(' ', (size_t)s.latex_table_max_col_width);
                    std::string::size_type comma_pos = row_strings[cnt].rfind(',', (size_t)s.latex_table_max_col_width);

                    if (space_pos == std::string::npos)
                    {
                        if (comma_pos == std::string::npos)
                        {
                            break; // no 64-bit-or-less substring
                        }
                        else
                        {
                            row_strings[cnt] = row_strings[ cnt ].substr(0, comma_pos) + "...";
                        }
                    }
                    else
                    {
                        row_strings[cnt] = row_strings[ cnt ].substr(0, space_pos) + "...";
                    }
                }
            }
        }

        if (table.hasReference(row)) // \hyperref[sec:marker2]{SecondSection}
        {
            ref = table.reference(row);
            row_strings[0] = "\\hyperref[sec:" + ref + "]{" + row_strings.at(0) + "}";
        }

        for (unsigned int cnt=0; cnt < num_cols; ++cnt)
        {
            if (row_strings[cnt] == "true" || row_strings[cnt] == "Passed")
                row_strings[cnt] = "\\textcolor{darkgreen}{"+row_strings[cnt]+"}";
            else if (row_strings[cnt] == "false" || row_strings[cnt] == "Failed")
                row_strings[cnt] = "\\textcolor{red}{"+row_strings[cnt]+"}";
        }

        //add row to current table
        current_table->addRow(std::move(row_strings));
    }

    return Result::succeeded();
}

/**
 */
Result ReportExporterLatex::exportText_impl(SectionContentText& text)
{
    auto it = latex_sections_.find(text.parentSection());
    if (it == latex_sections_.end())
        return Result::failed("Content '" + text.name() + "' obtains no parent section in report"); 

    auto& latex_section = latex_doc_->getSection(it->second);

    for (const auto& txt_it : text.texts())
        latex_section.addText(txt_it);

    return Result::succeeded();
}

/**
 */
Result ReportExporterLatex::writePDF() const
{
    assert(latex_doc_);

    std::string command_out;
    std::string command = "cd \"" + latex_doc_->path() + "\" && pdflatex --interaction=nonstopmode \"" + latex_doc_->filename()
            + "\" | awk 'BEGIN{IGNORECASE = 1}/warning|!/,/^$/;'";

    loginf << "ReportExporterLatex: writePDF: running pdflatex";

    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    loginf << "ReportExporterLatex: writePDF: cmd '" << command << "'";

    command_out = Utils::System::exec(command);

    logdbg << "ReportExporterLatex: writePDF: cmd done";

    const auto& s = settings();

    unsigned int max_runs = s.latex_pdf_max_reruns;
    unsigned int run_cnt  = 0;

    while (run_cnt < max_runs || 
           (command_out.find("Rerun to get outlines right"        ) != std::string::npos) || 
           (command_out.find("Rerun to get cross-references right") != std::string::npos))
    {
        loginf << "ReportExporterLatex: writePDF: re-running pdflatex";
        
        command_out = Utils::System::exec(command);

        logdbg << "ReportExporterLatex: writePDF: re-run done";

        ++run_cnt;
    }

    loginf << "ReportExporterLatex: writePDF: result '" << command_out << "'";

    if (command_out.size())
        return Result::failed("PDF Latex failed with warnings:\n\n" + std::string(command_out.c_str()));

    if (settings().open_created_file && hasInteraction())
    {
        std::string fullpath = exportResourceDir() + "/" + exportFilename();
        if (Utils::String::hasEnding(fullpath, ".tex"))
            Utils::String::replace(fullpath, ".tex", ".pdf");

        loginf << "ReportExporterLatex: writePDF: opening '" << fullpath << "'";

        if (!QDesktopServices::openUrl(QUrl(fullpath.c_str())))
            logerr << "ReportExporterLatex: writePDF: could not open created file";
    }

    return Result::succeeded();
}

}
