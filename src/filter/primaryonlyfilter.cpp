#include "primaryonlyfilter.h"
#include "primaryonlyfilterwidget.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "compass.h"

using namespace std;
using namespace nlohmann;

PrimaryOnlyFilter::PrimaryOnlyFilter(const std::string& class_id, const std::string& instance_id,
                                     Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{

    createSubConfigurables();
}

PrimaryOnlyFilter::~PrimaryOnlyFilter()
{

}

bool PrimaryOnlyFilter::filters(const std::string& dbo_name)
{
    DBContentManager& cont_man = COMPASS::instance().dbContentManager();

    if (cont_man.metaCanGetVariable(dbo_name, DBContent::meta_var_m3a_))
        return true;

    if (cont_man.metaCanGetVariable(dbo_name, DBContent::meta_var_mc_))
        return true;

    if (cont_man.metaCanGetVariable(dbo_name, DBContent::meta_var_ta_))
        return true;

    if (cont_man.metaCanGetVariable(dbo_name, DBContent::meta_var_ti_))
        return true;

    return false;
}

std::string PrimaryOnlyFilter::getConditionString(const std::string& dbo_name, bool& first,
                                           std::vector<std::string>& extra_from_parts,
                                           std::vector<dbContent::Variable*>& filtered_variables)
{
    logdbg << "PrimaryOnlyFilter: getConditionString: dbo " << dbo_name << " active " << active_;

    stringstream ss;

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();

    if (cont_man.metaCanGetVariable(dbo_name, DBContent::meta_var_m3a_))
    {
        dbContent::Variable& var = cont_man.metaGetVariable(dbo_name, DBContent::meta_var_m3a_);
        filtered_variables.push_back(&var);

        if (!first)
            ss << " AND";

        ss << " " + var.dbColumnName() << " IS NULL";

        first = false;
    }

    if (cont_man.metaCanGetVariable(dbo_name, DBContent::meta_var_mc_))
    {
        dbContent::Variable& var = cont_man.metaGetVariable(dbo_name, DBContent::meta_var_mc_);
        filtered_variables.push_back(&var);

        if (!first)
            ss << " AND";

        ss << " " + var.dbColumnName() << " IS NULL";

        first = false;
    }

    if (cont_man.metaCanGetVariable(dbo_name, DBContent::meta_var_ta_))
    {
        dbContent::Variable& var = cont_man.metaGetVariable(dbo_name, DBContent::meta_var_ta_);
        filtered_variables.push_back(&var);

        if (!first)
            ss << " AND";

        ss << " " + var.dbColumnName() << " IS NULL";

        first = false;
    }

    if (cont_man.metaCanGetVariable(dbo_name, DBContent::meta_var_ti_))
    {
        dbContent::Variable& var = cont_man.metaGetVariable(dbo_name, DBContent::meta_var_ti_);
        filtered_variables.push_back(&var);

        if (!first)
            ss << " AND";

        ss << " " + var.dbColumnName() << " IS NULL";

        first = false;
    }


    logdbg << "PrimaryOnlyFilter: getConditionString: here '" << ss.str() << "'";

    return ss.str();
}

void PrimaryOnlyFilter::generateSubConfigurable(const std::string& class_id, const std::string& instance_id)
{
    logdbg << "PrimaryOnlyFilter: generateSubConfigurable: class_id " << class_id;

    throw std::runtime_error("PrimaryOnlyFilter: generateSubConfigurable: unknown class_id " + class_id);
}

void PrimaryOnlyFilter::checkSubConfigurables()
{
    logdbg << "PrimaryOnlyFilter: checkSubConfigurables";
}

DBFilterWidget* PrimaryOnlyFilter::createWidget()
{
    return new PrimaryOnlyFilterWidget(*this);
}


void PrimaryOnlyFilter::reset()
{
    if (widget_)
        widget_->update();
}

void PrimaryOnlyFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (!filters.contains(name_));
    filters[name_] = json::object();
//    json& filter = filters.at(name_);

//    filter["Aircraft Address Values"] = values_str_;
}

void PrimaryOnlyFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (filters.contains(name_));
//    const json& filter = filters.at(name_);

//    assert (filter.contains("Aircraft Address Values"));
//    values_str_ = filter.at("Aircraft Address Values");

//    updateValuesFromStr(values_str_);

//    if (widget())
//        widget()->update();
}

bool PrimaryOnlyFilter::activeInLiveMode()
{
    return true;
}

std::vector<size_t> PrimaryOnlyFilter::filterBuffer(const std::string& dbcontent_name, std::shared_ptr<Buffer> buffer)
{
    std::vector<size_t> to_be_removed;

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();

    NullableVector<unsigned int>* m3a_vec {nullptr};
    if (cont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_))
    {
        dbContent::Variable& var = cont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_);
        assert (buffer->has<unsigned int> (var.name()));
        m3a_vec = &buffer->get<unsigned int> (var.name());
    }

    NullableVector<float>* mc_vec {nullptr};
    if (cont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_))
    {
        dbContent::Variable& var = cont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_);
        assert (buffer->has<float> (var.name()));
        mc_vec = &buffer->get<float> (var.name());
    }

    NullableVector<unsigned int>* ta_vec {nullptr};
    if (cont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ta_))
    {
        dbContent::Variable& var = cont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ta_);
        assert (buffer->has<unsigned int> (var.name()));
        ta_vec = &buffer->get<unsigned int> (var.name());
    }

    NullableVector<string>* ti_vec {nullptr};
    if (cont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ti_))
    {
        dbContent::Variable& var = cont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ti_);
        assert (buffer->has<string> (var.name()));
        ti_vec = &buffer->get<string> (var.name());
    }

    for (unsigned int cnt=0; cnt < buffer->size(); ++cnt)
    {
        if (m3a_vec && !m3a_vec->isNull(cnt))
            to_be_removed.push_back(cnt);
        else if (mc_vec && !mc_vec->isNull(cnt))
            to_be_removed.push_back(cnt);
        else if (ta_vec && !ta_vec->isNull(cnt))
            to_be_removed.push_back(cnt);
        else if (ti_vec && !ti_vec->isNull(cnt))
            to_be_removed.push_back(cnt);
    }

    return to_be_removed;
}