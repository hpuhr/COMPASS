#include "latexvisitor.h"
#include "latexdocument.h"
#include "latexsection.h"
#include "latextable.h"
#include "lateximage.h"
#include "viewpoint.h"
#include "listboxview.h"
#include "listboxviewdatawidget.h"
#include "allbuffertablewidget.h"
#include "logger.h"
#include "stringconv.h"
#include "json.h"
#include "files.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttable.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontentfigure.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include "osgview.h"
#include "osgviewdatawidget.h"
#endif

#include <sstream>

#include <QPixmap>

using namespace std;
using namespace Utils;

LatexVisitor::LatexVisitor(LatexDocument& report, bool group_by_type, bool add_overview_table,
                           bool add_overview_screenshot, bool wait_on_map_loading)
    : report_(report), group_by_type_(group_by_type), add_overview_table_(add_overview_table),
      add_overview_screenshot_(add_overview_screenshot), wait_on_map_loading_(wait_on_map_loading)
{
}


void LatexVisitor::visit(const ViewPoint* e)
{
    assert (e);

    loginf << "LatexVisitor: visit: ViewPoint id " << e->id();

    const nlohmann::json& j_data = e->data();

    assert (j_data.contains("name"));
    string name = String::latexString(j_data.at("name"));

    assert (j_data.contains("type"));
    string type = String::latexString(j_data.at("type"));

    assert (j_data.contains("status"));
    string status = j_data.at("status");

    string comment;

    if (j_data.contains("comment"))
        comment = String::latexString(j_data.at("comment"));

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

    info_table.addRow({"id", to_string(e->id())});
    info_table.addRow({"name", j_data.at("name")});
    info_table.addRow({"type", j_data.at("type")});
    info_table.addRow({"status", status});

    for (auto& j_it : j_data.items())
    {
        if (!j_it.value().is_primitive())
            continue;

        if (j_it.key() == "id" || j_it.key() == "name" || j_it.key() == "type" || j_it.key() == "status")
            continue;

        info_table.addRow({j_it.key(), JSON::toString(j_it.value())});
    }

    // write overview table entry

    if (add_overview_table_)
    {
        LatexSection& overview_sec = report_.getSection("Overview");

        if (!overview_sec.hasTable("ViewPoints Overview"))
            overview_sec.addTable("ViewPoints Overview", 6, {"id","name","type", "status", "comment", ""},
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

    current_section_name_ = e->compoundResultsHeading(); // slightly hacky, remove "Results" from top

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

    if (headings.size() >= 9)
        table.setWideTable(true);

    unsigned int num_rows = e->rowCount();
    vector<string> row_strings;

    for (unsigned int row=0; row < num_rows; ++row)
    {
        row_strings = e->sortedRowStrings(row);
        assert (row_strings.size() == num_cols);

        if (e->hasReference(row)) // \hyperref[sec:marker2]{SecondSection}
            row_strings[0] = "\\hyperref[sec:"+e->reference(row)+"]{"+row_strings.at(0)+"}";

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
    loginf << "LatexVisitor: visit: EvaluationResultsReportSectionContentFigure" << e->name();
}

void LatexVisitor::visit(ListBoxView* e)
{
    assert (e);

    loginf << "LatexVisitor: visit: ListBoxView " << e->instanceId();

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

#if USE_EXPERIMENTAL_SOURCE == true
void LatexVisitor::visit(OSGView* e)
{
    assert (e);
    loginf << "LatexVisitor: visit: OSGView " << e->instanceId();

    std::string screenshot_path = report_.path()+"/screenshots";

    loginf << "LatexVisitor: visit: path '" << screenshot_path << "'";

    if (!screenshot_folder_created_)
    {
        Files::createMissingDirectories(screenshot_path);
        screenshot_folder_created_ = true;
    }

    e->showInTabWidget();

    OSGViewDataWidget* data_widget = e->getDataWidget();
    assert (data_widget);

    if (wait_on_map_loading_)
        data_widget->waitUntilMapLoaded();

    data_widget->clearMouseCoordinates();

    // normal screenshot

    QImage screenshot = data_widget->grabFrameBuffer();

    std::string image_path = screenshot_path+"/"+image_prefix_+"_"+e->instanceId()+".jpg";
    assert (!screenshot.isNull());

    loginf << "LatexVisitor: visit: saving screenshot as '" << image_path << "'";
    bool ret = screenshot.save(image_path.c_str(), "JPG"); // , 50
    assert (ret);

    LatexSection& sec = report_.getSection(current_section_name_);

    if (add_overview_screenshot_) // overview screenshot
    {
        data_widget->zoomHomeSlot();
        data_widget->addDataMarker();

        QImage overview_screenshot = data_widget->grabFrameBuffer();

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

void LatexVisitor::imagePrefix(const std::string& image_prefix)
{
    image_prefix_ = image_prefix;
}

