#include "latexvisitor.h"
#include "latexdocument.h"
#include "latexsection.h"
#include "latextable.h"
#include "viewpoint.h"
#include "logger.h"
#include "stringconv.h"
#include "json.h"

#include <sstream>

using namespace std;
using namespace Utils;

LatexVisitor::LatexVisitor(LatexDocument& report)
    : report_(report)
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

    string section_name = "View Points:ID "+to_string(e->id())+" "+name;

    LatexSection& sec = report_.getSection(section_name);

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
