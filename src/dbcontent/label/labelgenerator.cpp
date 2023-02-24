#include "dbcontent/label/labelgenerator.h"
#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/label/labelgeneratorwidget.h"
#include "dbcontent/label/labelcontentdialog.h"
#include "datasourcemanager.h"
#include "dbdatasource.h"
#include "logger.h"
#include "util/stringconv.h"
#include "util/number.h"

#include "global.h"

#if USE_EXPERIMENTAL_SOURCE
#include "geometryleafitemlabels.h"
#endif

#include <QRect>

#include <osg/Vec4>

#include <algorithm>

using namespace std;
using namespace nlohmann;
using namespace Utils;

namespace dbContent
{

LabelGenerator::LabelGenerator(const std::string& class_id, const std::string& instance_id,
                               DBContentManager& manager)
    : Configurable(class_id, instance_id, &manager), dbcont_manager_(manager)
{
    registerParameter("auto_label", &auto_label_, true);
    registerParameter("label_directions", &label_directions_, json::object());
    registerParameter("label_lines", &label_lines_, json::object());
    registerParameter("label_config", &label_config_, json::object());
    registerParameter("label_ds_ids", &label_ds_ids_, json::object());
    registerParameter("declutter_labels", &declutter_labels_, true);
    registerParameter("max_declutter_labels", &max_declutter_labels_, 200);

    registerParameter("filter_mode3a_active", &filter_mode3a_active_, false);
    registerParameter("filter_mode3a_values", &filter_mode3a_values_, "7000,7777");
    updateM3AValuesFromStr(filter_mode3a_values_);

    registerParameter("filter_modec_min_active", &filter_modec_min_active_, false);
    registerParameter("filter_modec_min_value", &filter_modec_min_value_, 10);
    registerParameter("filter_modec_max_active", &filter_modec_max_active_, false);
    registerParameter("filter_modec_max_value", &filter_modec_max_value_, 400);
    registerParameter("filter_modec_null_wanted", &filter_modec_null_wanted_, false);

    registerParameter("filter_ti_active", &filter_ti_active_, false);
    registerParameter("filter_ti_values", &filter_ti_values_, "OE");
    updateTIValuesFromStr(filter_ti_values_);

    registerParameter("filter_ta_active", &filter_ta_active_, false);
    registerParameter("filter_ta_values", &filter_ta_values_, "AADDCC");
    updateTAValuesFromStr(filter_ta_values_);

    registerParameter("filter_primary_only_activ", &filter_primary_only_active_, false);

    registerParameter("label_opacity", &label_opacity_, 0.9);

    createSubConfigurables();
}

LabelGenerator::~LabelGenerator()
{
}

void LabelGenerator::generateSubConfigurable(const string& class_id, const string& instance_id)
{
    throw runtime_error("DBContentLabelGenerator: generateSubConfigurable: unknown class_id " + class_id);

}

std::vector<std::string> LabelGenerator::getLabelTexts(
        const std::string& dbcontent_name, unsigned int buffer_index)
{
    std::vector<std::string> tmp;


    std::map<std::string, std::shared_ptr<Buffer>> buffers = dbcont_manager_.loadedData();
    if (!buffers.count(dbcontent_name))
    {
        logerr << "LabelGenerator: getLabelTexts: dbcontent_name '" << dbcontent_name << "' not in buffers";
        return tmp;
    }

    std::shared_ptr<Buffer> buffer = buffers.at(dbcontent_name);
    assert (buffer_index < buffer->size());

    using namespace dbContent;

    Variable& assoc_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_associations_);

    Variable* acid_var {nullptr};
    if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ti_))
        acid_var = &dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_ti_);

    Variable* acad_var {nullptr};
    if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ta_))
        acad_var = &dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_ta_);

    Variable& m3a_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_);

    // first row
    // 1x1
    {
        string main_id("?");

        if (buffer->has<nlohmann::json>(assoc_var.name())
                && !buffer->get<nlohmann::json>(assoc_var.name()).isNull(buffer_index))
        {
            main_id = buffer->get<nlohmann::json>(assoc_var.name()).get(buffer_index).dump();
            main_id = main_id.substr(1, main_id.size()-2); // remove first and last chars []
        }
        else if (acid_var && buffer->has<string>(acid_var->name())
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
            main_id = getMode3AText(dbcontent_name, buffer_index, buffer);

        tmp.push_back(main_id);
    }

    if (round(current_lod_) == 1)
        return tmp;

    // 1,2
    string acid;

    if (acid_var && buffer->has<string>(acid_var->name())
            && !buffer->get<string>(acid_var->name()).isNull(buffer_index))
        acid = buffer->get<string>(acid_var->name()).get(buffer_index);

    acid.erase(std::remove(acid.begin(), acid.end(), ' '), acid.end());
    tmp.push_back(acid);

    if (round(current_lod_) == 3)
    {
        // 1,3
        tmp.push_back(getVariableValue(dbcontent_name, 0*3+2, buffer, buffer_index));
    }

    // row 2

    // 2,1
    string m3a;

    if (buffer->has<unsigned int>(m3a_var.name()) &&
            !buffer->get<unsigned int>(m3a_var.name()).isNull(buffer_index))
        m3a = getMode3AText(dbcontent_name, buffer_index, buffer);

    tmp.push_back(m3a);

    // 2,2
    Variable& mc_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_);
    string mc;

    if (buffer->has<float>(mc_var.name()) &&
            !buffer->get<float>(mc_var.name()).isNull(buffer_index))
        mc = getModeCText(dbcontent_name, buffer_index, buffer);

    tmp.push_back(mc);

    if (round(current_lod_) == 2)
        return tmp;

    // 2,3
    tmp.push_back(getVariableValue(dbcontent_name, 1*3+2, buffer, buffer_index));

    // row 3

    // 3,1
    tmp.push_back(getVariableValue(dbcontent_name, 2*3+0, buffer, buffer_index));

    // 3,2
    tmp.push_back(getVariableValue(dbcontent_name, 2*3+1, buffer, buffer_index));

    // 3,3

    tmp.push_back(getVariableValue(dbcontent_name, 2*3+2, buffer, buffer_index));

    return tmp;
}

