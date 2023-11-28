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

#include "latexvisitor.h"
#include "latexdocument.h"
#include "latexsection.h"
#include "latextable.h"
//#include "lateximage.h"
#include "viewpoint.h"
#include "listboxview.h"
#include "listboxviewdatawidget.h"
#include "allbuffertablewidget.h"
#include "histogramview.h"
#include "histogramviewdatawidget.h"
#include "scatterplotview.h"
#include "scatterplotviewdatawidget.h"
#include "logger.h"
#include "stringconv.h"
#include "json.h"
#include "files.h"
#include "eval/results/single.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttable.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontentfigure.h"
#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "viewmanager.h"
#include "files.h"
#include "ViewerWidget.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include "osgview.h"
#include "osgviewdatawidget.h"
#endif

#include <QCoreApplication>
#include <QApplication>

#include <sstream>

#include <QPixmap>

using namespace std;
using namespace Utils;

LatexVisitor::LatexVisitor(LatexDocument& report, bool group_by_type, bool add_overview_table,
                           bool add_overview_screenshot, bool include_target_details,
                           bool include_target_tr_details, unsigned int max_table_col_width, bool wait_on_map_loading)
    : report_(report), group_by_type_(group_by_type), add_overview_table_(add_overview_table),
      add_overview_screenshot_(add_overview_screenshot), include_target_details_(include_target_details),
      include_target_tr_details_(include_target_tr_details), max_table_col_width_(max_table_col_width),
      wait_on_map_loading_(wait_on_map_loading)
{
}


void LatexVisitor::visit(const ViewPoint* e)
{
    assert (e);

    loginf << "LatexVisitor: visit: ViewPoint id " << e->id();

    const nlohmann::json& j_data = e->data();

    assert (j_data.contains(VP_NAME_KEY));
    string name = String::latexString(j_data.at(VP_NAME_KEY));

    assert (j_data.contains(VP_TYPE_KEY));
    string type = String::latexString(j_data.at(VP_TYPE_KEY));

    assert (j_data.contains(VP_STATUS_KEY));
    string status = j_data.at(VP_STATUS_KEY);

    string comment;

    if (j_data.contains(VP_COMMENT_KEY))
        comment = String::latexString(j_data.at(VP_COMMENT_KEY));

    // overview table
    if (add_overview_table_)
        report_.getSection("Overview"); // add before other section

    if (group_by_type_)
        current_section_name_ = "View Points:"+type+":ID "+to_string(e->id())+" "+name;
    else
        current_section_name_ = "View Points:ID "+to_string(e->id())+" "+name;

    LatexSection& sec = report_.getSection(current_section_name_);

    // set label
    std::string current_section_label = "sec:view-point-"+to_string(e->id());
    //boost::replace_all(current_section_label, " ", "-");
    sec.label(current_section_label);

    // details table

    sec.addTable("Info", 2, {"Key","Value"}, "| l | X |");
    LatexTable& info_table = sec.getTable("Info");

    info_table.addRow({VP_ID_KEY, to_string(e->id())});
    info_table.addRow({VP_NAME_KEY, j_data.at(VP_NAME_KEY)});
    info_table.addRow({VP_TYPE_KEY, j_data.at(VP_TYPE_KEY)});
    info_table.addRow({VP_STATUS_KEY, status});

    for (auto& j_it : j_data.items())
    {
        if (!j_it.value().is_primitive())
            continue;

        if (j_it.key() == VP_ID_KEY || j_it.key() == VP_NAME_KEY
                || j_it.key() == VP_TYPE_KEY || j_it.key() == VP_STATUS_KEY)
            continue;

        info_table.addRow({j_it.key(), JSON::toString(j_it.value())});
    }

    // write overview table entry

    if (add_overview_table_)
    {
        LatexSection& overview_sec = report_.getSection("Overview");

        if (!overview_sec.hasTable("ViewPoints Overview"))
            overview_sec.addTable("ViewPoints Overview", 6,
                                  {VP_ID_KEY , VP_NAME_KEY, VP_TYPE_KEY, VP_STATUS_KEY, VP_COMMENT_KEY, ""},
                                  "| l | l | l | l | X | l |", false);

        LatexTable& overview_table = overview_sec.getTable("ViewPoints Overview");

        overview_table.addRow({
                                  to_string(e->id()),
                                  name,
                                  type,
                                  status,
                                  comment,
                                  R"(\hyperref[)"+current_section_label+"]{Link}"
                              });
    }
}

