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

#include "variableviewstashdatawidget.h"
#include "viewvariable.h"
#include "variableview.h"

#include "buffer.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "datasourcemanager.h"
#include "stringconv.h"
#include "number.h"

#include "logger.h"
#include "property_templates.h"

#include <algorithm>

#include <QApplication>

using namespace std;
using namespace Utils;

/**
*/
VariableViewStashDataWidget::VariableViewStashDataWidget(ViewWidget* view_widget,
                                                         VariableView* view,
                                                         bool group_per_datasource,
                                                         QWidget* parent, 
                                                         Qt::WindowFlags f)
    :   VariableViewDataWidget(view_widget, view, parent, f)
    ,   stash_                (view->numVariables())
    ,   group_per_datasource_(group_per_datasource)
{
}

/**
*/
VariableViewStashDataWidget::~VariableViewStashDataWidget() = default;

/**
*/
unsigned int VariableViewStashDataWidget::nullValueCount() const
{
    return stash_.nan_value_count_;
}

/**
*/
void VariableViewStashDataWidget::resetVariableData()
{
    //reset stash data
    resetStash();
}

/**
*/
void VariableViewStashDataWidget::resetIntermediateVariableData()
{
    //clear the stash itself
    resetStash();

    //clear data generated from the stash during redraw recomputation
    resetStashDependentData();
}

/**
*/
void VariableViewStashDataWidget::preUpdateVariableDataEvent()
{
    //nothing to do here (yet)
}

/**
*/
void VariableViewStashDataWidget::postUpdateVariableDataEvent() 
{
    //update the stash (bounds, counts, etc.)
    updateStash();

    //check the stash integrity
    assert(stash_.isValid());

    //invoke derived to process stash data
    processStash(stash_);
}

/**
*/
void VariableViewStashDataWidget::updateVariableData(const std::string& dbcontent_name,
                                                     Buffer& buffer)
{
    auto view = variableView();

    loginf << "VariableViewStashDataWidget: updateVariableData: updating data for view " << view->classId();

    for (size_t i = 0; i < view->numVariables(); ++i)
    {
        const ViewVariable& view_var = variableView()->variable(i);

        if (!view_var.getFor(dbcontent_name))
        {
            loginf << "VariableViewStashDataWidget: updateVariableData: view_var " << view_var.variableName()
                   << " not existing in " << dbcontent_name << ", skipping";
            return;
        }
    }

    unsigned int last_size = last_buffer_size_[dbcontent_name];
    unsigned int current_size = buffer.size();

    map<string, vector<unsigned int>> grouped_indexes; // group name -> indexes
    std::string group_name;

    if (group_per_datasource_) // add by DS ID + Line ID
    {
        DBContentManager&  dbcontent_man = COMPASS::instance().dbContentManager();
        DataSourceManager& ds_man        = COMPASS::instance().dataSourceManager();

        assert(buffer.has<unsigned int>(
            dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ds_id_).name()));
        assert(buffer.has<unsigned int>(
            dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_).name()));

        const NullableVector<unsigned int>& ds_ids = buffer.get<unsigned int>(
            dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ds_id_).name());

        const NullableVector<unsigned int>& line_ids = buffer.get<unsigned int>(
            dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_).name());

        unsigned int ds_id, line_id;

        const NullableVector<unsigned char>* sensor_sacs {nullptr};
        const NullableVector<unsigned char>* sensor_sics {nullptr};

        if (dbcontent_name == "CAT063") // use sensor sec/sic special case
        {
            assert(buffer.has<unsigned char>(
                dbcontent_man.getVariable(dbcontent_name, DBContent::var_cat063_sensor_sac_).name()));
            assert(buffer.has<unsigned char>(
                dbcontent_man.getVariable(dbcontent_name, DBContent::var_cat063_sensor_sic_).name()));

            sensor_sacs = &buffer.get<unsigned char>(
                dbcontent_man.getVariable(dbcontent_name, DBContent::var_cat063_sensor_sac_).name());
            sensor_sics = &buffer.get<unsigned char>(
                dbcontent_man.getVariable(dbcontent_name, DBContent::var_cat063_sensor_sic_).name());
        }

        for (unsigned int index = last_size; index < current_size; ++index)
        {
            assert (!ds_ids.isNull(index));
            assert (!line_ids.isNull(index));

            ds_id = ds_ids.get(index);
            line_id = line_ids.get(index);

            if (sensor_sacs && sensor_sics)
            {
                if(!sensor_sacs->isNull(index) && !sensor_sics->isNull(index))
                    ds_id = Number::dsIdFrom(sensor_sacs->get(index), sensor_sics->get(index));
                else
                    continue;
            }

            if (!ds_man.hasDBDataSource(ds_id))
                group_name = to_string(Number::sacFromDsId(ds_id))+"/"+to_string(Number::sicFromDsId(ds_id));
            else
                group_name = ds_man.dbDataSource(ds_id).name();

            group_name +=  " " + String::lineStrFrom(line_id);

            grouped_indexes[group_name].push_back(index);
        }
    }
    else // simple add
    {
        group_name = dbcontent_name;

        for (unsigned int index = last_size; index < current_size; ++index)
            grouped_indexes[dbcontent_name].push_back(index);
    }

    // add selected flags & rec_nums
    assert (buffer.has<bool>(DBContent::selected_var.name()));
    assert (buffer.has<unsigned long>(DBContent::meta_var_rec_num_.name()));

    const NullableVector<bool>&          selected_vec = buffer.get<bool>(DBContent::selected_var.name());
    const NullableVector<unsigned long>& rec_num_vec  = buffer.get<unsigned long>(DBContent::meta_var_rec_num_.name());

    for (auto& group_it : grouped_indexes)
    {
        group_name = group_it.first;

        if (!group_it.second.size())
            continue;

        auto& group_stash = stash_.groupedDataStash(group_name);

        logdbg << "VariableViewStashDataWidget: updateVariableData: value counts before:";

        for (size_t i = 0; i < view->numVariables(); ++i)
            logdbg << "   " << view->variable(i).id() << " " << group_stash.variable_stashes[ i ].values.size();

        std::vector<bool>&          selected_data = group_stash.selected_values;
        std::vector<unsigned long>& rec_num_data  = group_stash.record_numbers;

        //auto& x_stash = group_stash.variable_stashes[ 0 ];

        //unsigned int last_size = x_stash.count;

        for (auto index : group_it.second)
        {
            if (selected_vec.isNull(index))
                selected_data.push_back(false);
            else
                selected_data.push_back(selected_vec.get(index));

            assert (!rec_num_vec.isNull(index));
            rec_num_data.push_back(rec_num_vec.get(index));
        }

        //collect data for all variables
        for (size_t i = 0; i < view->numVariables(); ++i)
            updateVariableData(i, group_name, buffer, group_it.second);

        //check counts
        assert (stash_.isValid());

        logdbg << "VariableViewStashDataWidget: updateVariableData: value counts after:";

        for (size_t i = 0; i < view->numVariables(); ++i)
            logdbg << "   " << view->variable(i).id() << " " << group_stash.variable_stashes[ i ].values.size();
    }

    last_buffer_size_[dbcontent_name] = current_size;
}

