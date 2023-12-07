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

#include "eval/results/intervalbase.h"
#include "eval/results/evaluationdetail.h"

#include "eval/requirement/base/base.h"
#include "eval/requirement/base/intervalbase.h"

#include "evaluationtargetdata.h"
#include "evaluationmanager.h"

#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"

#include "logger.h"
#include "stringconv.h"
#include "viewpoint.h"
#include "sectorlayer.h"

#include <cassert>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace EvaluationRequirementResult
{

/***********************************************************************************************
 * SingleIntervalBase
 ***********************************************************************************************/

/**
*/
SingleIntervalBase::SingleIntervalBase(const std::string& result_type, 
                                       const std::string& result_id, 
                                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                                       const SectorLayer& sector_layer,
                                       unsigned int utn,
                                       const EvaluationTargetData* target,
                                       EvaluationManager& eval_man,
                                       const EvaluationDetails& details,
                                       int sum_uis,
                                       int missed_uis,
                                      TimePeriodCollection ref_periods)
    :   Single      (result_type, result_id, requirement, sector_layer, utn, target, eval_man, details)
    ,   sum_uis_    (sum_uis)
    ,   missed_uis_ (missed_uis)
    ,   ref_periods_(ref_periods)
{
    updateProbability();
}

/**
*/
void SingleIntervalBase::updateProbability()
{
    probability_.reset();

    if (sum_uis_)
    {
        logdbg << type() << ": updatePD: utn " << utn_ << " missed_uis " << missed_uis_ << " sum_uis " << sum_uis_;

        assert (missed_uis_ <= sum_uis_);

        std::shared_ptr<EvaluationRequirement::IntervalBase> req = std::static_pointer_cast<EvaluationRequirement::IntervalBase>(requirement_);
        assert (req);

        probability_ = 1.0 - ((float)missed_uis_/(float)(sum_uis_));
    }

    result_usable_ = probability_.has_value();

    updateUseFromTarget();
}

/**
*/
QVariant SingleIntervalBase::probabilityVar() const
{
    QVariant pcd_var;
    if (probability_.has_value())
        pcd_var = roundf(probability_.value() * 10000.0) / 100.0;

    return pcd_var;
}

/**
*/
std::vector<std::string> SingleIntervalBase::targetTableColumns() const
{
    return { "UTN", "Begin", "End", "Callsign", "TA", "M3/A", "MC Min", "MC Max", "#EUIs", "#MUIs", probabilityName() };
}

/**
*/
std::vector<QVariant> SingleIntervalBase::targetTableValues() const
{
    return { utn_, 
             target_->timeBeginStr().c_str(), 
             target_->timeEndStr().c_str(),
             target_->acidsStr().c_str(), 
             target_->acadsStr().c_str(),
             target_->modeACodesStr().c_str(),
             target_->modeCMinStr().c_str(),
             target_->modeCMaxStr().c_str(),
             sum_uis_,
             missed_uis_,
             probabilityVar() };
}

/**
*/
std::vector<Base::ReportParam> SingleIntervalBase::detailsOverviewDescriptions() const
{
    std::vector<Base::ReportParam> descr;

    return { { "Use"            , "To be used in results"         , use_             },
             { "#EUIs [1]"      , "Expected Update Intervals"     , sum_uis_         },
             { "#MUIs [1]"      , "Missed Update Intervals"       , missed_uis_      },
             { probabilityName(), probabilityDescription()        , probabilityVar() } };
}

/**
*/
std::vector<std::string> SingleIntervalBase::detailsTableColumns() const
{
    return {"ToD", "DToD", "Ref.", "#MUIs", "Comment"};
}

/**
*/
std::vector<QVariant> SingleIntervalBase::detailsTableValues(const EvaluationDetail& detail) const
{
    auto d_tod = detail.getValue(DetailKey::DiffTOD);
    QVariant d_tod_str = d_tod.isValid() ? QVariant(String::timeStringFromDouble(d_tod.toFloat()).c_str()) : QVariant();

    return { Time::toString(detail.timestamp()).c_str(),
             d_tod_str,
             detail.getValue(DetailKey::RefExists),
             detail.getValue(DetailKey::MissedUIs),
             detail.comments().generalComment().c_str() };
}

/**
*/
unsigned int SingleIntervalBase::sortColumn() const
{
    return 10;
}

/**
*/
void SingleIntervalBase::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << type() << " " <<  requirement_->name() <<": addToReport";

    // add target to requirements->group->req
    addTargetToOverviewTable(root_item);

    // add requirement results to targets->utn->requirements->group->req
    addTargetDetailsToReport(root_item);

    // TODO add requirement description, methods
}