void LatexVisitor::visit(const EvaluationResultsReport::Section* e)
{
    assert (e);
    loginf << "LatexVisitor: visit: EvaluationResultsReportSection " << e->heading();

    current_section_name_ = String::latexString(e->compoundResultsHeading()); // slightly hacky, remove "Results" from top

    // ignore if top "Results"
    if (current_section_name_ == "")
        return;

    LatexSection& section = report_.getSection(current_section_name_);
    section.label("sec:"+e->compoundResultsHeading());

    for (const auto& cont_it : e->content())
        cont_it->accept(*this);
}

void LatexVisitor::visit(const EvaluationResultsReport::SectionContentTable* e)
{
    assert (e);
    loginf << "LatexVisitor: visit: EvaluationResultsReportSectionContentTable " << e->name();

    if (!include_target_tr_details_ && e->name() == EvaluationRequirementResult::Single::tr_details_table_name_)
        return; // do not generate this table

    LatexSection& section = report_.getSection(current_section_name_);

    string table_name = e->name();
    assert (!section.hasTable(table_name));

    vector<string> headings = e->headings();
    unsigned int num_cols = headings.size();

    for (unsigned int cnt=0; cnt < num_cols; ++cnt) // latexify headings
        headings[cnt] = String::latexString(headings[cnt]);

    assert (num_cols);

    //    const std::string& name, unsigned int num_columns,
    //                                 std::vector<std::string> headings, std::string heading_alignment,
    //                                 bool convert_to_latex
    //    if (!overview_sec.hasTable("ViewPoints Overview"))
    //        overview_sec.addTable("ViewPoints Overview", 6, {"id","name","type", "status", "comment", ""},
    //                              "| l | l | l | l | X | l |", false);

    section.addTable(table_name, num_cols, headings, "", false);
    LatexTable& table = section.getTable(table_name);

    bool wide_table = false;

    if (headings.size() >= 9)
    {
        table.setWideTable(true);
        wide_table = true;
    }

    unsigned int num_rows = e->filteredRowCount();
    vector<string> row_strings;
    string ref;

    for (unsigned int row=0; row < num_rows; ++row)
    {
        row_strings = e->sortedRowStrings(row);
        assert (row_strings.size() == num_cols);

        if (wide_table) // truncate texts
        {
            for (unsigned int cnt=0; cnt < num_cols; ++cnt)
            {
                if (cnt > 2 && row_strings[cnt].size() > max_table_col_width_)
                {
                    std::string::size_type space_pos = row_strings[cnt].rfind(' ', max_table_col_width_);
                    std::string::size_type comma_pos = row_strings[cnt].rfind(',', max_table_col_width_);

                    if (space_pos == std::string::npos)
                    {
                        if (comma_pos == std::string::npos)
                        {
                            break; // no 64-bit-or-less substring
                        }
                        else
                        {
                            row_strings[cnt] = row_strings[cnt].substr(0, comma_pos)+"...";
                        }
                    }
                    else
                    {
                        row_strings[cnt] = row_strings[cnt].substr(0, space_pos)+"...";
                    }
                }
            }
        }

        if (e->hasReference(row)) // \hyperref[sec:marker2]{SecondSection}
        {
            ref = e->reference(row);
            if (!include_target_details_ && (ref.rfind("Targets", 0) == 0)) // reference to details
                ; // do not do hyperref
            else
            {


                row_strings[0] = "\\hyperref[sec:"+ref+"]{"+row_strings.at(0)+"}";
            }
        }

        for (unsigned int cnt=0; cnt < num_cols; ++cnt)
        {
            if (row_strings[cnt] == "true" || row_strings[cnt] == "Passed")
                row_strings[cnt] = "\\textcolor{darkgreen}{"+row_strings[cnt]+"}";
            else if (row_strings[cnt] == "false" || row_strings[cnt] == "Failed")
                row_strings[cnt] = "\\textcolor{red}{"+row_strings[cnt]+"}";
        }

        table.addRow(move(row_strings));
    }

}

