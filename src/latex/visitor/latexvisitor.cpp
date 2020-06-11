#include "latexvisitor.h"
#include "latexdocument.h"
#include "latexsection.h"
#include "latextable.h"
#include "lateximage.h"
#include "viewpoint.h"
#include "listboxview.h"
#include "listboxviewdatawidget.h"
#include "allbuffertablewidget.h"
#include "osgview.h"
#include "osgviewdatawidget.h"
#include "logger.h"
#include "stringconv.h"
#include "json.h"
#include "files.h"

#include <sstream>

#include <QPixmap>

using namespace std;
using namespace Utils;

LatexVisitor::LatexVisitor(LatexDocument& report, bool group_by_type)
    : report_(report), group_by_type_(group_by_type)
{
}


void LatexVisitor::visit(ViewPoint* e)
{
    assert (e);

    loginf << "LatexVisitor: visit: id " << e->id();

    const nlohmann::json& j_data = e->data();

    assert (j_data.contains("name"));
    string name = String::latexString(j_data.at("name"));

    assert (j_data.contains("type"));
    string type = String::latexString(j_data.at("type"));

    assert (j_data.contains("status"));
    string status = j_data.at("status");

    if (group_by_type_)
        current_section_name_ = "View Points:"+type+":ID "+to_string(e->id())+" "+name;
    else
        current_section_name_ = "View Points:ID "+to_string(e->id())+" "+name;

    LatexSection& sec = report_.getSection(current_section_name_);

    //    stringstream ss;

    //    ss << "Information:\n";
    //    ss << R"(\begin{itemize})" << "\n";
    //    ss << R"( \item \textbf{id}: )" << e->id() << "\n";
    //    ss << R"( \item \textbf{name}: )" << name << "\n";
    //    ss << R"( \item \textbf{type}: )" << type << "\n";
    //    ss << R"( \item \textbf{status}: )" << status << "\n";

    //    for (auto& j_it : j_data.items())
    //    {
    //        if (!j_it.value().is_primitive())
    //            continue;

    //        if (j_it.key() == "id" || j_it.key() == "name" || j_it.key() == "type" || j_it.key() == "status")
    //            continue;

    //        ss << R"( \item \textbf{)" << String::latexString(j_it.key())
    //           << "}: " << String::latexString(JSON::toString(j_it.value())) << "\n";
    //    }

    //    ss << R"(\end{itemize})" << "\n";

    //    sec.addText(ss.str());

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
}

void LatexVisitor::visit(ListBoxView* e)
{
    assert (e);

    loginf << "LatexVisitor: visit: listboxview " << e->instanceId();

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
}

#if USE_EXPERIMENTAL_SOURCE == true
void LatexVisitor::visit(OSGView* e)
{
    assert (e);
    loginf << "LatexVisitor: visit: osgview " << e->instanceId();

    std::string screenshot_path = report_.path()+"/screenshots";

    loginf << "LatexVisitor: visit: path '" << screenshot_path << "'";

    if (!screenshot_folder_created_)
    {
        Files::createMissingDirectories(screenshot_path);
        screenshot_folder_created_ = true;
    }

    OSGViewDataWidget* data_widget = e->getDataWidget();
    assert (data_widget);

    //QPixmap screenshot = QPixmap::grabWidget(data_widget, data_widget->rect());

//    QPixmap screenshot(data_widget->size());
//    data_widget->render(&screenshot); // , QPoint(), QRegion(data_widget->size())

    QImage screenshot = data_widget->grabFrameBuffer();

    std::string image_path = screenshot_path+"/"+image_prefix_+"_"+e->instanceId()+".jpg";

    if(screenshot.isNull())
        logerr << "LatexVisitor: visit: unable to create screenshot";
    else
    {
        loginf << "LatexVisitor: visit: saving screenshot as '" << image_path << "'";
        bool ret = screenshot.save(image_path.c_str(), "JPG"); // , 50
        assert (ret);

        LatexSection& sec = report_.getSection(current_section_name_);

        sec.addImage(image_path, e->instanceId());
        //LatexImage& image = sec.getImage(image_path);
    }
}
#endif

void LatexVisitor::imagePrefix(const std::string& image_prefix)
{
    image_prefix_ = image_prefix;
}