/**
*/
void SingleIntervalBase::addTargetToOverviewTable(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    addTargetDetailsToTable(getRequirementSection(root_item), target_table_name_);

    // add to general sum table
    if (eval_man_.settings().report_split_results_by_mops_ || 
        eval_man_.settings().report_split_results_by_aconly_ms_) 
        addTargetDetailsToTable(root_item->getSection(getRequirementSumSectionID()), target_table_name_);
}

/**
*/
void SingleIntervalBase::addTargetDetailsToTable (EvaluationResultsReport::Section& section, 
                                                  const std::string& table_name)
{
    if (!section.hasTable(table_name))
    {
        std::shared_ptr<EvaluationRequirement::IntervalBase> req = std::static_pointer_cast<EvaluationRequirement::IntervalBase>(requirement_);
        assert (req);

        Qt::SortOrder order;
        order = Qt::AscendingOrder;

        std::vector<std::string> columns = targetTableColumns();

        section.addTable(table_name, columns.size(), columns, true, sortColumn(), order);
    }

    EvaluationResultsReport::SectionContentTable& target_table = section.getTable(table_name);

    std::vector<QVariant> values = targetTableValues();

    target_table.addRow(values, this, {utn_});
}

/**
*/
void SingleIntervalBase::addTargetDetailsToReport(shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    root_item->getSection(getTargetSectionID()).perTargetSection(true); // mark utn section per target
    EvaluationResultsReport::Section& utn_req_section = root_item->getSection(getTargetRequirementSectionID());

    if (!utn_req_section.hasTable("details_overview_table"))
        utn_req_section.addTable("details_overview_table", 3, {"Name", "comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& utn_req_table = utn_req_section.getTable("details_overview_table");

    addCommonDetails(root_item);

    auto descr = detailsOverviewDescriptions();

    for (const auto& d : descr)
        utn_req_table.addRow({ d.name.c_str(), d.descr.c_str(), d.value }, this);

    size_t inside_sectors = 0;

    for (unsigned int cnt=0; cnt < ref_periods_.size(); ++cnt)
    {
        //skip outside sectors
        if (ref_periods_.period(cnt).type() == TimePeriod::Type::OutsideSector)
            continue;

        ++inside_sectors;

        utn_req_table.addRow({("Reference Period "+to_string(cnt)).c_str(), "Time inside sector", ref_periods_.period(cnt).str().c_str()}, this);
    }

    if (inside_sectors == 0)
        utn_req_table.addRow({"Reference Period", "Time inside sector", "None"}, this);

    // condition
    std::shared_ptr<EvaluationRequirement::IntervalBase> req = std::static_pointer_cast<EvaluationRequirement::IntervalBase>(requirement_);
    assert (req);

    utn_req_table.addRow({"Condition", "", req->getConditionStr().c_str()}, this);

    if (req->getConditionStr() == "Failed")
    {
        root_item->getSection(getTargetSectionID()).perTargetWithIssues(true); // mark utn section as with issue
        utn_req_section.perTargetWithIssues(true);
    }

    string result {"Unknown"};

    if (probability_.has_value())
        result = req->getConditionResultStr(probability_.value());

    utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()}, this);

    if (req->mustHoldForAnyTarget().has_value())
        utn_req_table.addRow({"Must hold for any target ", "", req->mustHoldForAnyTarget().value()}, this);

    // add figure
    if (probability_.has_value())
    {
        utn_req_section.addFigure("target_errors_overview", "Target Errors Overview",
                                  [this](void) { return this->getTargetErrorsViewable(); });
    }
    else
    {
        utn_req_section.addText("target_errors_overview_no_figure");
        utn_req_section.getText("target_errors_overview_no_figure").addText(
                    "No target errors found, therefore no figure was generated.");
    }

    // add further details
    reportDetails(utn_req_section);
}

