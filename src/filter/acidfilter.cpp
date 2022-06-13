#include "acidfilter.h"
#include "compass.h"
#include "acidfilterwidget.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "logger.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace dbContent;

ACIDFilter::ACIDFilter(const std::string& class_id, const std::string& instance_id,
                       Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    registerParameter("values_str", &values_str_, "");
    updateValuesFromStr(values_str_);

    name_ = "Aircraft Identification";

    createSubConfigurables();
}

ACIDFilter::~ACIDFilter() {}

bool ACIDFilter::filters(const std::string& dbo_type)
{
    return COMPASS::instance().dbContentManager().metaVariable(DBContent::meta_var_ti_.name()).existsIn(dbo_type);
}

std::string ACIDFilter::getConditionString(const std::string& dbcontent_name, bool& first,
                                           std::vector<std::string>& extra_from_parts,
                                           std::vector<dbContent::Variable*>& filtered_variables)
{
    logdbg << "ACIDFilter: getConditionString: dbo " << dbcontent_name << " active " << active_;

    if (!COMPASS::instance().dbContentManager().metaVariable(DBContent::meta_var_ti_.name()).existsIn(dbcontent_name))
        return "";

    stringstream ss;

    if (active_  && (values_.size() || null_wanted_))
    {
        dbContent::Variable& var = COMPASS::instance().dbContentManager().metaVariable(
                    DBContent::meta_var_ti_.name()).getFor(dbcontent_name);

        filtered_variables.push_back(&var);

        if (!first)
            ss << " AND";

        ss << " (";

        bool first_val = true;

        for (auto val_it : values_)
        {
            if (!first_val)
                ss << " OR";

             ss << " " << var.dbColumnName()  << " LIKE '%" << val_it << "%'";

            first_val = false;
        }

        if (null_wanted_)
        {
            if (!first_val)
                ss << " OR";

            ss << " " << var.dbColumnName()  << " IS NULL";
        }

        ss << ")";

        first = false;
    }

    logdbg << "ACIDFilter: getConditionString: here '" << ss.str() << "'";

    return ss.str();
}

void ACIDFilter::generateSubConfigurable(const std::string& class_id, const std::string& instance_id)
{
    logdbg << "ACIDFilter: generateSubConfigurable: class_id " << class_id;

    throw std::runtime_error("ACIDFilter: generateSubConfigurable: unknown class_id " + class_id);
}

void ACIDFilter::checkSubConfigurables()
{
    logdbg << "ACIDFilter: checkSubConfigurables";
}

DBFilterWidget* ACIDFilter::createWidget()
{
    return new ACIDFilterWidget(*this);
}

void ACIDFilter::reset()
{
    if (widget())
        widget_->update();
}

void ACIDFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (!filters.contains(name_));
    filters[name_] = json::object();
    json& filter = filters.at(name_);

    filter["Aircraft Identification Values"] = values_str_;
}

void ACIDFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (filters.contains(name_));
    const json& filter = filters.at(name_);

    assert (filter.contains("Aircraft Identification Values"));
    values_str_ = filter.at("Aircraft Identification Values");

    if (widget())
        widget()->update();
}

std::string ACIDFilter::valuesString() const
{
    return values_str_;
}

void ACIDFilter::valuesString(const std::string& values_str)
{
    if (!updateValuesFromStr(values_str)) // false on failure
        return;

    values_str_ = values_str;
}

bool ACIDFilter::activeInLiveMode()
{
    return true;
}

std::vector<size_t> ACIDFilter::filterBuffer(const std::string& dbcontent_name, std::shared_ptr<Buffer> buffer)
{
    std::vector<size_t> to_be_removed;

    if (!COMPASS::instance().dbContentManager().metaVariable(DBContent::meta_var_ti_.name()).existsIn(dbcontent_name))
        return to_be_removed;

    dbContent::Variable& var = COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_ti_.name()).getFor(dbcontent_name);

    assert (buffer->has<string> (var.name()));

    NullableVector<string>& data_vec = buffer->get<string> (var.name());

    bool found;

    for (unsigned int cnt=0; cnt < buffer->size(); ++cnt)
    {
        if (data_vec.isNull(cnt)) // null or not found
        {
            if (!null_wanted_)
                to_be_removed.push_back(cnt);

            continue;
        }
        else
        {
            found = false;

            for (auto& val_it : values_)
            {
                if (data_vec.get(cnt).find(val_it) != std::string::npos)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
                to_be_removed.push_back(cnt);
        }
    }

    loginf << "ACIDFilter: filterBuffer: content " << dbcontent_name << " erase '" << values_str_ << "' num "
           << to_be_removed.size() << " total " << buffer->size();

    return to_be_removed;
}

bool ACIDFilter::updateValuesFromStr(const std::string& values_str)
{
    set<string> values_tmp;
    vector<string> split_str = String::split(values_str, ',');

    null_wanted_ = false;

    for (auto& tmp_str : split_str)
    {
        if (String::trim(tmp_str) == "NULL" || String::trim(tmp_str) == "null")
        {
            null_wanted_ = true;
            continue;
        }

        values_tmp.insert(boost::algorithm::to_upper_copy(tmp_str));
    }

    values_ = values_tmp;

    return true;
}