/**
*/
void VariableViewStashDataWidget::updateVariableData(
    size_t var_idx, std::string group_name, const Buffer& buffer,
    const std::vector<unsigned int>& indexes)
{
    logdbg << "VariableViewStashDataWidget: updateVariableData: group_name "
           << group_name << " indexes size " << indexes.size();

    auto& group_stash = stash_.groupedDataStash(group_name);
    auto& var_stash   = group_stash.variable_stashes.at(var_idx);

    auto& buffer_counts = var_stash.count;
    auto& values        = var_stash.values;

    unsigned int last_size = buffer_counts;

    const ViewVariable& view_var = variableView()->variable(var_idx);
    const dbContent::Variable* data_var = view_var.getFor(buffer.dbContentName());

    if (!data_var)
    {
        logwrn << "VariableViewStashDataWidget: updateVariableData: could not retrieve data var";
        return;
    }

    PropertyDataType data_type        = data_var->dataType();
    std::string      current_var_name = data_var->name();

    logdbg << "VariableViewStashDataWidget: updateVariableData: updating, last size " << last_size;

#define UpdateFunc(PDType, DType)                                                \
        assert(view_var.settings().valid_data_types.count(PDType) != 0);         \
        assert(buffer.has<DType>(current_var_name));                             \
                                                                                 \
        const NullableVector<DType>& data = buffer.get<DType>(current_var_name); \
                                                                                 \
        appendData(data, values, indexes);                                       \
        //buffer_counts = current_size;

#define NotFoundFunc                                                                                                             \
    logerr << "VariableViewStashDataWidget: updateVariableData: impossible for property type " \
           << Property::asString(data_type); \
        throw std::runtime_error("VariableViewStashDataWidget: updateVariableData: impossible property type " \
                                 + Property::asString(data_type));

    SwitchPropertyDataType(data_type, UpdateFunc, NotFoundFunc)

    logdbg << "VariableViewStashDataWidget: updateVariableData: updated size " << buffer_counts;

}

/**
*/
void VariableViewStashDataWidget::resetStash()
{
    stash_.reset();
    last_buffer_size_.clear();
}

/**
*/
void VariableViewStashDataWidget::updateStash()
{
    stash_.update();

    const auto& data_ranges = stash_.dataRanges();

    logdbg << "VariableViewStashDataWidget: updateMinMax: ";

    for (size_t i = 0; i < variableView()->numVariables(); ++i)
    {
        logdbg << "   " << variableView()->variable(i).id() << " "
               << "has_min_max " << data_ranges[ i ].has_value() << " "
               << "x_min " << (data_ranges[ i ].has_value() ? std::to_string(data_ranges[ i ]->first) : "-") << " "
               << "x_max " << (data_ranges[ i ].has_value() ? std::to_string(data_ranges[ i ]->second) : "-");
    }
}