/**
*/
void SingleIntervalBase::reportDetails(EvaluationResultsReport::Section& utn_req_section)
{
    if (!utn_req_section.hasTable(tr_details_table_name_))
    {
        auto columns = detailsTableColumns();

        utn_req_section.addTable(tr_details_table_name_, 5, columns);
    }

    EvaluationResultsReport::SectionContentTable& utn_req_details_table =
            utn_req_section.getTable(tr_details_table_name_);

    utn_req_details_table.setCreateOnDemand(
            [this, &utn_req_details_table](void)
            {
                unsigned int detail_cnt = 0;

                for (auto& rq_det_it : getDetails())
                {
                    auto values = detailsTableValues(rq_det_it);

                    utn_req_details_table.addRow(values, this, detail_cnt++);
                }
            }
        );
}

/**
*/
bool SingleIntervalBase::hasViewableData (const EvaluationResultsReport::SectionContentTable& table, 
                                          const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else if (table.name() == tr_details_table_name_ && annotation.isValid() && annotation.toUInt() < getDetails().size())
        return true;
    
    return false;
}

/**
*/
std::unique_ptr<nlohmann::json::object_t> SingleIntervalBase::viewableData(
        const EvaluationResultsReport::SectionContentTable& table, 
        const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));
    if (table.name() == target_table_name_)
    {
        return getTargetErrorsViewable();
    }
    else if (table.name() == tr_details_table_name_ && annotation.isValid())
    {
        unsigned int detail_cnt = annotation.toUInt();

        loginf << "SinglePositionMaxDistance: viewableData: detail_cnt " << detail_cnt;

        const auto& detail = getDetail(detail_cnt);

        return getTargetErrorsViewable(&detail);
    }
    else
        return nullptr;
}

/**
*/
std::unique_ptr<nlohmann::json::object_t> SingleIntervalBase::getTargetErrorsViewable (const EvaluationDetail* detail)
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr = eval_man_.getViewableForEvaluation(
                utn_, req_grp_id_, result_id_);

    bool has_pos = false;
    double lat_min, lat_max, lon_min, lon_max;

    if (detail)
    {
        assert (detail->numPositions() >= 1);

        double lat  = detail->position(0).latitude_;
        double lon  = detail->position(0).longitude_;
        double zoom = eval_man_.settings().result_detail_zoom_;

        lat_min = lat - zoom;
        lat_max = lat + zoom;
        lon_min = lon - zoom;
        lon_max = lon + zoom;

        has_pos = true;
    }
    else
    {
        for (auto& detail_it : getDetails())
        {
            if (!detail_it.getValue(DetailKey::MissOccurred).toBool())
                continue;

            assert(detail_it.numPositions() >= 1);

            const auto& pos = detail_it.position(0);

            bool has_last_pos = detail_it.numPositions() >= 2;

            if (has_pos)
            {
                lat_min = min(lat_min, pos.latitude_);
                lat_max = max(lat_max, pos.latitude_);

                lon_min = min(lon_min, pos.longitude_);
                lon_max = max(lon_max, pos.longitude_);
            }
            else // tst pos always set
            {
                lat_min = pos.latitude_;
                lat_max = pos.latitude_;

                lon_min = pos.longitude_;
                lon_max = pos.longitude_;

                has_pos = true;
            }

            if (has_last_pos)
            {
                const auto& pos_last = detail_it.position(1);

                lat_min = min(lat_min, pos_last.latitude_);
                lat_max = max(lat_max, pos_last.latitude_);

                lon_min = min(lon_min, pos_last.longitude_);
                lon_max = max(lon_max, pos_last.longitude_);
            }
        }
    }

    if (has_pos)
    {
        (*viewable_ptr)[VP_POS_LAT_KEY] = (lat_max + lat_min) / 2.0;
        (*viewable_ptr)[VP_POS_LON_KEY] = (lon_max + lon_min) / 2.0;

        double lat_w = OSGVIEW_POS_WINDOW_SCALE * (lat_max - lat_min) / 2.0;
        double lon_w = OSGVIEW_POS_WINDOW_SCALE * (lon_max - lon_min) / 2.0;

        if (lat_w < eval_man_.settings().result_detail_zoom_)
            lat_w = eval_man_.settings().result_detail_zoom_;

        if (lon_w < eval_man_.settings().result_detail_zoom_)
            lon_w = eval_man_.settings().result_detail_zoom_;

        (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = lat_w;
        (*viewable_ptr)[VP_POS_WIN_LON_KEY] = lon_w;
    }

    //add track annotations
    addAnnotations(*viewable_ptr, false, true);

    if (detail)
    {
        (*viewable_ptr)[VP_TIMESTAMP_KEY  ] = Time::toString(detail->timestamp());

        //highlight detail
        addAnnotations(*viewable_ptr, *detail);
    }

    return viewable_ptr;
}