std::vector<std::string> LabelGenerator::getFullTexts(const std::string& dbcontent_name, unsigned int buffer_index)
{
    std::vector<std::string> tmp;

    std::map<std::string, std::shared_ptr<Buffer>> buffers = dbcont_manager_.loadedData();
    if (!buffers.count(dbcontent_name))
    {
        logerr << "LabelGenerator: getFullTexts: dbcontent_name '" << dbcontent_name << "' not in buffers";
        return tmp;
    }

    DBContent& db_content = dbcont_manager_.dbContent(dbcontent_name);

    std::shared_ptr<Buffer> buffer = buffers.at(dbcontent_name);
    assert (buffer_index < buffer->size());

    using namespace dbContent;

    string value_str;
    string property_name;
    bool null;
    bool use_presentation;

    tmp.push_back("Variable");
    tmp.push_back("Value");
    //tmp.push_back("Description");
    tmp.push_back("Unit");

    // do common label parts
    set<string> used_varnames;
    {
        using namespace dbContent;

        Variable& assoc_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_associations_);

        Variable* acid_var {nullptr};
        if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ti_))
            acid_var = &dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_ti_);

        Variable* acad_var {nullptr};
        if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ta_))
            acad_var = &dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_ta_);

        Variable& m3a_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_);

        string varname, value, unit;

        // first row
        // 1x1
        {
            varname = "Best Available Identification";
            value = "?";
            unit = "";

            if (buffer->has<nlohmann::json>(assoc_var.name())
                    && !buffer->get<nlohmann::json>(assoc_var.name()).isNull(buffer_index))
            {
                value = buffer->get<nlohmann::json>(assoc_var.name()).get(buffer_index).dump();
                value = value.substr(1, value.size()-2); // remove first and last chars []
                value += " ("+assoc_var.name()+")";
            }
            else if (acid_var && buffer->has<string>(acid_var->name())
                     && !buffer->get<string>(acid_var->name()).isNull(buffer_index))
            {
                value = buffer->get<string>(acid_var->name()).get(buffer_index);
                value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
                value += " ("+acid_var->name()+")";
            }
            else if (acad_var && buffer->has<unsigned int>(acad_var->name()) &&
                     !buffer->get<unsigned int>(acad_var->name()).isNull(buffer_index))
            {
                value = acad_var->getAsSpecialRepresentationString(
                            buffer->get<unsigned int>(acad_var->name()).get(buffer_index));
                value += " ("+acad_var->name()+")";
            }
            else if (buffer->has<unsigned int>(m3a_var.name()) &&
                     !buffer->get<unsigned int>(m3a_var.name()).isNull(buffer_index))
            {
                value = getMode3AText(dbcontent_name, buffer_index, buffer);
                value += " ("+m3a_var.name()+")";
            }

            tmp.push_back(varname);
            tmp.push_back(value);
            tmp.push_back(unit);
        }

        // 1,2
        varname = "";
        value = "";
        unit = "";

        if (acid_var)
        {
            varname = acid_var->name();

            if (buffer->has<string>(acid_var->name())
                    && !buffer->get<string>(acid_var->name()).isNull(buffer_index))
            {

                value = buffer->get<string>(acid_var->name()).get(buffer_index);
                value.erase(std::remove(value.begin(), value.end(), ' '), value.end());
            }

            used_varnames.insert(varname);
        }
        tmp.push_back(varname);
        tmp.push_back(value);
        tmp.push_back(unit);

        unsigned int index;

        // 1,3
        {
            index = 0*3+2;
            varname = getVariableName(dbcontent_name, index);
            tmp.push_back(varname);
            tmp.push_back(getVariableValue(dbcontent_name, index, buffer, buffer_index));
            tmp.push_back(getVariableUnit(dbcontent_name, index));

            if (varname.size())
                used_varnames.insert(varname);
        }

        // row 2

        // 2,1
        varname = "";
        value = "";
        unit = "";

        varname = m3a_var.name();
        if (buffer->has<unsigned int>(m3a_var.name()) &&
                !buffer->get<unsigned int>(m3a_var.name()).isNull(buffer_index))
        {
            value = getMode3AText(dbcontent_name, buffer_index, buffer);
        }
        tmp.push_back(varname);
        tmp.push_back(value);
        tmp.push_back(unit);

        if (varname.size())
            used_varnames.insert(varname);

        // 2,2
        Variable& mc_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_);
        varname = mc_var.name();
        value = "";
        unit = mc_var.dimensionUnitStr();

        if (buffer->has<float>(mc_var.name()) &&
                !buffer->get<float>(mc_var.name()).isNull(buffer_index))
        {
            value = getModeCText(dbcontent_name, buffer_index, buffer);
        }
        tmp.push_back(varname);
        tmp.push_back(value);
        tmp.push_back(unit);

        if (varname.size())
            used_varnames.insert(varname);


        // 2,3
        index = 1*3+2;
        varname = getVariableName(dbcontent_name, index);
        tmp.push_back(varname);
        tmp.push_back(getVariableValue(dbcontent_name, index, buffer, buffer_index));
        tmp.push_back(getVariableUnit(dbcontent_name, index));

        if (varname.size())
            used_varnames.insert(varname);

        // row 3

        // 3,1
        index = 2*3+0;
        varname = getVariableName(dbcontent_name, index);

        tmp.push_back(varname);
        tmp.push_back(getVariableValue(dbcontent_name, index, buffer, buffer_index));
        tmp.push_back(getVariableUnit(dbcontent_name, index));

        if (varname.size())
            used_varnames.insert(varname);

        // 3,2
        index = 2*3+1;
        varname = getVariableName(dbcontent_name, index);

        tmp.push_back(varname);
        tmp.push_back(getVariableValue(dbcontent_name, index, buffer, buffer_index));
        tmp.push_back(getVariableUnit(dbcontent_name, index));

        if (varname.size())
            used_varnames.insert(varname);

        // 3,3
        index = 2*3+2;
        varname = getVariableName(dbcontent_name, index);

        tmp.push_back(varname);
        tmp.push_back(getVariableValue(dbcontent_name, index, buffer, buffer_index));
        tmp.push_back(getVariableUnit(dbcontent_name, index));

        if (varname.size())
            used_varnames.insert(varname);
    }

    // do rest, first an empty line
    tmp.push_back("");
    tmp.push_back("");
    tmp.push_back("");

    for (auto& var_it : db_content.variables())
    {
        if (used_varnames.count(var_it.first)) // skip if used
            continue;

        if (buffer->hasProperty(*var_it.second.get()))
        {
            property_name = var_it.first;
            PropertyDataType data_type = var_it.second->dataType();

            use_presentation = var_it.second->representation() != Variable::Representation::STANDARD;

            value_str = NULL_STRING;

            if (data_type == PropertyDataType::BOOL)
            {
                assert(buffer->has<bool>(property_name));
                null = buffer->get<bool>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation)
                        value_str = var_it.second->getRepresentationStringFromValue(
                                    buffer->get<bool>(property_name).getAsString(buffer_index));
                    else
                        value_str = buffer->get<bool>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::CHAR)
            {
                assert(buffer->has<char>(property_name));
                null = buffer->get<char>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation)
                        value_str = var_it.second->getRepresentationStringFromValue(
                                    buffer->get<char>(property_name).getAsString(buffer_index));
                    else
                        value_str = buffer->get<char>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::UCHAR)
            {
                assert(buffer->has<unsigned char>(property_name));
                null = buffer->get<unsigned char>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation)
                        value_str = var_it.second->getRepresentationStringFromValue(
                                    buffer->get<unsigned char>(property_name).getAsString(buffer_index));
                    else
                        value_str =
                                buffer->get<unsigned char>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::INT)
            {
                assert(buffer->has<int>(property_name));
                null = buffer->get<int>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation)
                        value_str = var_it.second->getRepresentationStringFromValue(
                                    buffer->get<int>(property_name).getAsString(buffer_index));
                    else
                        value_str = buffer->get<int>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::UINT)
            {
                assert(buffer->has<unsigned int>(property_name));
                null = buffer->get<unsigned int>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation)
                        value_str = var_it.second->getRepresentationStringFromValue(
                                    buffer->get<unsigned int>(property_name).getAsString(buffer_index));
                    else
                        value_str =
                                buffer->get<unsigned int>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::LONGINT)
            {
                assert(buffer->has<long int>(property_name));
                null = buffer->get<long int>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation)
                        value_str = var_it.second->getRepresentationStringFromValue(
                                    buffer->get<long int>(property_name).getAsString(buffer_index));
                    else
                        value_str = buffer->get<long int>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::ULONGINT)
            {
                assert(buffer->has<unsigned long int>(property_name));
                null = buffer->get<unsigned long int>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation)
                        value_str = var_it.second->getRepresentationStringFromValue(
                                    buffer->get<unsigned long int>(property_name)
                                    .getAsString(buffer_index));
                    else
                        value_str =
                                buffer->get<unsigned long int>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::FLOAT)
            {
                assert(buffer->has<float>(property_name));
                null = buffer->get<float>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation)
                        value_str = var_it.second->getRepresentationStringFromValue(
                                    buffer->get<float>(property_name).getAsString(buffer_index));
                    else
                        value_str = buffer->get<float>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::DOUBLE)
            {
                assert(buffer->has<double>(property_name));
                null = buffer->get<double>(property_name).isNull(buffer_index);
                if (!null)
                {
                    if (use_presentation)
                        value_str = var_it.second->getRepresentationStringFromValue(
                                    buffer->get<double>(property_name).getAsString(buffer_index));
                    else
                        value_str = buffer->get<double>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::STRING)
            {
                assert(buffer->has<std::string>(property_name));
                null = buffer->get<std::string>(property_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = buffer->get<std::string>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::JSON)
            {
                assert(buffer->has<nlohmann::json>(property_name));
                null = buffer->get<nlohmann::json>(property_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = buffer->get<nlohmann::json>(property_name).getAsString(buffer_index);
                }
            }
            else if (data_type == PropertyDataType::TIMESTAMP)
            {
                assert(buffer->has<boost::posix_time::ptime>(property_name));
                null = buffer->get<boost::posix_time::ptime>(property_name).isNull(buffer_index);
                if (!null)
                {
                    value_str = buffer->get<boost::posix_time::ptime>(property_name).getAsString(buffer_index);
                }
            }
            else
                throw std::domain_error("LabelGenerator: getFullTexts: unknown property data type");

            tmp.push_back(property_name);
            tmp.push_back(value_str);
            //tmp.push_back(var_it->description());
            tmp.push_back(var_it.second->dimensionUnitStr());
        }
    }

    assert (tmp.size() % 3 == 0);

    return tmp;
}

