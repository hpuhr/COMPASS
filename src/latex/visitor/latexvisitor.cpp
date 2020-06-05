#include "latexvisitor.h"
#include "latexdocument.h"
#include "latexsection.h"
#include "viewpoint.h"
#include "logger.h"
#include "stringconv.h"

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
    string name = j_data.at("name");

    assert (j_data.contains("type"));
    string type = j_data.at("type");

    string section_name = String::latexString("View Points:ID "+to_string(e->id())+" "+name);

    LatexSection& sec = report_.getSection(section_name);
    sec.addText(type);

}