/**
*/
bool SingleIntervalBase::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == target_table_name_ && annotation.toUInt() == utn_)
        return true;
    else
        return false;;
}

/**
*/
std::string SingleIntervalBase::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));

    return "Report:Results:"+getTargetRequirementSectionID();
}

/**
*/
bool SingleIntervalBase::hasFailed() const
{
    std::shared_ptr<EvaluationRequirement::IntervalBase> req = std::static_pointer_cast<EvaluationRequirement::IntervalBase>(requirement_);
    assert (req);

    if (probability_.has_value())
        return !req->getConditionResult(probability_.value());
    else
        return false;
}

/**
*/
void SingleIntervalBase::addAnnotations(nlohmann::json::object_t& viewable, bool overview, bool add_ok)
{
    //addAnnotationFeatures(viewable, overview, true);

    json& error_line_coordinates  = annotationLineCoords(viewable, TypeError, overview);
    json& error_point_coordinates = annotationPointCoords(viewable, TypeError, overview);
    json& ok_line_coordinates     = annotationLineCoords(viewable, TypeOk, overview);
    json& ok_point_coordinates    = annotationPointCoords(viewable, TypeOk, overview);

    for (auto& detail_it : getDetails())
    {
        auto check_failed = detail_it.getValueAsOrAssert<bool>(
                    EvaluationRequirementResult::SingleIntervalBase::DetailKey::MissOccurred);

        if (detail_it.numPositions() == 1)
            continue;

        assert (detail_it.numPositions() >= 2);

        if (check_failed)
        {
            error_point_coordinates.push_back(detail_it.position(0).asVector());
            error_point_coordinates.push_back(detail_it.position(1).asVector());

            error_line_coordinates.push_back(detail_it.position(0).asVector());
            error_line_coordinates.push_back(detail_it.position(1).asVector());
        }
        else if (add_ok)
        {
            ok_point_coordinates.push_back(detail_it.position(0).asVector());
            ok_point_coordinates.push_back(detail_it.position(1).asVector());

            if (!overview)
            {
                ok_line_coordinates.push_back(detail_it.position(0).asVector());
                ok_line_coordinates.push_back(detail_it.position(1).asVector());
            }
        }
    }
}

/**
*/
void SingleIntervalBase::addAnnotations(nlohmann::json::object_t& viewable,
                                        const EvaluationDetail& detail)
{
    if (detail.numPositions() == 1)
        return;

    assert (detail.numPositions() >= 2);

    addAnnotationPos(viewable, detail.position(0), TypeHighlight);
    addAnnotationPos(viewable, detail.position(1), TypeHighlight);
    addAnnotationLine(viewable, detail.position(0), detail.position(1), TypeHighlight);
}

/**
*/
int SingleIntervalBase::sumUIs() const
{
    return sum_uis_;
}

/**
*/
int SingleIntervalBase::missedUIs() const
{
    return missed_uis_;
}

/***********************************************************************************************
 * JoinedIntervalBase
 ***********************************************************************************************/