bool LabelGenerator::autoLabel() const
{
    return auto_label_;
}

void LabelGenerator::autoLabel(bool auto_label)
{
    auto_label_ = auto_label;

    emit labelOptionsChangedSignal();
}

void LabelGenerator::autoAdustCurrentLOD(unsigned int num_labels_on_screen)
{
    float old_lod = current_lod_;

    if (num_labels_on_screen < 25)
        current_lod_ = (2*old_lod + 3) / 3;
    else if (num_labels_on_screen < 50)
        current_lod_ = (2*old_lod + 2) / 3;
    else if (num_labels_on_screen < 75)
        current_lod_ = (2*old_lod + 1) / 3;
    else
        current_lod_ = 1;

    //loginf << "DBContentLabelGenerator: autoAdustCurrentLOD: old " << old_lod << " current " << current_lod_;

    // failsafe
    if (current_lod_ < 1)
        current_lod_ = 1;
    else if (current_lod_ > 3)
        current_lod_ = 3;

    logdbg << "DBContentLabelGenerator: autoAdustCurrentLOD: num labels on screen "
           << num_labels_on_screen << " old " << (unsigned int) old_lod
           << " current " << round(current_lod_) << " float " << current_lod_;
}

unsigned int LabelGenerator::currentLOD() const
{
    return round(current_lod_);
}

void LabelGenerator::currentLOD(unsigned int current_lod)
{
    current_lod_ = current_lod;
}


bool LabelGenerator::autoLOD() const
{
    return auto_lod_;
}

void LabelGenerator::autoLOD(bool auto_lod)
{
    auto_lod_ = auto_lod;

    emit labelOptionsChangedSignal();
}

void LabelGenerator::addLabelDSID(unsigned int ds_id)
{
    label_ds_ids_[to_string(ds_id)] = true;

    emit labelOptionsChangedSignal();
}

void LabelGenerator::removeLabelDSID(unsigned int ds_id)
{
    label_ds_ids_[to_string(ds_id)] = false;

    emit labelOptionsChangedSignal();
}

void LabelGenerator::labelAllDSIDs()
{
    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    for (const auto& ds_it : ds_man.dbDataSources())
        label_ds_ids_[to_string(ds_it->id())] = true;

    emit labelOptionsChangedSignal();
}

void LabelGenerator::labelNoDSIDs()
{
    label_ds_ids_.clear();

    emit labelOptionsChangedSignal();
}

//const std::map<unsigned int, bool> LabelGenerator::labelDSIDs() const
//{
//    return label_ds_ids_.get<std::map<unsigned int, bool>>();
//}

bool LabelGenerator::anyDSIDLabelWanted()
{
    for (auto& ds_it : label_ds_ids_.get<std::map<string, bool>>())
    {
        if (ds_it.second)
            return true;
    }

    return false;
}

bool LabelGenerator::labelWanted(unsigned int ds_id)
{
    if (label_ds_ids_.contains(to_string(ds_id)))
        return label_ds_ids_.at(to_string(ds_id));
    else
        return false;
}