void LatexVisitor::visit(const EvaluationResultsReport::SectionContentText* e)
{
    assert (e);
    loginf << "LatexVisitor: visit: EvaluationResultsReportSectionContentText" << e->name();

    LatexSection& section = report_.getSection(current_section_name_);

    for (const auto& txt_it : e->texts())
        section.addText(txt_it);
}

void LatexVisitor::visit(const EvaluationResultsReport::SectionContentFigure* e)
{
    assert (e);
    loginf << "LatexVisitor: visit: EvaluationResultsReportSectionContentFigure " << e->name();

#if USE_EXPERIMENTAL_SOURCE == true

    ignore_listbox_views_ = true;

    DBContentManager& obj_man = COMPASS::instance().dbContentManager();
    ViewManager& view_man = COMPASS::instance().viewManager();

    while (QCoreApplication::hasPendingEvents())
        QCoreApplication::processEvents();

    e->view();

    while (obj_man.loadInProgress() || QCoreApplication::hasPendingEvents())
        QCoreApplication::processEvents();

    image_prefix_ = e->getSubPath()+e->name();

    loginf << "LatexVisitor: visit: EvaluationResultsReportSectionContentFigure " << e->name()
           << " prefix " << image_prefix_;

    for (auto& view_it : view_man.getViews())
        view_it.second->accept(*this);

    ignore_listbox_views_ = false;
#endif
}

void LatexVisitor::visit(ListBoxView* e)
{
    assert (e);

    loginf << "LatexVisitor: visit: ListBoxView " << e->instanceId();

    if (ignore_listbox_views_)
        return;

    AllBufferTableWidget* allbuf = e->getDataWidget()->getAllBufferTableWidget();
    assert (allbuf);

    std::vector<std::vector<std::string>> data = allbuf->getSelectedText();

    if (data.size())
    {
        LatexSection& sec = report_.getSection(current_section_name_);

        sec.addTable(e->instanceId(), data.at(0).size(), data.at(0));
        LatexTable& table = sec.getTable(e->instanceId());
        table.setWideTable(true);

        data.erase(data.begin()); // remove first header row#

        for (auto& row_it : data)
            table.addRow(row_it);
    }
    else
    {
        data = allbuf->getText();

        if (data.size())
        {
            LatexSection& sec = report_.getSection(current_section_name_);

            sec.addTable(e->instanceId(), data.at(0).size(), data.at(0));
            LatexTable& table = sec.getTable(e->instanceId());
            table.setWideTable(true);

            data.erase(data.begin()); // remove first header row#

            for (auto& row_it : data)
                table.addRow(row_it);
        }
    }
}

void LatexVisitor::visit(HistogramView* e)
{
    assert (e);

    loginf << "LatexVisitor: visit: HistogramView " << e->instanceId();

    std::string screenshot_path = report_.path()+"/screenshots";

    loginf << "LatexVisitor: visit: path '" << screenshot_path << "'";

    if (!screenshot_folder_created_)
    {
        bool ret = Files::createMissingDirectories(screenshot_path);

        if (!ret)
            throw runtime_error("LatexVisitor: visit: HistogramView: unable to create directories for '"
                                +screenshot_path+"'");

        screenshot_folder_created_ = true;
    }

    e->showInTabWidget();

    HistogramViewDataWidget* data_widget = e->getDataWidget();
    assert (data_widget);

    if (!data_widget->showsData())
        return;

    // normal screenshot
    QPixmap pmap = data_widget->renderPixmap();;

    QImage screenshot = pmap.toImage();

    std::string image_path = screenshot_path+"/"+image_prefix_+"_"+e->instanceId()+".jpg";
    assert (!screenshot.isNull());

    loginf << "LatexVisitor: visit: saving screenshot as '" << image_path << "'";
    bool ret = Files::createMissingDirectories(Files::getDirectoryFromPath(image_path));

    if (!ret)
        throw runtime_error("LatexVisitor: visit: HistogramView: unable to create directories for '"
                            +image_path+"'");

    ret = screenshot.save(image_path.c_str(), "JPG"); // , 50
    assert (ret);

    LatexSection& sec = report_.getSection(current_section_name_);

    // add normal screenshot after overview
    sec.addImage(image_path, e->instanceId());
}

