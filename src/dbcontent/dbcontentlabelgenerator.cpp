#include "dbcontentlabelgenerator.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "logger.h"
#include "util/stringconv.h"

#include <algorithm>

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
        {
            main_id = buffer->get<string>(acid_var->name()).get(buffer_index);
            main_id.erase(std::remove(main_id.begin(), main_id.end(), ' '), main_id.end());
        }
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

    if (current_lod_ == 1)
        return tmp;

    // 2x2
    {
        // 1,2

        string acid;

        if (acid_var && buffer->has<string>(acid_var->name())
                && !buffer->get<string>(acid_var->name()).isNull(buffer_index))
            acid = buffer->get<string>(acid_var->name()).get(buffer_index);

        acid.erase(std::remove(acid.begin(), acid.end(), ' '), acid.end());
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

    if (current_lod_ == 2)
        return tmp;

    // 3x3
    {
        // 1,3

        string acad;

        if (acad_var && buffer->has<unsigned int>(acad_var->name())
                && !buffer->get<unsigned int>(acad_var->name()).isNull(buffer_index))
            acad = acad_var->getAsSpecialRepresentationString(
                        buffer->get<unsigned int>(acad_var->name()).get(buffer_index));

        tmp.push_back(acad);

        // 2,3
        string ds_name;

        Variable& dsid_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_);

        if (buffer->has<unsigned int>(dsid_var.name()) &&
                !buffer->get<unsigned int>(dsid_var.name()).isNull(buffer_index))
            ds_name = dsid_var.getAsSpecialRepresentationString(
                        buffer->get<unsigned int>(dsid_var.name()).get(buffer_index));

        tmp.push_back(ds_name);

        // 3,1
        Variable* spi_var {nullptr};
        if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_spi_))
            spi_var = &dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_spi_);

        string spi("?");

        if (spi_var && buffer->has<bool>(spi_var->name()) &&
                !buffer->get<bool>(spi_var->name()).isNull(buffer_index))
            spi = to_string(buffer->get<bool>(spi_var->name()).get(buffer_index));

        tmp.push_back(spi);

        // 3,2
        Variable* c_d_var {nullptr};
        if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_climb_descent_))
            c_d_var = &dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_climb_descent_);

        string c_d("?");

        if (c_d_var && buffer->has<unsigned char>(c_d_var->name()) &&
                !buffer->get<unsigned char>(c_d_var->name()).isNull(buffer_index))
            c_d = to_string(buffer->get<unsigned char>(c_d_var->name()).get(buffer_index));

        tmp.push_back(c_d);

        // 3,3
        tmp.push_back("UGA");
    }

    //        Variable& tod_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_tod_);
    //        string tod;

    //        if (buffer->has<float>(tod_var.name()) &&
    //                !buffer->get<float>(tod_var.name()).isNull(buffer_index))
    //            tod = String::timeStringFromDouble(buffer->get<float>(tod_var.name()).get(buffer_index));

    //        tmp.push_back(tod);

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

void DBContentLabelGenerator::registerLeafItemLabel (GeometryLeafItemLabels& item_label)
{
    assert (!item_labels_.count(&item_label));
    item_labels_.insert(&item_label);
}

void DBContentLabelGenerator::unregisterLeafItemLabel (GeometryLeafItemLabels& item_label)
{
    assert (item_labels_.count(&item_label));
    item_labels_.erase(&item_label);
}

unsigned int DBContentLabelGenerator::currentLOD() const
{
    return current_lod_;
}

void DBContentLabelGenerator::currentLOD(unsigned int current_lod)
{
    current_lod_ = current_lod;
}

void DBContentLabelGenerator::checkSubConfigurables()
{
    // nothing to see here
}