bool LabelGenerator::labelWanted(std::shared_ptr<Buffer> buffer, unsigned int index)
{
    string dbcont_name = buffer->dbContentName();

    // check line
    {
        assert (dbcont_manager_.metaCanGetVariable(dbcont_name, DBContent::meta_var_line_id_));
        dbContent::Variable& line_var = dbcont_manager_.metaGetVariable(
                    dbcont_name, DBContent::meta_var_line_id_);
        assert (dbcont_manager_.metaCanGetVariable(dbcont_name, DBContent::meta_var_datasource_id_));
        dbContent::Variable& ds_id_var = dbcont_manager_.metaGetVariable(
                    dbcont_name, DBContent::meta_var_datasource_id_);


        assert (buffer->has<unsigned int> (line_var.name()));
        assert (buffer->has<unsigned int> (ds_id_var.name()));

        NullableVector<unsigned int>& line_vec = buffer->get<unsigned int> (line_var.name());
        assert (!line_vec.isNull(index));

        NullableVector<unsigned int>& ds_id_vec = buffer->get<unsigned int> (ds_id_var.name());
        assert (!ds_id_vec.isNull(index));

        if (!labelWanted(ds_id_vec.get(index)))
            return false;

        if (labelLine(ds_id_vec.get(index)) != line_vec.get(index))
            return false;
    }

    if (filter_mode3a_active_)
    {
        if (!dbcont_manager_.metaCanGetVariable(dbcont_name, DBContent::meta_var_m3a_))
            return false;

        dbContent::Variable& var = dbcont_manager_.metaGetVariable(dbcont_name, DBContent::meta_var_m3a_);

        assert (buffer->has<unsigned int> (var.name()));

        NullableVector<unsigned int>& data_vec = buffer->get<unsigned int> (var.name());

        if (data_vec.isNull(index))
        {
            if (!filter_m3a_null_wanted_)
                return false; // null and not wanted
        }
        else if (!filter_m3a_values_set_.count(data_vec.get(index)))
            return false; // set and not in values
    }

    if (filter_modec_min_active_ || filter_modec_max_active_)
    {
        if (!dbcont_manager_.metaCanGetVariable(dbcont_name, DBContent::meta_var_mc_))
            return false;

        dbContent::Variable& var = dbcont_manager_.metaGetVariable(dbcont_name, DBContent::meta_var_mc_);

        assert (buffer->has<float> (var.name()));

        NullableVector<float>& data_vec = buffer->get<float> (var.name());

        if (data_vec.isNull(index))
        {
            if (!filter_modec_null_wanted_)
                return false; // null and not wanted
        }
        else
        {
            if (filter_modec_min_active_ && data_vec.get(index)/100.0 < filter_modec_min_value_)
                return false;

            if (filter_modec_max_active_ && data_vec.get(index)/100.0 > filter_modec_max_value_)
                return false;
        }
    }

    if (filter_ti_active_)
    {
        if (!dbcont_manager_.metaCanGetVariable(dbcont_name, DBContent::meta_var_ti_))
            return false;

        dbContent::Variable& acid_var = dbcont_manager_.metaGetVariable(dbcont_name, DBContent::meta_var_ti_);

        assert (buffer->has<string> (acid_var.name()));

        NullableVector<string>& acid_vec = buffer->get<string> (acid_var.name());

        dbContent::Variable* cs_fpl_var {nullptr}; // only set in cat062
        NullableVector<string>* cs_fpl_vec {nullptr}; // only set in cat062

        if (dbcont_name == "CAT062")
        {
            assert (dbcont_manager_.canGetVariable(dbcont_name, DBContent::var_cat062_callsign_fpl_));

            cs_fpl_var = &dbcont_manager_.getVariable(dbcont_name, DBContent::var_cat062_callsign_fpl_);

            assert (buffer->has<string> (cs_fpl_var->name()));
            cs_fpl_vec = &buffer->get<string> (cs_fpl_var->name());
        }

//        if (acid_vec.isNull(index))
//        {
//            if (!filter_ti_null_wanted_
//                    || (cs_fpl_vec != nullptr ? cs_fpl_vec->isNull(index) : false))
//                return false; // null not wanted
//        }
        if (acid_vec.isNull(index)
                && (cs_fpl_vec != nullptr ? cs_fpl_vec->isNull(index) : true)) // null or not found
        {
            return filter_ti_null_wanted_;
        }
        else
        {
            bool found = false;

            if (!acid_vec.isNull(index))
            {
                for (auto& val_it : filter_ti_values_set_)
                {
                    if (acid_vec.get(index).find(val_it) != std::string::npos)
                    {
                        found = true;
                        break;
                    }
                }
            }

            if (cs_fpl_vec && !cs_fpl_vec->isNull(index))
            {
                for (auto& val_it : filter_ti_values_set_)
                {
                    if (cs_fpl_vec->get(index).find(val_it) != std::string::npos)
                    {
                        found = true;
                        break;
                    }
                }
            }

            if (!found)
                return false;
        }
    }

    if (filter_ta_active_)
    {
        if (!dbcont_manager_.metaCanGetVariable(dbcont_name, DBContent::meta_var_ta_))
            return false;

        dbContent::Variable& var = dbcont_manager_.metaGetVariable(dbcont_name, DBContent::meta_var_ta_);

        assert (buffer->has<unsigned int> (var.name()));

        NullableVector<unsigned int>& data_vec = buffer->get<unsigned int> (var.name());

        if (data_vec.isNull(index))
        {
            if (!filter_ta_null_wanted_)
                return false; // null and not wanted
        }
        else if (!filter_ta_values_set_.count(data_vec.get(index)))
            return false; // set and not in values
    }

    if (filter_primary_only_active_)
    {
        NullableVector<unsigned int>* m3a_vec {nullptr};
        if (dbcont_manager_.metaCanGetVariable(dbcont_name, DBContent::meta_var_m3a_))
        {
            dbContent::Variable& var = dbcont_manager_.metaGetVariable(dbcont_name, DBContent::meta_var_m3a_);
            assert (buffer->has<unsigned int> (var.name()));
            m3a_vec = &buffer->get<unsigned int> (var.name());
        }

        NullableVector<float>* mc_vec {nullptr};
        if (dbcont_manager_.metaCanGetVariable(dbcont_name, DBContent::meta_var_mc_))
        {
            dbContent::Variable& var = dbcont_manager_.metaGetVariable(dbcont_name, DBContent::meta_var_mc_);
            assert (buffer->has<float> (var.name()));
            mc_vec = &buffer->get<float> (var.name());
        }

        NullableVector<unsigned int>* ta_vec {nullptr};
        if (dbcont_manager_.metaCanGetVariable(dbcont_name, DBContent::meta_var_ta_))
        {
            dbContent::Variable& var = dbcont_manager_.metaGetVariable(dbcont_name, DBContent::meta_var_ta_);
            assert (buffer->has<unsigned int> (var.name()));
            ta_vec = &buffer->get<unsigned int> (var.name());
        }

        NullableVector<string>* ti_vec {nullptr};
        if (dbcont_manager_.metaCanGetVariable(dbcont_name, DBContent::meta_var_ti_))
        {
            dbContent::Variable& var = dbcont_manager_.metaGetVariable(dbcont_name, DBContent::meta_var_ti_);
            assert (buffer->has<string> (var.name()));
            ti_vec = &buffer->get<string> (var.name());
        }

        NullableVector<unsigned char>* type_vec {nullptr};
        if (dbcont_manager_.metaCanGetVariable(dbcont_name, DBContent::meta_var_detection_type_))
        {
            dbContent::Variable& var = dbcont_manager_.metaGetVariable(dbcont_name, DBContent::meta_var_detection_type_);
            assert (buffer->has<unsigned char> (var.name()));
            type_vec = &buffer->get<unsigned char> (var.name());
        }

        std::set<unsigned char> psr_detection {1,3,6,7};

        if (m3a_vec && !m3a_vec->isNull(index))
            return false;
        else if (mc_vec && !mc_vec->isNull(index))
            return false;
        else if (ta_vec && !ta_vec->isNull(index))
            return false;
        else if (ti_vec && !ti_vec->isNull(index))
            return false;
        else if (type_vec && !type_vec->isNull(index) && !psr_detection.count(type_vec->get(index)))
            return false;
    }

    return true;
}