#if USE_EXPERIMENTAL_SOURCE == true
void LatexVisitor::visit(OSGView* e)
{
    assert (e);
    loginf << "LatexVisitor: visit: OSGView " << e->instanceId();

    std::string screenshot_path = report_.path()+"/screenshots";

    loginf << "LatexVisitor: visit: path '" << screenshot_path << "'";

    if (!screenshot_folder_created_)
    {
        bool ret = Files::createMissingDirectories(screenshot_path);

        if (!ret)
            throw runtime_error("LatexVisitor: visit: OSGView: unable to create directories for '"
                                +screenshot_path+"'");

        screenshot_folder_created_ = true;
    }

    e->showInTabWidget();

    OSGViewDataWidget* data_widget = e->getDataWidget();
    assert (data_widget);

    if (wait_on_map_loading_)
        data_widget->waitUntilMapLoaded();

    //data_widget->clearMouseCoordinates();

    // normal screenshot

    QImage screenshot = data_widget->osgViewerWidget()->grabFrameBuffer();

    std::string image_path = screenshot_path+"/"+image_prefix_+"_"+e->instanceId()+".jpg";
    assert (!screenshot.isNull());

    loginf << "LatexVisitor: visit: saving screenshot as '" << image_path << "'";
    bool ret = Files::createMissingDirectories(Files::getDirectoryFromPath(image_path));

    if (!ret)
        throw runtime_error("LatexVisitor: visit: OSGView: unable to create directories for '"
                            +image_path+"'");

    ret = screenshot.save(image_path.c_str(), "JPG"); // , 50
    assert (ret);

    LatexSection& sec = report_.getSection(current_section_name_);

    if (add_overview_screenshot_) // overview screenshot
    {
        data_widget->zoomToDataSlot(); // TODO wrong
        data_widget->addDataMarker();

        QImage overview_screenshot = data_widget->osgViewerWidget()->grabFrameBuffer();

        data_widget->removeDataMarker();

        std::string overview_image_path = screenshot_path+"/"+image_prefix_+"_overview_"+e->instanceId()+".jpg";
        assert (!overview_screenshot.isNull());

        loginf << "LatexVisitor: visit: saving overview screenshot as '" << overview_image_path << "'";
        ret = overview_screenshot.save(overview_image_path.c_str(), "JPG"); // , 50
        assert (ret);

        sec.addImage(overview_image_path, e->instanceId()+" Overview");
    }

    // add normal screenshot after overview
    sec.addImage(image_path, e->instanceId());
}
#endif

void LatexVisitor::visit(ScatterPlotView* e)
{
    assert (e);

    loginf << "LatexVisitor: visit: ScatterPlotView " << e->instanceId();

    std::string screenshot_path = report_.path()+"/screenshots";

    loginf << "LatexVisitor: visit: path '" << screenshot_path << "'";

    if (!screenshot_folder_created_)
    {
        bool ret = Files::createMissingDirectories(screenshot_path);

        if (!ret)
            throw runtime_error("LatexVisitor: visit: ScatterPlotView: unable to create directories for '"
                                +screenshot_path+"'");

        screenshot_folder_created_ = true;
    }

    e->showInTabWidget();

    ScatterPlotViewDataWidget* data_widget = e->getDataWidget();
    assert (data_widget);

    if (!data_widget->showsData())
        return;

    // normal screenshot
    QPixmap pmap = data_widget->renderPixmap();

    QImage screenshot = pmap.toImage();

    std::string image_path = screenshot_path+"/"+image_prefix_+"_"+e->instanceId()+".jpg";
    assert (!screenshot.isNull());

    loginf << "LatexVisitor: visit: saving screenshot as '" << image_path << "'";
    bool ret = Files::createMissingDirectories(Files::getDirectoryFromPath(image_path));

    if (!ret)
        throw runtime_error("LatexVisitor: visit: ScatterPlotView: unable to create directories for '"
                            +image_path+"'");

    ret = screenshot.save(image_path.c_str(), "JPG"); // , 50
    assert (ret);

    LatexSection& sec = report_.getSection(current_section_name_);

    // add normal screenshot after overview
    sec.addImage(image_path, e->instanceId());
}

void LatexVisitor::imagePrefix(const std::string& image_prefix)
{
    image_prefix_ = image_prefix;
}