/**
 */
QRectF VariableViewStashDataWidget::getPlanarBounds(int var_x, int var_y) const
{
    const auto& data_bounds = getStash().dataRanges();

    if (!data_bounds[ var_x ].has_value() || !data_bounds[ var_y ].has_value())
        return QRectF();

    double xmin = data_bounds[ var_x ].value().first;
    double xmax = data_bounds[ var_x ].value().second;
    double ymin = data_bounds[ var_y ].value().first;
    double ymax = data_bounds[ var_y ].value().second;

    return QRectF(xmin, ymin, xmax - xmin, ymax - ymin);
}

/**
 */
boost::optional<std::pair<double, double>> VariableViewStashDataWidget::getBounds(int var) const
{
    const auto& data_bounds = getStash().dataRanges();

    return data_bounds.at(var);
}

/**
 */
void VariableViewStashDataWidget::selectData(double x_min, 
                                             double x_max, 
                                             double y_min, 
                                             double y_max,
                                             int var_x,
                                             int var_y)
{
    bool ctrl_pressed = QApplication::keyboardModifiers() & Qt::ControlModifier;

    loginf << "VariableViewStashDataWidget: selectData: "
           << "x_min " << x_min << " "
           << "x_max " << x_max << " "
           << "y_min " << y_min << " "
           << "y_max " << y_max << " "
           << "var_x " << var_x << " "
           << "var_y " << var_y << " "
           << "ctrl pressed " << ctrl_pressed;
    
    unsigned int sel_cnt = 0;

    for (auto& buf_it : viewData())
    {
        assert (buf_it.second->has<bool>(DBContent::selected_var.name()));
        NullableVector<bool>& selected_vec = buf_it.second->get<bool>(DBContent::selected_var.name());

        assert (buf_it.second->has<unsigned long>(DBContent::meta_var_rec_num_.name()));
        NullableVector<unsigned long>& rec_num_vec = buf_it.second->get<unsigned long>(
            DBContent::meta_var_rec_num_.name());

        std::map<unsigned long, std::vector<unsigned int>> rec_num_indexes =
            rec_num_vec.distinctValuesWithIndexes(0, rec_num_vec.size());
        // rec_num -> index

        assert (getStash().hasGroupStash(buf_it.first));
        const auto& dbc_stash = getStash().groupStash(buf_it.first);

        const std::vector<double>&        x_values       = dbc_stash.variable_stashes[ var_x ].values;
        const std::vector<double>&        y_values       = dbc_stash.variable_stashes[ var_y ].values;
        const std::vector<unsigned long>& rec_num_values = dbc_stash.record_numbers;

        assert (x_values.size() == y_values.size());
        assert (x_values.size() == rec_num_values.size());

        double x, y;
        bool in_range;
        unsigned long rec_num, index;

        for (unsigned int cnt=0; cnt < x_values.size(); ++cnt)
        {
            x = x_values.at(cnt);
            y = y_values.at(cnt);
            rec_num = rec_num_values.at(cnt);

            in_range = false;

            if (!std::isnan(x) && !std::isnan(y))
                in_range =  x >= x_min && x <= x_max && y >= y_min && y <= y_max;

            assert (rec_num_indexes.count(rec_num));
            std::vector<unsigned int>& indexes = rec_num_indexes.at(rec_num);
            assert (indexes.size() == 1);

            index = indexes.at(0);

            if (ctrl_pressed && !selected_vec.isNull(index) && selected_vec.get(index))
                in_range = true; // add selection to existing

            selected_vec.set(index, in_range);

            if (in_range)
                ++sel_cnt;
        }
    }

    loginf << "VariableViewStashDataWidget: selectData: sel_cnt " << sel_cnt;

    emit variableView()->selectionChangedSignal();
}

/**
 */
void VariableViewStashDataWidget::viewInfoJSON_impl(nlohmann::json& info) const
{
    //!call base!
    VariableViewDataWidget::viewInfoJSON_impl(info);

    info[ "num_valid"   ] = stash_.valid_count_;
    info[ "num_selected"] = stash_.selected_count_;
    info[ "num_nan"     ] = stash_.nan_value_count_;
    
    nlohmann::json input_value_info = nlohmann::json::array();

    for (const auto& it : stash_.groupedStashes())
    {
        nlohmann::json dbo_info;

        dbo_info[ "dbo_type"       ] = it.first;

        for (size_t i = 0; i < variableView()->numVariables(); ++i)
            dbo_info[ "num_input_" + variableView()->variable(i).id() ] = it.second.variable_stashes[ i ].values.size();

        dbo_info[ "num_input_valid"] = it.second.valid_count;

        input_value_info.push_back(dbo_info);
    }

    info[ "input_values" ] = input_value_info;
}