bool LabelGenerator::filterMode3aActive() const
{
    return filter_mode3a_active_;
}

void LabelGenerator::filterMode3aActive(bool filter_active)
{
    filter_mode3a_active_ = filter_active;
}

std::string LabelGenerator::filterMode3aValues() const
{
    return filter_mode3a_values_;
}

void LabelGenerator::filterMode3aValues(const std::string &filter_values)
{
    filter_mode3a_values_ = filter_values;
    updateM3AValuesFromStr(filter_mode3a_values_);
}

bool LabelGenerator::filterTIActive() const
{
    return filter_ti_active_;
}

void LabelGenerator::filterTIActive(bool filter_active)
{
    filter_ti_active_ = filter_active;
}

std::string LabelGenerator::filterTIValues() const
{
    return filter_ti_values_;
}

void LabelGenerator::filterTIValues(const std::string &filter_values)
{
    filter_ti_values_ = filter_values;
    updateTIValuesFromStr(filter_ti_values_);
}

bool LabelGenerator::filterTAActive() const
{
    return filter_ta_active_;
}

void LabelGenerator::filterTAActive(bool filter_active)
{
    filter_ta_active_ = filter_active;
}

std::string LabelGenerator::filterTAValues() const
{
    return filter_ta_values_;
}

void LabelGenerator::filterTAValues(const std::string &filter_values)
{
    filter_ta_values_ = filter_values;
    updateTAValuesFromStr(filter_ta_values_);
}

bool LabelGenerator::filterModecMinActive() const
{
    return filter_modec_min_active_;
}

void LabelGenerator::filterModecMinActive(bool value)
{
    filter_modec_min_active_ = value;
}

float LabelGenerator::filterModecMinValue() const
{
    return filter_modec_min_value_;
}

void LabelGenerator::filterModecMinValue(float value)
{
    filter_modec_min_value_ = value;
}

bool LabelGenerator::filterModecMaxActive() const
{
    return filter_modec_max_active_;
}

void LabelGenerator::filterModecMaxActive(bool value)
{
    filter_modec_max_active_ = value;
}

float LabelGenerator::filterModecMaxValue() const
{
    return filter_modec_max_value_;
}

void LabelGenerator::filterModecMaxValue(float value)
{
    filter_modec_max_value_ = value;
}

bool LabelGenerator::filterModecNullWanted() const
{
    return filter_modec_null_wanted_;
}

void LabelGenerator::filterModecNullWanted(bool value)
{
    filter_modec_null_wanted_ = value;
}

void LabelGenerator::checkSubConfigurables()
{
    // nothing to see here
}

void LabelGenerator::checkLabelConfig()
{
    string key;

    for (auto& dbcont_it : dbcont_manager_)
    {
        if (!label_config_.contains(dbcont_it.first)) // create
        {
            label_config_[dbcont_it.first] = json::object();

            json& dbcont_def = label_config_.at(dbcont_it.first);

            for (unsigned int row=0; row < 3; row++)
            {
                for (unsigned int col=0; col < 3; col++)
                {
                    key = to_string(row*3 + col);

                    if (row == 0 && col == 0)
                        dbcont_def[key] = "Best available Identification";
                    else if (row == 0 && col == 1
                             && dbcont_manager_.metaCanGetVariable(dbcont_it.first, DBContent::meta_var_ti_))
                        dbcont_def[key] =
                                dbcont_manager_.metaGetVariable(dbcont_it.first, DBContent::meta_var_ti_).name();
                    else if (row == 0 && col == 2
                             && dbcont_manager_.metaCanGetVariable(dbcont_it.first, DBContent::meta_var_ta_))
                        dbcont_def[key] =
                                dbcont_manager_.metaGetVariable(dbcont_it.first, DBContent::meta_var_ta_).name();
                    else if (row == 1 && col == 0
                             && dbcont_manager_.metaCanGetVariable(dbcont_it.first, DBContent::meta_var_m3a_))
                        dbcont_def[key] =
                                dbcont_manager_.metaGetVariable(dbcont_it.first, DBContent::meta_var_m3a_).name();
                    else if (row == 1 && col == 1
                             && dbcont_manager_.metaCanGetVariable(dbcont_it.first, DBContent::meta_var_mc_))
                        dbcont_def[key] =
                                dbcont_manager_.metaGetVariable(dbcont_it.first, DBContent::meta_var_mc_).name();
                    else if (row == 1 && col == 2
                             && dbcont_manager_.metaCanGetVariable(dbcont_it.first, DBContent::meta_var_datasource_id_))
                        dbcont_def[key] =
                                dbcont_manager_.metaGetVariable(dbcont_it.first, DBContent::meta_var_datasource_id_).name();
                    else if (row == 2 && col == 0
                             && dbcont_manager_.metaCanGetVariable(dbcont_it.first, DBContent::meta_var_ground_speed_))
                        dbcont_def[key] =
                                dbcont_manager_.metaGetVariable(dbcont_it.first, DBContent::meta_var_ground_speed_).name();
                    else if (row == 2 && col == 2
                             && dbcont_manager_.metaCanGetVariable(dbcont_it.first, DBContent::meta_var_timestamp_))
                        dbcont_def[key] =
                                dbcont_manager_.metaGetVariable(dbcont_it.first, DBContent::meta_var_timestamp_).name();
                }
            }
        }
    }
}

