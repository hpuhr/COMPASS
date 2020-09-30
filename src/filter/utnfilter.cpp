#include "utnfilter.h"
#include "atsdb.h"
#include "utnfilterwidget.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "logger.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;

UTNFilter::UTNFilter(const std::string& class_id, const std::string& instance_id,
                     Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    registerParameter("utns_str", &utns_str_, "");
    utns(utns_str_);

    name_ = "UTNs";

    if (!ATSDB::instance().objectManager().hasAssociations())
    {
        loginf << "UTNFilter: contructor: disabled since no associations";

        active_ = false;
        visible_ = false;
    }

    createSubConfigurables();

    assert(widget_);
}

UTNFilter::~UTNFilter() {}

bool UTNFilter::filters(const std::string& dbo_type)
{
    if (!ATSDB::instance().objectManager().hasAssociations())
        return false;

    return true;
}

std::string UTNFilter::getConditionString(const std::string& dbo_name, bool& first,
                                          std::vector<DBOVariable*>& filtered_variables)
{
    loginf << "UTNFilter: getConditionString: dbo " << dbo_name << " active " << active_;

    if (!ATSDB::instance().objectManager().hasAssociations())
        return "";

    stringstream ss;

    if (active_)
    {
        DBObjectManager& obj_man = ATSDB::instance().objectManager();
        assert (obj_man.existsObject(dbo_name));

        DBObject& object = obj_man.object(dbo_name);

        if (!object.hasAssociations())
        {
            loginf << "UTNFilter: getConditionString: no associations";
            return "";
        }

        vector<unsigned int> rec_nums;
        const DBOAssociationCollection& assocations = object.associations();

        for (auto utn : utns_)
        {
            vector<unsigned int> rec_nums_utn = assocations.getRecNumsForUTN(utn);

            loginf << "UTNFilter: getConditionString: utn " << utn << " num rec_nums " << rec_nums_utn.size();

            rec_nums.insert(rec_nums.end(), rec_nums_utn.begin(), rec_nums_utn.end());
        }

        loginf << "UTNFilter: getConditionString got " << rec_nums.size() << " rec_nums for dbo " << dbo_name;;

        if (!rec_nums.size())
        {
            loginf << "UTNFilter: getConditionString: no rec_nums";
            return "";
        }

        filtered_variables.push_back(&object.variable("rec_num"));

        if (!first)
        {
            ss << " AND";
        }

        ss << " rec_num IN (";

        for (unsigned int cnt = 0; cnt < rec_nums.size(); ++cnt)
        {
            if (cnt != 0)
                ss << ",";

            ss << to_string(rec_nums.at(cnt));
        }

        ss << ")";

        first = false;
    }

    loginf << "UTNFilter: getConditionString: here '" << ss.str() << "'";

    return ss.str();
}

void UTNFilter::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    logdbg << "UTNFilter: generateSubConfigurable: class_id " << class_id;

    if (class_id.compare("UTNFilterWidget") == 0)
    {
        assert(!widget_);
        widget_ = new UTNFilterWidget(*this, class_id, instance_id);

        if (!ATSDB::instance().objectManager().hasAssociations())
        {
            widget_->setDisabled(true);
            widget_->setInvisible();
        }
    }
    else
        throw std::runtime_error("UTNFilter: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

void UTNFilter::checkSubConfigurables()
{
    logdbg << "UTNFilter: checkSubConfigurables";

    if (!widget_)
    {
        logdbg << "UTNFilter: checkSubConfigurables: generating my filter widget";
        widget_ = new UTNFilterWidget(*this, "UTNFilterWidget", instanceId() + "Widget0");

        if (!ATSDB::instance().objectManager().hasAssociations())
        {
            widget_->setDisabled(true);
            widget_->setInvisible();
        }

    }
    assert(widget_);

//    for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
//    {
//        DBFilterWidget* filter_widget = sub_filters_.at(cnt)->widget();
//        QObject::connect((QWidget*)filter_widget, SIGNAL(possibleFilterChange()), (QWidget*)widget_,
//                         SLOT(possibleSubFilterChange()));
//        widget_->addChildWidget(filter_widget);
//    }
}


void UTNFilter::reset()
{
//    for (auto& it : data_sources_)
//        it.second.setActive(true);

    widget_->update();
}

void UTNFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

//    assert (!filters.contains(name_));
//    filters[name_] = json::object();
//    json& filter = filters.at(name_);

//    filter["active_sources"] = json::array();
//    json& values = filter.at("active_sources");

//    unsigned int cnt=0;

//    for (auto& ds_it : data_sources_)
//    {
//        if (ds_it.second.isActive())
//        {
//            values[cnt] = ds_it.second.getNumber();
//            ++cnt;
//        }
//    }
}

void UTNFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

//    assert (filters.contains(name_));
//    const json& filter = filters.at(name_);

//    assert (filter.contains("active_sources"));
//    const json& active_sources = filter.at("active_sources");

//    assert (active_sources.is_array());

//    // disable all sources
//    for (auto& ds_it : data_sources_)
//        ds_it.second.setActive(false);

//    // set active sources
//    for (auto& ds_it : active_sources.get<json::array_t>())
//    {
//        int number = ds_it;

//        if (data_sources_.count(number))
//            data_sources_.at(number).setActive(true);
//        else
//            logwrn << "UTNFilter: loadViewPointConditions: source " << number << " not found";
//    }

    if (widget())
        widget()->update();
}

std::string UTNFilter::utns() const
{
    return utns_str_;
}

void UTNFilter::utns(const std::string& utns)
{
    vector<unsigned int> utns_tmp;
    vector<string> utns_tmp_str = String::split(utns, ',');

    bool ok;

    for (auto& utn_str : utns_tmp_str)
    {
        unsigned int utn_tmp = QString(utn_str.c_str()).toInt(&ok);

        if (!ok)
        {
            logerr << "UTNFilter: utns: utn '" << utn_str << "' not valid";
            break;
        }

        utns_tmp.push_back(utn_tmp);
    }

    if (!ok)
    {
        if (widget_)
            widget_->update();
        return;
    }


    utns_str_ = utns;
    utns_ = utns_tmp;
}