/**
*/
JoinedIntervalBase::JoinedIntervalBase(const std::string& result_type, 
                                       const std::string& result_id, 
                                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                                       const SectorLayer& sector_layer, 
                                       EvaluationManager& eval_man)
:   Joined(result_type, result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
void JoinedIntervalBase::join_impl(std::shared_ptr<Single> other)
{
    std::shared_ptr<SingleIntervalBase> other_sub = std::static_pointer_cast<SingleIntervalBase>(other);
    assert (other_sub);

    addToValues(other_sub);
}

/**
*/
void JoinedIntervalBase::addToValues (std::shared_ptr<SingleIntervalBase> single_result)
{
    assert (single_result);

    if (!single_result->use())
        return;

    sum_uis_    += single_result->sumUIs();
    missed_uis_ += single_result->missedUIs();

    ++num_single_targets_;
    if (single_result->hasFailed())
        ++num_failed_single_targets_;

    updateProbability();
}

/**
*/
void JoinedIntervalBase::updateProbability()
{
    probability_.reset();

    if (sum_uis_)
    {
        logdbg << type() << ": updatePD: result_id " << result_id_ << " missed_uis " << missed_uis_ << " sum_uis " << sum_uis_;

        assert (missed_uis_ <= sum_uis_);

        std::shared_ptr<EvaluationRequirement::IntervalBase> req = std::static_pointer_cast<EvaluationRequirement::IntervalBase>(requirement_);
        assert (req);

        probability_ = 1.0 - ((float)missed_uis_/(float)(sum_uis_));
    }
}

/**
*/
void JoinedIntervalBase::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << type() << " " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << type() << " " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << type() << " " <<  requirement_->name() << ": addToReport: adding joined result";

    addToOverviewTable(root_item);
    addDetails(root_item);
}

/**
*/
void JoinedIntervalBase::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirement::IntervalBase> req = std::static_pointer_cast<EvaluationRequirement::IntervalBase>(requirement_);
    assert (req);

    if (req->mustHoldForAnyTarget().has_value() && req->mustHoldForAnyTarget().value()) // for any target
    {
        string result = num_failed_single_targets_ == 0 ? "Passed" : "Failed";

        // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
        ov_table.addRow({ sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                          requirement_->shortname().c_str(),
                          result_id_.c_str(), {num_single_targets_},
                          num_failed_single_targets_, "= 0", result.c_str()}, this, {} );
    }
    else // pd
    {
        QVariant pd_var;

        string result {"Unknown"};

        if (probability_.has_value())
        {
            pd_var = String::percentToString(probability_.value() * 100.0, req->getNumProbDecimals()).c_str();

            //loginf << "UGA '" << pd_var.toString().toStdString() << "' dec " << req->getNumProbDecimals();

            result = req->getConditionResultStr(probability_.value());
        }

        // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
        ov_table.addRow({ sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                          requirement_->shortname().c_str(),
                          result_id_.c_str(), {sum_uis_},
                          pd_var, req->getConditionStr().c_str(), result.c_str()}, this, {} );
    }
}

/**
*/
std::vector<Base::ReportParam> JoinedIntervalBase::detailsOverviewDescriptions() const
{
    // condition
    std::shared_ptr<EvaluationRequirement::IntervalBase> req = std::static_pointer_cast<EvaluationRequirement::IntervalBase>(requirement_);
    assert (req);

    // pd
    QVariant p_var;
    std::string result {"Unknown"};

    if (probability_.has_value())
    {
        p_var = String::percentToString(probability_.value() * 100.0, req->getNumProbDecimals()).c_str();
        result = req->getConditionResultStr(probability_.value());
    }

    std::vector<Base::ReportParam> params;

    params.emplace_back("#Updates/#EUIs [1]", "Total number update intervals", sum_uis_);
    params.emplace_back("#MUIs [1]", "Number of missed update intervals", missed_uis_);
    params.emplace_back(probabilityName(), probabilityDescription(), p_var);

    params.emplace_back("Condition", "", req->getConditionStr().c_str());
    params.emplace_back("Condition Fulfilled", "", result.c_str());

    if (req->mustHoldForAnyTarget().has_value())
        params.emplace_back("Must hold for any target ", "", req->mustHoldForAnyTarget().value());

    params.emplace_back("#Single Targets", "", num_single_targets_);
    params.emplace_back("#Failed Single Targets", "", num_failed_single_targets_);

    return params;
}