LabelDirection LabelGenerator::labelDirection (unsigned int ds_id)
{
    string key = to_string(ds_id);

    if (label_directions_.contains(key))
    {
        unsigned int direction = label_directions_.at(key);
        assert (direction <= 3);
        return LabelDirection(direction);
    }
    else
    {
        unsigned int direction = Number::randomNumber(0, 3.99);
        assert (direction <= 3);
        label_directions_[key] = direction;
        return LabelDirection(direction);
    }
}

float LabelGenerator::labelDirectionAngle (unsigned int ds_id)
{
    LabelDirection direction = labelDirection(ds_id);

    if (direction == LEFT_UP)
        return DEG2RAD * 135.0;
    else if (direction == RIGHT_UP)
        return DEG2RAD * 45.0;
    else if (direction == LEFT_DOWN)
        return DEG2RAD * 225.0;
    else // RIGHT_DOWN
        return DEG2RAD * 315.0;
}


void LabelGenerator::labelDirection (unsigned int ds_id, LabelDirection direction)
{
    label_directions_[to_string(ds_id)] = direction;
}

void LabelGenerator::editLabelContents(const std::string& dbcontent_name)
{
    assert (!label_edit_dialog_);

    label_edit_dialog_.reset(new LabelContentDialog(dbcontent_name, *this));

    connect(label_edit_dialog_.get(), &LabelContentDialog::doneSignal,
            this, &LabelGenerator::editLabelContentsDoneSlot);

    label_edit_dialog_->show();
}

unsigned int LabelGenerator::labelLine (unsigned int ds_id) // returns 0...3
{
    string key = to_string(ds_id);

    if (label_lines_.contains(key))
    {
        unsigned int line = label_lines_.at(key);
        assert (line <= 3);
        return line;
    }
    else
    {
        DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();
        assert (ds_man.hasDBDataSource(ds_id));

        dbContent::DBDataSource& ds = ds_man.dbDataSource(ds_id);

        unsigned int line = 0;

        if (ds.hasAnyNumLoaded())
            line = ds.getFirstLoadedLine();

        label_lines_[key] = line;
        return line;
    }
}

void LabelGenerator::labelLine (unsigned int ds_id, unsigned int line)
{
    assert (line <= 3);
    string key = to_string(ds_id);
    label_lines_[key] = line;
}

// updates lines to be label according to available lines with loaded data
void LabelGenerator::updateAvailableLabelLines()
{
    logdbg << "LabelGenerator: updateAvailableLabelLines";

    unsigned int ds_id;
    unsigned int line_id;

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    bool something_changed {false};

    for (auto& line_it : label_lines_.get<std::map<std::string, unsigned int>>())
    {
        ds_id = std::atoi(line_it.first.c_str());
        line_id = line_it.second;

        if (!ds_man.hasDBDataSource(ds_id)) // check if existing in current db
            continue;

        dbContent::DBDataSource& ds = ds_man.dbDataSource(ds_id);

        if (ds.hasAnyNumLoaded())
        {
            if (ds.hasNumLoaded(line_id)) // check if current line id has data
                continue; // has data in current line
            else // set to first line with data
            {
                labelLine(ds_id, ds.getFirstLoadedLine());
                something_changed = true;
            }
        }
        else // set to first line as default
        {
            labelLine(ds_id, 0);
            something_changed = true;
        }

        logdbg << "LabelGenerator: updateAvailableLabelLines: ds_id " << ds_id
               << " new line " << labelLine(ds_id);
    }

    if (something_changed)
    {
        logdbg << "LabelGenerator: updateAvailableLabelLines: emitting change";
        emit labelLinesChangedSignal();
    }
}


void LabelGenerator::editLabelContentsDoneSlot()
{
    loginf << "LabelGenerator: editLabelContentsDoneSlot";

    assert (label_edit_dialog_);
    label_config_ = label_edit_dialog_->labelConfig();

    label_edit_dialog_->close();
    label_edit_dialog_ = nullptr;
}

nlohmann::json LabelGenerator::labelConfig() const
{
    return label_config_;
}

void LabelGenerator::addVariables (const std::string& dbcontent_name, dbContent::VariableSet& read_set)
{
    assert (label_config_.contains(dbcontent_name));

    json& dbcont_def = label_config_.at(dbcontent_name);

    DBContent& db_content = dbcont_manager_.dbContent(dbcontent_name);

    for (auto& var_it : dbcont_def.get<std::map<std::string, std::string>>())
    {
        if (var_it.second == "Best available Identification" || var_it.second == "")
            continue;

        if (!db_content.hasVariable(var_it.second))
        {
            logwrn << "LabelGenerator: addVariables: unknown var '" << var_it.second << "' in " << dbcontent_name;
            continue;
        }

        Variable& var = db_content.variable(var_it.second);
        if (!read_set.hasVariable(var))
            read_set.add(var);
    }

    // "Mode C Garbled"
    if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_g_))
    {
        Variable& var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_g_);
        if (!read_set.hasVariable(var))
            read_set.add(var);
    }
    // "Mode C Valid"
    if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_v_))
    {
        Variable& var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_v_);
        if (!read_set.hasVariable(var))
            read_set.add(var);
    }

    // "Mode 3/A Valid"
    if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_smoothed_))
    {
        Variable& var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_smoothed_);
        if (!read_set.hasVariable(var))
            read_set.add(var);
    }
    // "Mode 3/A Garbled"
    if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_g_))
    {
        Variable& var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_g_);
        if (!read_set.hasVariable(var))
            read_set.add(var);
    }

    // "Mode 3/A Smoothed"
    if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_v_))
    {
        Variable& var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_v_);
        if (!read_set.hasVariable(var))
            read_set.add(var);
    }
}

bool LabelGenerator::declutterLabels() const
{
    return declutter_labels_;
}

void LabelGenerator::declutterLabels(bool declutter_labels)
{
    declutter_labels_ = declutter_labels;

    if (!declutter_labels_)
        emit labelClearAllSignal(); // since since do not update otherwise - reason unknown

    emit labelOptionsChangedSignal(); // updates
}

