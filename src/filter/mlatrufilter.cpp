#include "mlatrufilter.h"
#include "compass.h"
#include "mlatrufilterwidget.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "logger.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace dbContent;

MLATRUFilter::MLATRUFilter(const std::string& class_id, const std::string& instance_id,
                     Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    registerParameter("rus_str", &rus_str_, std::string());
    updateRUsFromStr(rus_str_);

    name_ = "MLAT RUs";

    createSubConfigurables();
}

MLATRUFilter::~MLATRUFilter() {}

bool MLATRUFilter::filters(const std::string& dbcontent_name)
{
    return dbcontent_name == "CAT020";
}

std::string MLATRUFilter::getConditionString(const std::string& dbcontent_name, bool& first)
{
    logdbg << "MLATRUFilter: getConditionString: dbo " << dbcontent_name << " active " << active_;

    stringstream ss;

    if (active_ && values_.size())
    {
        assert (dbcontent_name == "CAT020");

        DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
        assert (dbcontent_man.existsDBContent(dbcontent_name));

        assert (dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).existsIn(dbcontent_name));

        if (!first)
            ss << " AND";

        // SELECT x FROM data_cat062, json_each(data_cat062.associations) WHERE json_each.value == 0;

        // any

        ss << " json_each.value IN (" << rus_str_ << ")"; // rest done in SQLGenerator::getSelectCommand

        // each

        // SELECT f.* FROM data_cat020 f, json_each(contributing_receivers) t WHERE t.value IN (2, 6) GROUP BY f.record_number HAVING COUNT(*) = 2;

//        ss << " (";

//        for (auto& ru_val : values_)
//        {
//            if (ru_val != *values_.begin())
//                ss << " OR";

//            ss << " json_each.value == " << to_string(ru_val);
//        }

//        ss << ")";

        first = false;
    }

    logerr << "MLATRUFilter: getConditionString: here '" << ss.str() << "'";

    return ss.str();
}

void MLATRUFilter::generateSubConfigurable(const std::string& class_id,
                                           const std::string& instance_id)
{
    logdbg << "MLATRUFilter: generateSubConfigurable: class_id " << class_id;

    throw std::runtime_error("MLATRUFilter: generateSubConfigurable: unknown class_id " + class_id);
}

void MLATRUFilter::checkSubConfigurables()
{
    logdbg << "MLATRUFilter: checkSubConfigurables";

}

DBFilterWidget* MLATRUFilter::createWidget()
{
    return new MLATRUFilterWidget(*this);
}


void MLATRUFilter::reset()
{
    if (widget_)
        widget_->update();
}

void MLATRUFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (!filters.contains(name_));
    filters[name_] = json::object();
    json& filter = filters.at(name_);

    filter["rus"] = rus_str_;
}

void MLATRUFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (filters.contains(name_));
    const json& filter = filters.at(name_);

    assert (filter.contains("rus"));
    rus_str_ = filter.at("rus");

    updateRUsFromStr(rus_str_);

    if (widget())
        widget()->update();
}

std::string MLATRUFilter::rus() const
{
    return rus_str_;
}

void MLATRUFilter::rus(const std::string& rus_str)
{
    if (!updateRUsFromStr(rus_str)) // false on failure
    {
//        if (widget_)
//            widget_->update();

        return;
    }

    rus_str_ = rus_str;
}

bool MLATRUFilter::updateRUsFromStr(const std::string& values_str)
{
    values_.clear();

    vector<unsigned int> values_tmp;
    vector<string> split_str = String::split(values_str, ',');

    bool ok = true;

    for (auto& tmp_str : split_str)
    {
        unsigned int utn_tmp = QString(tmp_str.c_str()).toInt(&ok);

        if (!ok)
        {
            logerr << "MLATRUFilter: updateRUsFromStr: utn '" << tmp_str << "' not valid";
            break;
        }

        values_tmp.push_back(utn_tmp);
    }

    if (!ok)
        return false;

    values_ = values_tmp;

    return true;
}
