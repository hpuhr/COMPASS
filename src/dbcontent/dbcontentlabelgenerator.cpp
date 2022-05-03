#include "dbcontentlabelgenerator.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontentlabelgeneratorwidget.h"
#include "logger.h"
#include "util/stringconv.h"

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
        Variable* c_d_var {nullptr};
        if (dbcont_manager_.metaCanGetVariable(dbcontent_name, DBContent::meta_var_climb_descent_))
            c_d_var = &dbcont_manager_.metaGetVariable(dbcontent_name, DBContent::meta_var_climb_descent_);

        string c_d;

        if (c_d_var && buffer->has<unsigned char>(c_d_var->name()) &&
                !buffer->get<unsigned char>(c_d_var->name()).isNull(buffer_index))
            c_d = c_d_var->getAsSpecialRepresentationString((buffer->get<unsigned char>(c_d_var->name()).get(buffer_index)));

        tmp.push_back(c_d);

        // 3,2
        {
            bool calc_vx_vy;
            string var1, var2;
            bool cant_calculate = false;
            double speed_ms;

            if (dbcont_manager_.metaVariable(DBContent::meta_var_vx_.name()).existsIn(dbcontent_name)
                    && buffer->has<double>(
                        dbcont_manager_.metaVariable(DBContent::meta_var_vx_.name()).getFor(dbcontent_name).name())
                    && dbcont_manager_.metaVariable(DBContent::meta_var_vy_.name()).existsIn(dbcontent_name)
                    && buffer->has<double>(
                        dbcont_manager_.metaVariable(DBContent::meta_var_vy_.name()).getFor(dbcontent_name).name()))
            {
                // calculate based on vx, vy
                calc_vx_vy = true;

                var1 = dbcont_manager_.metaVariable(DBContent::meta_var_vx_.name()).getFor(dbcontent_name).name();
                var2 = dbcont_manager_.metaVariable(DBContent::meta_var_vy_.name()).getFor(dbcontent_name).name();
            }
            else if (dbcont_manager_.metaVariable(DBContent::meta_var_ground_speed_.name()).existsIn(dbcontent_name)
                     && buffer->has<double>(
                         dbcont_manager_.metaVariable(DBContent::meta_var_ground_speed_.name()).getFor(dbcontent_name).name()))
            {
                // calculate based on spd, track angle
                calc_vx_vy = false;

                var1 = dbcont_manager_.metaVariable(DBContent::meta_var_ground_speed_.name()).getFor(dbcontent_name).name();
            }
            else
                cant_calculate = true;

            if (cant_calculate)
                tmp.push_back(""); // cant
            else
            {
                if (calc_vx_vy)
                {
                    NullableVector<double>& vxs = buffer->get<double>(var1);
                    NullableVector<double>& vys = buffer->get<double>(var2);

                    if (!vxs.isNull(buffer_index) && !vys.isNull(buffer_index))
                    {
                        speed_ms = sqrt(pow(vxs.get(buffer_index), 2)+pow(vys.get(buffer_index), 2));
                        tmp.push_back(String::doubleToStringPrecision(speed_ms * M_S2KNOTS, 2));
                    }
                    else
                        tmp.push_back(""); // cant
                }
                else
                {
                    NullableVector<double>& speeds = buffer->get<double>(var1);

                    if (!speeds.isNull(buffer_index))
                    {
                        speed_ms = speeds.get(buffer_index);
                        tmp.push_back(String::doubleToStringPrecision(speed_ms, 2)); // should be kts
                    }
                    else
                        tmp.push_back(""); // cant
                }
            }
        }

        // 3,3

        if (dbcontent_name == "CAT062" && buffer->has<string>(DBContent::var_cat062_wtc_.name())
                && !buffer->get<string>(DBContent::var_cat062_wtc_.name()).isNull(buffer_index))
            tmp.push_back(buffer->get<string>(DBContent::var_cat062_wtc_.name()).get(buffer_index));
        else
            tmp.push_back("");

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

//void DBContentLabelGenerator::registerLeafItemLabel (GeometryLeafItemLabels& item_label)
//{
//    assert (!item_labels_.count(&item_label));
//    item_labels_.insert(&item_label);
//}

//void DBContentLabelGenerator::unregisterLeafItemLabel (GeometryLeafItemLabels& item_label)
//{
//    assert (item_labels_.count(&item_label));
//    item_labels_.erase(&item_label);
//}

void DBContentLabelGenerator::autoAdustCurrentLOD(unsigned int num_labels_on_screen)
{
//    assert (viewport.size() == 4);

//    loginf << "DBContentLabelGenerator: autoAdustCurrentLOD: x " << viewport.at(0)
//           << " y " << viewport.at(1) << " w " << viewport.at(2) << " h " << viewport.at(3);

//    osg::Vec4 pos;
//    int x, y;

//    bool inside;

//    unsigned int num_labels = 0;
//    unsigned int num_labels_on_screen = 0;

//    for (auto& it_lab_it : item_labels_)
//    {
//        for (auto& pos_it : it_lab_it->getLabelPositions())
//        {
//            ++num_labels;

//            pos = pos_it;
//            pos = pos * screen_transform;
//            pos = pos / pos.w();

//            x = pos.x();
//            y = pos.y();

//            inside = x >= viewport.at(0) && x < viewport.at(2) && y >= viewport.at(1) && y < viewport.at(3);

//            logdbg << "DBContentLabelGenerator: autoAdustCurrentLOD: x " << x << " y " << y << " inside " << inside;

//            if (inside)
//                ++num_labels_on_screen;
//        }
//    }

    loginf << "DBContentLabelGenerator: autoAdustCurrentLOD: num labels on screen "
           << num_labels_on_screen;

    if (num_labels_on_screen < 20)
        current_lod_ = 3;
    else if (num_labels_on_screen < 40)
        current_lod_ = 2;
    else
        current_lod_ = 1;
}

unsigned int DBContentLabelGenerator::currentLOD() const
{
    return current_lod_;
}

void DBContentLabelGenerator::currentLOD(unsigned int current_lod)
{
    current_lod_ = current_lod;
}


DBContentLabelGeneratorWidget& DBContentLabelGenerator::widget()
{
    if (!widget_)
    {
        widget_.reset(new DBContentLabelGeneratorWidget(*this));
    }

    return *widget_.get();
}

bool DBContentLabelGenerator::autoLOD() const
{
    return auto_lod_;
}

void DBContentLabelGenerator::autoLOD(bool auto_lod)
{
    auto_lod_ = auto_lod;
}

void DBContentLabelGenerator::addLabelDSID(unsigned int ds_id)
{
    label_ds_ids_.insert(ds_id);
}

void DBContentLabelGenerator::removeLabelDSID(unsigned int ds_id)
{
    assert (label_ds_ids_.count(ds_id));
    label_ds_ids_.erase(ds_id);
}

const std::set<unsigned int>& DBContentLabelGenerator::labelDSIDs() const
{
    return label_ds_ids_;
}

void DBContentLabelGenerator::checkSubConfigurables()
{
    // nothing to see here
}