bool LabelGenerator::showDeclutteringInfoOnce() const
{
    return show_decluttering_info_once_;
}

void LabelGenerator::showDeclutteringInfoOnce(bool show_decluttering_info_once)
{
    show_decluttering_info_once_ = show_decluttering_info_once;
}

unsigned int LabelGenerator::maxDeclutterlabels() const
{
    return max_declutter_labels_;
}

bool LabelGenerator::filterPrimaryOnlyActive() const
{
    return filter_primary_only_active_;
}

void LabelGenerator::filterPrimaryOnlyActive(bool value)
{
    loginf << "LabelGenerator: filterPrimaryOnlyActive: value " << value;

    filter_primary_only_active_ = value;
}

float LabelGenerator::labelOpacity() const
{
    return label_opacity_;
}

void LabelGenerator::labelOpacity(float label_opacity)
{
    label_opacity_ = label_opacity;
}

float LabelGenerator::labelDistance() const
{
    return label_distance_;
}

void LabelGenerator::labelDistance(float label_distance)
{
    label_distance_ = label_distance;
}

bool LabelGenerator::updateM3AValuesFromStr(const std::string& values)
{
    set<unsigned int> values_tmp;
    vector<string> split_str = String::split(values, ',');
    
    bool ok = true;

    filter_m3a_null_wanted_ = false;

    for (auto& tmp_str : split_str)
    {
        if (String::trim(tmp_str) == "NULL" || String::trim(tmp_str) == "null")
        {
            filter_m3a_null_wanted_ = true;
            continue;
        }

        unsigned int utn_tmp = QString(tmp_str.c_str()).toInt(&ok, 8);

        if (!ok)
        {
            logerr << "DBContentLabelGenerator: updateM3AValuesFromStr: value '" << tmp_str << "' not valid";
            break;
        }

        values_tmp.insert(utn_tmp);
    }

    if (!ok)
        return false;

    filter_m3a_values_set_ = values_tmp;

    return true;
}

bool LabelGenerator::updateTIValuesFromStr(const std::string& values)
{
    set<string> values_tmp;
    vector<string> split_str = String::split(values, ',');

    filter_ti_null_wanted_ = false;

    for (auto& tmp_str : split_str)
    {
        if (String::trim(tmp_str) == "NULL" || String::trim(tmp_str) == "null")
        {
            filter_ti_null_wanted_ = true;
            continue;
        }

        values_tmp.insert(boost::algorithm::to_upper_copy(tmp_str));
    }

    filter_ti_values_set_ = values_tmp;

    return true;
}

bool LabelGenerator::updateTAValuesFromStr(const std::string& values)
{
    set<unsigned int> values_tmp;
    vector<string> split_str = String::split(values, ',');

    bool ok = true;

    filter_ta_null_wanted_ = false;

    for (auto& tmp_str : split_str)
    {
        if (String::trim(tmp_str) == "NULL" || String::trim(tmp_str) == "null")
        {
            filter_ta_null_wanted_ = true;
            continue;
        }

        unsigned int utn_tmp = QString(tmp_str.c_str()).toInt(&ok, 16);

        if (!ok)
        {
            logerr << "DBContentLabelGenerator: updateTAValuesFromStr: value '" << tmp_str << "' not valid";
            break;
        }

        values_tmp.insert(utn_tmp);
    }

    if (!ok)
        return false;

    filter_ta_values_set_ = values_tmp;

    return true;
}

std::string LabelGenerator::getVariableName(const std::string& dbcontent_name, unsigned int key)
{
    assert (key != 0);
    assert (label_config_.contains(dbcontent_name));

    json& dbcont_def = label_config_.at(dbcontent_name);

    if (!dbcont_def.contains(to_string(key)))
        return "";

    string varname = dbcont_def.at(to_string(key));

    return varname;
}

