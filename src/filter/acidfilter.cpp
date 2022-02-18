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
    registerParameter("value_str", &value_str_, "");

    name_ = "Aircraft Identification";

    createSubConfigurables();
}

ACIDFilter::~ACIDFilter() {}

bool ACIDFilter::filters(const std::string& dbo_type)
{
    return COMPASS::instance().dbContentManager().metaVariable(DBContent::meta_var_ti_.name()).existsIn(dbo_type);
}

std::string ACIDFilter::getConditionString(const std::string& dbo_name, bool& first,
                                           std::vector<std::string>& extra_from_parts,
                                           std::vector<dbContent::Variable*>& filtered_variables)
{
    logdbg << "ACIDFilter: getConditionString: dbo " << dbo_name << " active " << active_;

    if (!COMPASS::instance().dbContentManager().metaVariable(DBContent::meta_var_ti_.name()).existsIn(dbo_name))
        return "";

    stringstream ss;

    if (active_ && value_str_.size())
    {
        dbContent::Variable& var = COMPASS::instance().dbContentManager().metaVariable(
                    DBContent::meta_var_ti_.name()).getFor(dbo_name);

        filtered_variables.push_back(&var);

        if (!first)
        {
            ss << " AND";
        }

        ss << " " + var.dbColumnName() << " LIKE '" << value_str_ << "'";

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

    filter["Aircraft Identification Values"] = value_str_;
}

void ACIDFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (filters.contains(name_));
    const json& filter = filters.at(name_);

    assert (filter.contains("Aircraft Identification Values"));
    value_str_ = filter.at("Aircraft Identification Values");

    if (widget())
        widget()->update();
}

std::string ACIDFilter::valueString() const
{
    return value_str_;
}

void ACIDFilter::valueString(const std::string& value_str)
{
    value_str_ = value_str;
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

    string fil_str = value_str_;

    fil_str.erase(remove(fil_str.begin(), fil_str.end(), '%'), fil_str.end()); // remove % char

    for (unsigned int cnt=0; cnt < data_vec.size(); ++cnt)
    {
        if (data_vec.isNull(cnt) || data_vec.get(cnt).find(fil_str) == std::string::npos) // null or not found
            to_be_removed.push_back(cnt);
    }

    return to_be_removed;
}

