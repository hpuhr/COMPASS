#include "dbcontentlabelgenerator.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "logger.h"
#include "util/stringconv.h"

using namespace std;
using namespace nlohmann;
using namespace Utils;

DBContentLabelGenerator::DBContentLabelGenerator(const std::string& class_id, const std::string& instance_id,
                                                 DBContentManager& manager)
    : Configurable(class_id, instance_id, &manager), dbcont_manager_(manager)
{
    registerParameter("auto_label", &auto_label_, true);
    registerParameter("label_config", &label_config_, json::object());

    createSubConfigurables();
}

DBContentLabelGenerator::~DBContentLabelGenerator()
{
}

void DBContentLabelGenerator::generateSubConfigurable(const string& class_id, const string& instance_id)
{
//    if (class_id == "Variable")
//    {
//        string var_name = configuration()
//                .getSubConfiguration(class_id, instance_id)
//                .getParameterConfigValueString("name");

//        if (hasVariable(var_name))
//            logerr << "DBContent: generateSubConfigurable: duplicate variable " << instance_id
//                   << " with name '" << var_name << "'";

//        assert(!hasVariable(var_name));

//        logdbg << "DBContent: generateSubConfigurable: generating variable " << instance_id
//               << " with name " << var_name;

//        variables_.emplace_back(new Variable(class_id, instance_id, this));
//    }
    throw runtime_error("DBContentLabelGenerator: generateSubConfigurable: unknown class_id " + class_id);

}

std::vector<std::string> DBContentLabelGenerator::getLabelTexts(
        const std::string& dbcontent_name, unsigned int buffer_index)
{
    std::map<std::string, std::shared_ptr<Buffer>> buffers = dbcont_manager_.loadedData();
    assert (buffers.count(dbcontent_name));

    std::shared_ptr<Buffer> buffer = buffers.at(dbcontent_name);
    assert (buffer_index < buffer->size());

    std::vector<std::string> tmp;

    using namespace dbContent;

    Variable* acid_var {nullptr};
    if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ti_))
        acid_var = &dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_ti_);

    Variable* acad_var {nullptr};
    if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ta_))
        acad_var = &dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_ta_);

    Variable& m3a_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_);

    // 1x1
    {
        string main_id;

        if (acid_var && buffer->has<string>(acid_var->name())
                && !buffer->get<string>(acid_var->name()).isNull(buffer_index))
            main_id = buffer->get<string>(acid_var->name()).get(buffer_index);
        else if (acad_var && buffer->has<unsigned int>(acad_var->name()) &&
                 !buffer->get<unsigned int>(acad_var->name()).isNull(buffer_index))
            main_id = acad_var->getAsSpecialRepresentationString(
                        buffer->get<unsigned int>(acad_var->name()).get(buffer_index));
        else if (buffer->has<unsigned int>(m3a_var.name()) &&
                 !buffer->get<unsigned int>(m3a_var.name()).isNull(buffer_index))
            main_id = m3a_var.getAsSpecialRepresentationString(
                        buffer->get<unsigned int>(m3a_var.name()).get(buffer_index));

        tmp.push_back(main_id);
    }

    // 2x2
    {
        // 1,2
        string acid;

        if (acid_var && buffer->has<string>(acid_var->name())
                && !buffer->get<string>(acid_var->name()).isNull(buffer_index))
            acid = buffer->get<string>(acid_var->name()).get(buffer_index);

        tmp.push_back(acid);

        // 2,1
        string m3a;

        if (buffer->has<unsigned int>(m3a_var.name()) &&
                !buffer->get<unsigned int>(m3a_var.name()).isNull(buffer_index))
            m3a = m3a_var.getAsSpecialRepresentationString(buffer->get<unsigned int>(m3a_var.name()).get(buffer_index));

        tmp.push_back(m3a);

        // 2,2
        Variable& mc_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_);
        string mc;

        if (buffer->has<float>(mc_var.name()) &&
                !buffer->get<float>(mc_var.name()).isNull(buffer_index))
            mc = String::doubleToStringPrecision(buffer->get<float>(mc_var.name()).get(buffer_index)/100.0,2);

        tmp.push_back(mc);
    }

    return tmp;
}

bool DBContentLabelGenerator::autoLabel() const
{
    return auto_label_;
}

void DBContentLabelGenerator::autoLabel(bool auto_label)
{
    auto_label_ = auto_label;
}

void DBContentLabelGenerator::checkSubConfigurables()
{
    // nothing to see here
}