std::string LabelGenerator::getVariableValue(const std::string& dbcontent_name, unsigned int key,
                                             std::shared_ptr<Buffer>& buffer, unsigned int index)
{
    assert (key != 0);
    assert (label_config_.contains(dbcontent_name));

    json& dbcont_def = label_config_.at(dbcontent_name);

    if (!dbcont_def.contains(to_string(key)))
        return "";

    string varname = dbcont_def.at(to_string(key));

    DBContent& db_content = dbcont_manager_.dbContent(dbcontent_name);
    assert (db_content.hasVariable(varname));
    Variable& var = db_content.variable(varname);

    PropertyDataType data_type = var.dataType();
    string value;

    switch (data_type)
    {
    case PropertyDataType::BOOL:
    {
        if (!buffer->has<bool>(varname))
            return "";
        else
        {
            NullableVector<bool>& values = buffer->get<bool>(varname);

            if (values.isNull(index))
                return "";

            if (var.representation() == Variable::Representation::STANDARD)
                value = values.getAsString(index);
            else
                value = var.getAsSpecialRepresentationString(values.get(index));

            return value;
        }

        break;
    }
    case PropertyDataType::CHAR:
    {
        if (!buffer->has<char>(varname))
            return "";
        else
        {
            NullableVector<char>& values = buffer->get<char>(varname);

            if (values.isNull(index))
                return "";

            if (var.representation() == Variable::Representation::STANDARD)
                value = values.getAsString(index);
            else
                value = var.getAsSpecialRepresentationString(values.get(index));

            return value;
        }

        break;
    }
    case PropertyDataType::UCHAR:
    {
        if (!buffer->has<unsigned char>(varname))
            return "";
        else
        {
            NullableVector<unsigned char>& values = buffer->get<unsigned char>(varname);

            if (values.isNull(index))
                return "";

            if (var.representation() == Variable::Representation::STANDARD)
                value = values.getAsString(index);
            else
                value = var.getAsSpecialRepresentationString(values.get(index));

            return value;
        }

        break;
    }
    case PropertyDataType::INT:
    {
        if (!buffer->has<int>(varname))
            return "";
        else
        {
            NullableVector<int>& values = buffer->get<int>(varname);

            if (values.isNull(index))
                return "";

            if (var.representation() == Variable::Representation::STANDARD)
                value = values.getAsString(index);
            else
                value = var.getAsSpecialRepresentationString(values.get(index));

            return value;
        }

        break;
    }
    case PropertyDataType::UINT:
    {
        if (!buffer->has<unsigned int>(varname))
            return "";
        else
        {
            NullableVector<unsigned int>& values = buffer->get<unsigned int>(varname);

            if (values.isNull(index))
                return "";

            if (var.representation() == Variable::Representation::STANDARD)
                value = values.getAsString(index);
            else
                value = var.getAsSpecialRepresentationString(values.get(index));

            return value;
        }

        break;
    }
    case PropertyDataType::LONGINT:
    {
        if (!buffer->has<long int>(varname))
            return "";
        else
        {
            NullableVector<long int>& values = buffer->get<long int>(varname);

            if (values.isNull(index))
                return "";

            if (var.representation() == Variable::Representation::STANDARD)
                value = values.getAsString(index);
            else
                value = var.getAsSpecialRepresentationString(values.get(index));

            return value;
        }

        break;
    }
    case PropertyDataType::ULONGINT:
    {
        if (!buffer->has<unsigned long int>(varname))
            return "";
        else
        {
            NullableVector<unsigned long int>& values = buffer->get<unsigned long int>(varname);

            if (values.isNull(index))
                return "";

            if (var.representation() == Variable::Representation::STANDARD)
                value = values.getAsString(index);
            else
                value = var.getAsSpecialRepresentationString(values.get(index));

            return value;
        }

        break;
    }
    case PropertyDataType::FLOAT:
    {
        if (!buffer->has<float>(varname))
            return "";
        else
        {
            NullableVector<float>& values = buffer->get<float>(varname);

            if (values.isNull(index))
                return "";

            if (var.representation() == Variable::Representation::STANDARD)
                value = values.getAsString(index);
            else
                value = var.getAsSpecialRepresentationString(values.get(index));

            return value;
        }

        break;
    }
    case PropertyDataType::DOUBLE:
    {
        if (!buffer->has<double>(varname))
            return "";
        else
        {
            NullableVector<double>& values = buffer->get<double>(varname);

            if (values.isNull(index))
                return "";

            if (var.representation() == Variable::Representation::STANDARD)
                value = values.getAsString(index);
            else
                value = var.getAsSpecialRepresentationString(values.get(index));

            return value;
        }

        break;
    }
    case PropertyDataType::STRING:
    {
        if (!buffer->has<string>(varname))
            return "";
        else
        {
            NullableVector<string>& values = buffer->get<string>(varname);

            if (values.isNull(index))
                return "";

            value = values.getAsString(index);

            return value;
        }

        break;
    }
    case PropertyDataType::JSON:
    {
        if (!buffer->has<json>(varname))
            return "";
        else
        {
            NullableVector<json>& values = buffer->get<json>(varname);

            if (values.isNull(index))
                return "";

            value = values.getAsString(index);

            return value;
        }

        break;
    }
    case PropertyDataType::TIMESTAMP:
    {
        if (!buffer->has<boost::posix_time::ptime>(varname))
            return "";
        else
        {
            NullableVector<boost::posix_time::ptime>& values = buffer->get<boost::posix_time::ptime>(varname);

            if (values.isNull(index))
                return "";

            assert (var.representation() == Variable::Representation::STANDARD); // only 1 representation
            //value = values.getAsString(index);

            value = Time::toTimeString(values.get(index)); // only display as HH:SS:MM:ZZZ

            return value;
        }

        break;
    }
    default:
        logerr << "LabelGenerator: getVariableValue: impossible for property type "
               << Property::asString(data_type);
        throw std::runtime_error(
                    "LabelGenerator: getVariableValue: impossible property type " +
                    Property::asString(data_type));
    }

}

std::string LabelGenerator::getVariableUnit(const std::string& dbcontent_name, unsigned int key)
{
    assert (key != 0);
    assert (label_config_.contains(dbcontent_name));

    json& dbcont_def = label_config_.at(dbcontent_name);

    if (!dbcont_def.contains(to_string(key)))
        return "";

    string varname = dbcont_def.at(to_string(key));

    DBContent& db_content = dbcont_manager_.dbContent(dbcontent_name);

    assert (db_content.hasVariable(varname));

    return db_content.variable(varname).dimensionUnitStr();
}

std::string LabelGenerator::getMode3AText (const std::string& dbcontent_name,
                                           unsigned int buffer_index, std::shared_ptr<Buffer>& buffer)
{
    string text;

    Variable& m3a_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_);

    if (buffer->has<unsigned int>(m3a_var.name()) &&
                     !buffer->get<unsigned int>(m3a_var.name()).isNull(buffer_index))
    {
        text = m3a_var.getAsSpecialRepresentationString(
                    buffer->get<unsigned int>(m3a_var.name()).get(buffer_index));

        bool valid=false, garbled=false, smoothed=false;

        // "Mode 3/A Valid"
        if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_v_))
        {
            Variable& var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_v_);

            if (buffer->has<bool>(var.name()) && !buffer->get<bool>(var.name()).isNull(buffer_index))
                valid = buffer->get<bool>(var.name()).get(buffer_index);
        }
        // "Mode 3/A Garbled"
        if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_g_))
        {
            Variable& var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_g_);

            if (buffer->has<bool>(var.name()) && !buffer->get<bool>(var.name()).isNull(buffer_index))
                garbled = buffer->get<bool>(var.name()).get(buffer_index);
        }

        // "Mode 3/A Smoothed"
        if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_smoothed_))
        {
            Variable& var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_smoothed_);

            if (buffer->has<bool>(var.name()) && !buffer->get<bool>(var.name()).isNull(buffer_index))
                smoothed = buffer->get<bool>(var.name()).get(buffer_index);
        }

        if (!valid || garbled || smoothed)
            text += " ";

        if (!valid)
            text += "I";

        if (garbled)
            text += "G";

        if (smoothed)
            text += "S";
    }

    return text;
}
std::string LabelGenerator::getModeCText (const std::string& dbcontent_name,
                                          unsigned int buffer_index, std::shared_ptr<Buffer>& buffer)
{
    string text;

    Variable& mc_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_);

    if (buffer->has<float>(mc_var.name()) &&
                     !buffer->get<float>(mc_var.name()).isNull(buffer_index))
    {
        text = String::doubleToStringPrecision(buffer->get<float>(mc_var.name()).get(buffer_index)/100.0,2);

        bool valid=false, garbled=false;

        // "Mode CValid"
        if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_v_))
        {
            Variable& var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_v_);

            if (buffer->has<bool>(var.name()) && !buffer->get<bool>(var.name()).isNull(buffer_index))
                valid = buffer->get<bool>(var.name()).get(buffer_index);
        }
        // "Mode C Garbled"
        if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_g_))
        {
            Variable& var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_g_);

            if (buffer->has<bool>(var.name()) && !buffer->get<bool>(var.name()).isNull(buffer_index))
                garbled = buffer->get<bool>(var.name()).get(buffer_index);
        }

        if (!valid || garbled)
            text += " ";

        if (!valid)
            text += "I";

        if (garbled)
            text += "G";
    }

    return text;
}

}
