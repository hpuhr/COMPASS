#include "dbospecificvaluesdbfilter.h"
#include "dbospecificvaluesdbfilterwidget.h"
#include "atsdb.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "logger.h"
#include "stringconv.h"
#include "dbfiltercondition.h"

using namespace Utils;
using namespace nlohmann;
using namespace std;

DBOSpecificValuesDBFilter::DBOSpecificValuesDBFilter(const std::string& class_id, const std::string& instance_id,
                                                     Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    logdbg << "DBOSpecificValuesDBFilter: constructor";

    registerParameter("dbo_name", &dbo_name_, "");
    registerParameter("variable_name", &variable_name_, "");
    registerParameter("condition_operator", &condition_operator_, "");

    // dbobject
    if (!ATSDB::instance().objectManager().existsObject(dbo_name_))
        throw std::invalid_argument("DataSourcesFilter: DataSourcesFilter: instance " +
                                    instance_id + " has non-existing object " + dbo_name_);

    object_ = &ATSDB::instance().objectManager().object(dbo_name_);
    assert (object_);

    if (!object_->hasCurrentDataSourceDefinition())
    {
        logerr << "DataSourcesFilter: DataSourcesFilter: instance " + instance_id + " object "
               << dbo_name_ + " has no data sources";
        disabled_ = true;
        return;
    }

    if (!object_->hasDataSources())
    {
        disabled_ = true;
        return;
    }

    if (!object_->existsInDB())
    {
        disabled_ = true;
        return;
    }

    ds_column_name_ = object_->currentDataSourceDefinition().localKey();

    // variable
    assert (variable_name_.size());
    if (!object_->hasVariable(variable_name_))
        throw std::invalid_argument("DataSourcesFilter: DataSourcesFilter: instance " +
                                    instance_id + " has non-existing variable " + variable_name_);

    variable_ = &object_->variable(variable_name_);
    assert (variable_);

    assert (condition_operator_.size());

    createSubConfigurables();

    assert(widget_);

    if (object_->count() == 0)
    {
        active_ = false;
        widget_->setInvisible();
        widget_->update();
        widget_->setDisabled(true);
    }
}

DBOSpecificValuesDBFilter::~DBOSpecificValuesDBFilter() {}

bool DBOSpecificValuesDBFilter::filters(const std::string& dbo_type) { return dbo_name_ == dbo_type; }

std::string DBOSpecificValuesDBFilter::getConditionString(const std::string& dbo_name, bool& first,
                                                          std::vector<DBOVariable*>& filtered_variables)
{
    assert(!disabled_);
    assert (object_->hasDataSources());

    std::stringstream ss;

    bool condition_set = false;

    if (active_)
    {
        if (dbo_name == dbo_name_)
        {
            if (!first)
            {
                if (op_and_)
                    ss << " AND ";
                else
                    ss << " OR ";
            }

            ss << "("; // first condition

            for (unsigned int cnt = 0; cnt < conditions_.size(); cnt++)
            {
                if (conditions_.at(cnt)->valueInvalid())
                {
                    logwrn << "DBOSpecificValuesDBFilter " << instanceId()
                           << ": getConditionString: invalid condition, will be skipped";
                    continue;
                }

                if (condition_set)
                    ss << " OR ";

                string cond_name = conditions_.at(cnt)->instanceId();

                DBObject:: DataSourceIterator it = find_if(object_->dsBegin(), object_->dsEnd(),
                                                           [cond_name, this] (const pair<int, DBODataSource>& s) {
                    return s.second.hasShortName() ?
                                (s.second.shortName()+" "+variable_name_) == cond_name
                              : (s.second.name()+" "+variable_name_) == cond_name; } );
                assert (it != object_->dsEnd());
                int ds_id = it->first;

                bool cond_first = true;
                std::string text =
                        conditions_.at(cnt)->getConditionString(dbo_name, cond_first, filtered_variables);

                ss << "(" << ds_column_name_ << "=" << ds_id << " AND " << text << ")";


                if (!condition_set) // first time only
                    filtered_variables.push_back(&object_->variable(ds_column_name_));

                condition_set = true;

            }

            ss << ")"; // there be conditions


            //        for (unsigned int cnt = 0; cnt < sub_filters_.size(); cnt++)
            //        {
            //            std::string text =
            //                sub_filters_.at(cnt)->getConditionString(dbo_name, first, filtered_variables);
            //            ss << text;
            //        }
        }
    }

    logdbg << "DBOSpecificValuesDBFilter " << instanceId() << ": getConditionString: object " << dbo_name
           << " here '" << ss.str() << "' first " << first << " condition_set " << condition_set;

    if (condition_set)
        return ss.str();
    else
        return "";
}

void DBOSpecificValuesDBFilter::checkSubConfigurables()
{
    logdbg << "DBOSpecificValuesDBFilter: checkSubConfigurables";

    // widget

    if (!widget_)
    {
        logdbg << "DBOSpecificValuesDBFilter: checkSubConfigurables: generating generic filter widget";
        widget_ = new DBFilterWidget("DBFilterWidget", instanceId() + "Widget0", *this);

        if (disabled_)
        {
            widget_->setInvisible();
            widget_->setDisabled(true);
        }
    }
    assert(widget_);


    // data sources
    assert (object_->hasDataSources());

    // find and delete outdated data source conditions
    vector<DBFilterCondition*> conditions_to_delete;

    for (auto cond_it : conditions_)
    {
        string cond_name = cond_it->instanceId();

        if (find_if(object_->dsBegin(), object_->dsEnd(),
                    [cond_name, this] (const pair<int, DBODataSource>& s) {
                    return s.second.hasShortName() ?
                    (s.second.shortName()+" "+variable_name_) == cond_name
                    : (s.second.name()+" "+variable_name_) == cond_name; } )
                == object_->dsEnd())
            conditions_to_delete.push_back(cond_it);
    }

    for (auto cond_it : conditions_to_delete)
    {
        logdbg << "DBOSpecificValuesDBFilter: checkSubConfigurables: deleting condition " << cond_it->instanceId();
        deleteCondition(cond_it);
    }

    conditions_to_delete.clear();

    // create new ones for data sources
    for (auto ds_it = object_->dsBegin(); ds_it != object_->dsEnd(); ++ds_it)
    {
        string ds_name = ds_it->second.name();
        if (ds_it->second.hasShortName())
            ds_name = ds_it->second.shortName();

        ds_name += " "+variable_name_;

        auto it = find_if(conditions_.begin(), conditions_.end(),
                          [ds_name] (const DBFilterCondition* s) { return s->instanceId() == ds_name; } );

        if (it == conditions_.end()) // add
        {
            logdbg << "DBOSpecificValuesDBFilter: checkSubConfigurables: creating new condition " << ds_name;
            Configuration& config = addNewSubConfiguration("DBFilterCondition", ds_name);
            config.addParameterString("reset_value", "4227");
            config.addParameterString("value", "4227");
            config.addParameterString("variable_dbo_name", dbo_name_);
            config.addParameterString("variable_name", variable_name_);
            config.addParameterBool("op_and", false);
            config.addParameterString("operator", condition_operator_);
            config.addParameterBool("display_instance_id", true);
            generateSubConfigurable("DBFilterCondition", config.getInstanceId());
        }
    }
}


