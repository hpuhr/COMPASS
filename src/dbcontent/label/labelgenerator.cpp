#include "dbcontent/label/labelgenerator.h"
#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/label/labelgeneratorwidget.h"
#include "dbcontent/label/labelcontentdialog.h"
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
    registerParameter("label_config", &label_config_, json::object());

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
    registerParameter("filter_ta_values", &filter_ta_values_, "AADDCCDD");
    updateTAValuesFromStr(filter_ta_values_);

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

    // 1,2
    string acid;

    if (acid_var && buffer->has<string>(acid_var->name())
            && !buffer->get<string>(acid_var->name()).isNull(buffer_index))
        acid = buffer->get<string>(acid_var->name()).get(buffer_index);

    acid.erase(std::remove(acid.begin(), acid.end(), ' '), acid.end());
    tmp.push_back(acid);

    if (current_lod_ == 3)
    {
        // 1,3
        tmp.push_back(getVariableValue(dbcontent_name, 0*3+2, buffer, buffer_index));
    }

    // row 2

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

    if (current_lod_ == 2)
        return tmp;

    // 2,3
    tmp.push_back(getVariableValue(dbcontent_name, 1*3+2, buffer, buffer_index));

    // row 3

    // 3,1
    tmp.push_back(getVariableValue(dbcontent_name, 2*3+0, buffer, buffer_index));

    {


//        bool calc_vx_vy;
//        string var1, var2;
//        bool cant_calculate = false;
//        double speed_ms;

//        if (dbcont_manager_.metaVariable(DBContent::meta_var_vx_.name()).existsIn(dbcontent_name)
//                && buffer->has<double>(
//                    dbcont_manager_.metaVariable(DBContent::meta_var_vx_.name()).getFor(dbcontent_name).name())
//                && dbcont_manager_.metaVariable(DBContent::meta_var_vy_.name()).existsIn(dbcontent_name)
//                && buffer->has<double>(
//                    dbcont_manager_.metaVariable(DBContent::meta_var_vy_.name()).getFor(dbcontent_name).name()))
//        {
//            // calculate based on vx, vy
//            calc_vx_vy = true;

//            var1 = dbcont_manager_.metaVariable(DBContent::meta_var_vx_.name()).getFor(dbcontent_name).name();
//            var2 = dbcont_manager_.metaVariable(DBContent::meta_var_vy_.name()).getFor(dbcontent_name).name();
//        }
//        else if (dbcont_manager_.metaVariable(DBContent::meta_var_ground_speed_.name()).existsIn(dbcontent_name)
//                 && buffer->has<double>(
//                     dbcont_manager_.metaVariable(DBContent::meta_var_ground_speed_.name()).getFor(dbcontent_name).name()))
//        {
//            // calculate based on spd, track angle
//            calc_vx_vy = false;

//            var1 = dbcont_manager_.metaVariable(DBContent::meta_var_ground_speed_.name()).getFor(dbcontent_name).name();
//        }
//        else
//            cant_calculate = true;

//        if (cant_calculate)
//            tmp.push_back(""); // cant
//        else
//        {
//            if (calc_vx_vy)
//            {
//                NullableVector<double>& vxs = buffer->get<double>(var1);
//                NullableVector<double>& vys = buffer->get<double>(var2);

//                if (!vxs.isNull(buffer_index) && !vys.isNull(buffer_index))
//                {
//                    speed_ms = sqrt(pow(vxs.get(buffer_index), 2)+pow(vys.get(buffer_index), 2));
//                    tmp.push_back(String::doubleToStringPrecision(speed_ms * M_S2KNOTS, 2));
//                }
//                else
//                    tmp.push_back(""); // cant
//            }
//            else
//            {
//                NullableVector<double>& speeds = buffer->get<double>(var1);

//                if (!speeds.isNull(buffer_index))
//                {
//                    speed_ms = speeds.get(buffer_index);
//                    tmp.push_back(String::doubleToStringPrecision(speed_ms, 2)); // should be kts
//                }
//                else
//                    tmp.push_back(""); // cant
//            }
//        }
    }

    // 3,2
    tmp.push_back(getVariableValue(dbcontent_name, 2*3+1, buffer, buffer_index));

//    Variable* c_d_var {nullptr};
//    if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_climb_descent_))
//        c_d_var = &dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_climb_descent_);

//    string c_d;

//    if (c_d_var && buffer->has<unsigned char>(c_d_var->name()) &&
//            !buffer->get<unsigned char>(c_d_var->name()).isNull(buffer_index))
//        c_d = c_d_var->getAsSpecialRepresentationString((buffer->get<unsigned char>(c_d_var->name()).get(buffer_index)));