/**
*/
void JoinedIntervalBase::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

    if (!sector_section.hasTable("sector_details_table"))
        sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& sec_det_table =
            sector_section.getTable("sector_details_table");

    addCommonDetails(sec_det_table);

    auto descriptions = detailsOverviewDescriptions();

    for (const auto& d : descriptions)
        sec_det_table.addRow({d.name.c_str(), d.descr.c_str(), d.value}, this);

    // figure
    sector_section.addFigure("sector_overview", "Sector Overview",
                             [this](void) { return this->getErrorsViewable(); });
}

/**
*/
bool JoinedIntervalBase::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    //loginf << "UGA2 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;;
}

/**
*/
std::unique_ptr<nlohmann::json::object_t> JoinedIntervalBase::viewableData(
        const EvaluationResultsReport::SectionContentTable& table, 
        const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));
    return getErrorsViewable();
}

/**
*/
std::unique_ptr<nlohmann::json::object_t> JoinedIntervalBase::getErrorsViewable ()
{
    std::unique_ptr<nlohmann::json::object_t> viewable_ptr =
            eval_man_.getViewableForEvaluation(req_grp_id_, result_id_);

    double lat_min, lat_max, lon_min, lon_max;

    tie(lat_min, lat_max) = sector_layer_.getMinMaxLatitude();
    tie(lon_min, lon_max) = sector_layer_.getMinMaxLongitude();

    (*viewable_ptr)[VP_POS_LAT_KEY] = (lat_max+lat_min)/2.0;
    (*viewable_ptr)[VP_POS_LON_KEY] = (lon_max+lon_min)/2.0;;

    double lat_w = OSGVIEW_POS_WINDOW_SCALE*(lat_max-lat_min)/2.0;
    double lon_w = OSGVIEW_POS_WINDOW_SCALE*(lon_max-lon_min)/2.0;

    if (lat_w < eval_man_.settings().result_detail_zoom_)
        lat_w = eval_man_.settings().result_detail_zoom_;

    if (lon_w < eval_man_.settings().result_detail_zoom_)
        lon_w = eval_man_.settings().result_detail_zoom_;

    (*viewable_ptr)[VP_POS_WIN_LAT_KEY] = lat_w;
    (*viewable_ptr)[VP_POS_WIN_LON_KEY] = lon_w;

    addAnnotationsFromSingles(*viewable_ptr);

    return viewable_ptr;
}

/**
*/
bool JoinedIntervalBase::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    //loginf << "UGA3 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

    if (table.name() == req_overview_table_name_)
        return true;
    else
        return false;
}

/**
*/
std::string JoinedIntervalBase::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));
    return "Report:Results:"+getRequirementSectionID();
}

/**
*/
void JoinedIntervalBase::updatesToUseChanges_impl()
{
    loginf << type() << ": updatesToUseChanges: prev sum_uis " << sum_uis_
            << " missed_uis " << missed_uis_;

    if (probability_.has_value())
        loginf << type() << ": updatesToUseChanges: prev result " << result_id_
                << " pcd " << 100.0 * probability_.value();
    else
        loginf << type() << ": updatesToUseChanges: prev result " << result_id_ << " has no data";

    sum_uis_    = 0;
    missed_uis_ = 0;

    for (auto result_it : results_)
    {
        std::shared_ptr<SingleIntervalBase> result = std::static_pointer_cast<SingleIntervalBase>(result_it);
        assert (result);

        addToValues(result);
    }

    loginf << type() << ": updatesToUseChanges: updt sum_uis " << sum_uis_
            << " missed_uis " << missed_uis_;

    if (probability_.has_value())
        loginf << type() << ": updatesToUseChanges: updt result " << result_id_
                << " pcd " << 100.0 * probability_.value();
    else
        loginf << type() << ": updatesToUseChanges: updt result " << result_id_ << " has no data";
}

}
