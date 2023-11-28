#include "mode3afilter.h"
#include "compass.h"
#include "mode3afilterwidget.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "logger.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace dbContent;

Mode3AFilter::Mode3AFilter(const std::string& class_id, const std::string& instance_id,
                       Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    registerParameter("values_str", &values_str_, std::string());
    updateValuesFromStr(values_str_);

    name_ = "Mode 3/A Codes";

    createSubConfigurables();
}

Mode3AFilter::~Mode3AFilter() {}

bool Mode3AFilter::filters(const std::string& dbo_type)
{
    return COMPASS::instance().dbContentManager().metaVariable(DBContent::meta_var_m3a_.name()).existsIn(dbo_type);
}

std::string Mode3AFilter::getConditionString(const std::string& dbcontent_name, bool& first)
{
    logdbg << "Mode3AFilter: getConditionString: dbo " << dbcontent_name << " active " << active_;

    if (!COMPASS::instance().dbContentManager().metaVariable(DBContent::meta_var_m3a_.name()).existsIn(dbcontent_name))
        return "";

    stringstream ss;

    if (active_ && (values_.size() || null_wanted_))
    {
        dbContent::Variable& var = COMPASS::instance().dbContentManager().metaVariable(
                    DBContent::meta_var_m3a_.name()).getFor(dbcontent_name);

        if (!first)
        {
            ss << " AND";
        }

        ss << " ";

        if (null_wanted_)
            ss << "(";

        if (values_.size())
        {
            ss << var.dbColumnName() << " IN (" << String::compress(values_, ',') << ")";
        }

        if (null_wanted_)
        {
            if (values_.size())
                ss << " OR";

            ss << " " << var.dbColumnName() << " IS NULL)";
        }

        first = false;
    }

    logdbg << "Mode3AFilter: getConditionString: here '" << ss.str() << "'";

    return ss.str();
}

void Mode3AFilter::generateSubConfigurable(const std::string& class_id, const std::string& instance_id)
{
    logdbg << "Mode3AFilter: generateSubConfigurable: class_id " << class_id;

    throw std::runtime_error("Mode3AFilter: generateSubConfigurable: unknown class_id " + class_id);
}

void Mode3AFilter::checkSubConfigurables()
{
    logdbg << "Mode3AFilter: checkSubConfigurables";
}

DBFilterWidget* Mode3AFilter::createWidget()
{
    return new Mode3AFilterWidget(*this);
}


void Mode3AFilter::reset()
{
    if (widget())
        widget_->update();
}

void Mode3AFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (!filters.contains(name_));
    filters[name_] = json::object();
    json& filter = filters.at(name_);

    filter["Mode 3/A Codes Values"] = values_str_;
}

void Mode3AFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (filters.contains(name_));
    const json& filter = filters.at(name_);

    assert (filter.contains("Mode 3/A Codes Values"));
    values_str_ = filter.at("Mode 3/A Codes Values");

    updateValuesFromStr(values_str_);

    if (widget())
        widget()->update();
}

std::string Mode3AFilter::valuesString() const
{
    return values_str_;
}

void Mode3AFilter::valuesString(const std::string& values_str)
{
    if (!updateValuesFromStr(values_str)) // false on failure
    {
//        if (widget_)
//            widget_->update();
        return;
    }

    values_str_ = values_str;
}

bool Mode3AFilter::activeInLiveMode()
{
    return true;
}

std::vector<size_t> Mode3AFilter::filterBuffer(const std::string& dbcontent_name, std::shared_ptr<Buffer> buffer)
{
    std::vector<size_t> to_be_removed;

    if (!COMPASS::instance().dbContentManager().metaVariable(DBContent::meta_var_m3a_.name()).existsIn(dbcontent_name))
        return to_be_removed;

    dbContent::Variable& var = COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_m3a_.name()).getFor(dbcontent_name);

    assert (buffer->has<unsigned int> (var.name()));

    NullableVector<unsigned int>& data_vec = buffer->get<unsigned int> (var.name());

    for (unsigned int cnt=0; cnt < buffer->size(); ++cnt)
    {
        if (data_vec.isNull(cnt))
        {
            if (!null_wanted_)
                to_be_removed.push_back(cnt);

            continue;
        }
        else if (!values_.count(data_vec.get(cnt)))
            to_be_removed.push_back(cnt);
    }

    return to_be_removed;
}

bool Mode3AFilter::updateValuesFromStr(const std::string& values_str)
{
    set<unsigned int> values_tmp;
    vector<string> split_str = String::split(values_str, ',');

    bool ok = true;

    null_wanted_ = false;

    for (auto& tmp_str : split_str)
    {
        if (String::trim(tmp_str) == "NULL" || String::trim(tmp_str) == "null")
        {
            null_wanted_ = true;
            continue;
        }

        unsigned int utn_tmp = QString(tmp_str.c_str()).toInt(&ok, 8);

        if (!ok)
        {
            logerr << "Mode3AFilter: updateUTNSFromStr: utn '" << tmp_str << "' not valid";
            break;
        }

        values_tmp.insert(utn_tmp);
    }

    if (!ok)
        return false;

    values_ = values_tmp;

    return true;
}