//    tmp.push_back(c_d);

    // 3,3

    tmp.push_back(getVariableValue(dbcontent_name, 2*3+2, buffer, buffer_index));

//    if (dbcontent_name == "CAT062" && buffer->has<string>(DBContent::var_cat062_wtc_.name())
//            && !buffer->get<string>(DBContent::var_cat062_wtc_.name()).isNull(buffer_index))
//        tmp.push_back(buffer->get<string>(DBContent::var_cat062_wtc_.name()).get(buffer_index));
//    else
//        tmp.push_back("");

    //        Variable& tod_var = dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_tod_);
    //        string tod;

    //        if (buffer->has<float>(tod_var.name()) &&
    //                !buffer->get<float>(tod_var.name()).isNull(buffer_index))
    //            tod = String::timeStringFromDouble(buffer->get<float>(tod_var.name()).get(buffer_index));

    //        tmp.push_back(tod);

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
    loginf << "DBContentLabelGenerator: autoAdustCurrentLOD: num labels on screen "
           << num_labels_on_screen;

    if (num_labels_on_screen < 20)
        current_lod_ = 3;
    else if (num_labels_on_screen < 40)
        current_lod_ = 2;
    else
        current_lod_ = 1;
}

unsigned int LabelGenerator::currentLOD() const
{
    return current_lod_;
}

void LabelGenerator::currentLOD(unsigned int current_lod)
{
    current_lod_ = current_lod;
}


LabelGeneratorWidget& LabelGenerator::widget()
{
    if (!widget_)
    {
        widget_.reset(new LabelGeneratorWidget(*this));
    }

    return *widget_.get();
}

bool LabelGenerator::autoLOD() const
{
    return auto_lod_;
}

void LabelGenerator::autoLOD(bool auto_lod)
{
    auto_lod_ = auto_lod;
}

void LabelGenerator::addLabelDSID(unsigned int ds_id)
{
    label_ds_ids_.insert(ds_id);

    emit labelOptionsChangedSignal();
}

void LabelGenerator::removeLabelDSID(unsigned int ds_id)
{
    assert (label_ds_ids_.count(ds_id));
    label_ds_ids_.erase(ds_id);

    emit labelOptionsChangedSignal();
}

const std::set<unsigned int>& LabelGenerator::labelDSIDs() const
{
    return label_ds_ids_;
}

bool LabelGenerator::labelWanted(unsigned int ds_id)
{
    return label_ds_ids_.count(ds_id);
}

bool LabelGenerator::labelWanted(std::shared_ptr<Buffer> buffer, unsigned int index)
{
    if (filter_mode3a_active_)
    {
        if (!dbcont_manager_.metaCanGetVariable(buffer->dbContentName(), DBContent::meta_var_m3a_))
            return false;

        dbContent::Variable& var = dbcont_manager_.metaGetVariable(buffer->dbContentName(), DBContent::meta_var_m3a_);

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
        if (!dbcont_manager_.metaCanGetVariable(buffer->dbContentName(), DBContent::meta_var_mc_))
            return false;

        dbContent::Variable& var = dbcont_manager_.metaGetVariable(buffer->dbContentName(), DBContent::meta_var_mc_);

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
        if (!dbcont_manager_.metaCanGetVariable(buffer->dbContentName(), DBContent::meta_var_ti_))
            return false;

        dbContent::Variable& var = dbcont_manager_.metaGetVariable(buffer->dbContentName(), DBContent::meta_var_ti_);

        assert (buffer->has<string> (var.name()));

        NullableVector<string>& data_vec = buffer->get<string> (var.name());

        if (data_vec.isNull(index))
        {
            if (!filter_ti_null_wanted_)
                return false; // null not wanted
        }
        else
        {
            bool found = false;

            for (auto& val_it : filter_ti_values_set_)
            {
                if (data_vec.get(index).find(val_it) != std::string::npos)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
                return false;
        }
    }

    if (filter_ta_active_)
    {
        if (!dbcont_manager_.metaCanGetVariable(buffer->dbContentName(), DBContent::meta_var_ta_))
            return false;

        dbContent::Variable& var = dbcont_manager_.metaGetVariable(buffer->dbContentName(), DBContent::meta_var_ta_);

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
    default:
        logerr << "LabelGenerator: getVariableValue: impossible for property type "
                 << Property::asString(data_type);
        throw std::runtime_error(
                    "LabelGenerator: getVariableValue: impossible property type " +
                    Property::asString(data_type));
    }

}

}
