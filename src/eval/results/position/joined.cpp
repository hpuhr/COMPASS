#include "eval/results/position/single.h"
#include "eval/results/position/joined.h"
#include "eval/requirement/base.h"
#include "eval/requirement/detection/detection.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"

#include <algorithm>
#include <cassert>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

    JoinedPositionMaxDistance::JoinedPositionMaxDistance(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer, EvaluationManager& eval_man)
        : Joined("JoinedPositionMaxDistance", result_id, requirement, sector_layer, eval_man)
    {
    }


    void JoinedPositionMaxDistance::join(std::shared_ptr<Base> other)
    {
        Joined::join(other);

        std::shared_ptr<SinglePositionMaxDistance> other_sub =
                std::static_pointer_cast<SinglePositionMaxDistance>(other);
        assert (other_sub);

        addToValues(other_sub);
    }

    void JoinedPositionMaxDistance::addToValues (std::shared_ptr<SinglePositionMaxDistance> single_result)
    {
        assert (single_result);

        if (!single_result->use())
            return;

        unsigned int num_distance = num_pos_ok_+num_pos_nok_;
        unsigned int other_num_distance = single_result->numPosOk()+single_result->numPosNOk();

        num_pos_ += single_result->numPos();
        num_no_ref_ += single_result->numNoRef();
        num_pos_outside_ += single_result->numPosOutside();
        num_pos_inside_ += single_result->numPosInside();
        num_pos_ok_ += single_result->numPosOk();
        num_pos_nok_ += single_result->numPosNOk();

        if (first_)
        {
            error_min_ = single_result->errorMin();
            error_max_ = single_result->errorMax();
            first_ = false;
        }
        else
        {
            error_min_ = min(error_min_, single_result->errorMin());
            error_max_ = max(error_max_, single_result->errorMax());
        }

        if (num_distance+other_num_distance)
            error_avg_ = (float)(error_avg_*num_distance + single_result->errorAvg()*other_num_distance)
                    /(float)(num_distance+other_num_distance);

        updatePMinPos();
    }

    void JoinedPositionMaxDistance::updatePMinPos()
    {
        assert (num_no_ref_ <= num_pos_);
        assert (num_pos_ - num_no_ref_ == num_pos_inside_ + num_pos_outside_);
        assert (num_pos_inside_ == num_pos_ok_ + num_pos_nok_);

        if (num_pos_ - num_no_ref_ - num_pos_outside_)
        {
            assert (num_pos_ == num_no_ref_ + num_pos_outside_+ num_pos_ok_ + num_pos_nok_);
            p_min_pos_ = (float)num_pos_ok_/(float)(num_pos_ - num_no_ref_ - num_pos_outside_);
            has_p_min_pos_ = true;
        }
        else
        {
            p_min_pos_ = 0;
            has_p_min_pos_ = false;
        }
    }

    void JoinedPositionMaxDistance::print()
    {
        std::shared_ptr<EvaluationRequirement::PositionMaxDistance> req =
                std::static_pointer_cast<EvaluationRequirement::PositionMaxDistance>(requirement_);
        assert (req);

        if (num_pos_)
            loginf << "JoinedPositionMaxDistance: print: req. name " << req->name()
                   << " pd " << String::percentToString(100.0 * p_min_pos_)
                   << " passed " << (p_min_pos_ >= req->minimumProbability());
        else
            loginf << "JoinedPositionMaxDistance: print: req. name " << req->name() << " has no data";
    }

    void JoinedPositionMaxDistance::addToReport (
            std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        logdbg << "JoinedPositionMaxDistance " <<  requirement_->name() <<": addToReport";

        if (!results_.size()) // some data must exist
        {
            logerr << "JoinedPositionMaxDistance " <<  requirement_->name() <<": addToReport: no data";
            return;
        }

        logdbg << "JoinedPositionMaxDistance " <<  requirement_->name() << ": addToReport: adding joined result";

        addToOverviewTable(root_item);
        addDetails(root_item);
    }

    void JoinedPositionMaxDistance::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

        // condition
        std::shared_ptr<EvaluationRequirement::PositionMaxDistance> req =
                std::static_pointer_cast<EvaluationRequirement::PositionMaxDistance>(requirement_);
        assert (req);

        string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

        // pd
        QVariant pd_var;

        string result {"Unknown"};

        if (has_p_min_pos_)
        {
            pd_var = String::percentToString(p_min_pos_ * 100.0).c_str();

            result = p_min_pos_ >= req->minimumProbability() ? "Passed" : "Failed";
        }

        // "Req.", "Group", "Result", "Condition", "Result"
        ov_table.addRow({sector_layer_.name().c_str(), requirement_->shortname().c_str(),
                         requirement_->groupName().c_str(), {num_pos_ok_+num_pos_nok_},
                         pd_var, condition.c_str(), result.c_str()}, this, {});
    }

    void JoinedPositionMaxDistance::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

        if (!sector_section.hasTable("sector_details_table"))
            sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

        EvaluationResultsReport::SectionContentTable& sec_det_table =
                sector_section.getTable("sector_details_table");

        addCommonDetails(sec_det_table);

        // condition
        std::shared_ptr<EvaluationRequirement::PositionMaxDistance> req =
                std::static_pointer_cast<EvaluationRequirement::PositionMaxDistance>(requirement_);
        assert (req);

        string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

        // pd
        QVariant pd_var;

        string result {"Unknown"};

        if (has_p_min_pos_)
        {
            pd_var = String::percentToString(p_min_pos_ * 100.0).c_str();

            result = p_min_pos_ >= req->minimumProbability() ? "Passed" : "Failed";
        }

        sec_det_table.addRow({"#Pos [1]", "Number of updates", num_pos_}, this);
        sec_det_table.addRow({"#NoRef [1]", "Number of updates w/o reference positions", num_no_ref_}, this);
        sec_det_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
        sec_det_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
        sec_det_table.addRow({"#POK [1]", "Number of updates with acceptable distance", num_pos_ok_}, this);
        sec_det_table.addRow({"#PNOK [1]", "Number of updates with unacceptable distance ", num_pos_nok_}, this);
        sec_det_table.addRow({"POK [%]", "Probability of acceptable position", pd_var}, this);

        sec_det_table.addRow({"Condition", {}, condition.c_str()}, this);
        sec_det_table.addRow({"Condition Fulfilled", {}, result.c_str()}, this);

        sec_det_table.addRow({"EMin [m]", "Distance Error minimum", error_min_}, this);
        sec_det_table.addRow({"EMax [m]", "Distance Error maxmimum", error_max_}, this);
        sec_det_table.addRow({"EAvg [m]", "Distance Error average", error_avg_}, this);

        // figure
        if (has_p_min_pos_ && p_min_pos_ != 1.0)
        {
            sector_section.addFigure("sector_errors_overview", "Sector Errors Overview",
                                     getErrorsViewable());
        }
        else
        {
            sector_section.addText("sector_errors_overview_no_figure");
            sector_section.getText("sector_errors_overview_no_figure").addText(
                        "No target errors found, therefore no figure was generated.");
        }
    }

    bool JoinedPositionMaxDistance::hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == "req_overview")
            return true;
        else
            return false;
    }

    std::unique_ptr<nlohmann::json::object_t> JoinedPositionMaxDistance::viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasViewableData(table, annotation));

        return getErrorsViewable();
    }

    std::unique_ptr<nlohmann::json::object_t> JoinedPositionMaxDistance::getErrorsViewable ()
    {
        std::unique_ptr<nlohmann::json::object_t> viewable_ptr =
                eval_man_.getViewableForEvaluation(req_grp_id_, result_id_);

        double lat_min, lat_max, lon_min, lon_max;

        tie(lat_min, lat_max) = sector_layer_.getMinMaxLatitude();
        tie(lon_min, lon_max) = sector_layer_.getMinMaxLongitude();

        (*viewable_ptr)["position_latitude"] = (lat_max+lat_min)/2.0;
        (*viewable_ptr)["position_longitude"] = (lon_max+lon_min)/2.0;;

        double lat_w = 1.1*(lat_max-lat_min)/2.0;
        double lon_w = 1.1*(lon_max-lon_min)/2.0;

        if (lat_w < 0.02)
            lat_w = 0.02;

        if (lon_w < 0.02)
            lon_w = 0.02;

        (*viewable_ptr)["position_window_latitude"] = lat_w;
        (*viewable_ptr)["position_window_longitude"] = lon_w;

        return viewable_ptr;
    }

    bool JoinedPositionMaxDistance::hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        if (table.name() == "req_overview")
            return true;
        else
            return false;;
    }

    std::string JoinedPositionMaxDistance::reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasReference(table, annotation));
        return "Report:Results:"+getRequirementSectionID();
    }

    void JoinedPositionMaxDistance::updatesToUseChanges()
    {
        loginf << "JoinedPositionMaxDistance: updatesToUseChanges: prev num_pos " << num_pos_
               << " num_no_ref " << num_no_ref_ << " num_pos_ok " << num_pos_ok_ << " num_pos_nok " << num_pos_nok_;

        if (num_pos_)
            loginf << "JoinedPositionMaxDistance: updatesToUseChanges: prev result " << result_id_
                   << " pd " << 100.0 * p_min_pos_;
        else
            loginf << "JoinedPositionMaxDistance: updatesToUseChanges: prev result " << result_id_ << " has no data";

        num_pos_ = 0;
        num_no_ref_ = 0;
        num_pos_outside_ = 0;
        num_pos_inside_ = 0;
        num_pos_ok_ = 0;
        num_pos_nok_ = 0;

        first_ = true;

        for (auto result_it : results_)
        {
            std::shared_ptr<SinglePositionMaxDistance> result =
                    std::static_pointer_cast<SinglePositionMaxDistance>(result_it);
            assert (result);

            addToValues(result);
        }

        loginf << "JoinedPositionMaxDistance: updatesToUseChanges: updt num_pos " << num_pos_
               << " num_no_ref " << num_no_ref_ << " num_pos_ok " << num_pos_ok_ << " num_pos_nok " << num_pos_nok_;

        if (num_pos_)
            loginf << "JoinedPositionMaxDistance: updatesToUseChanges: updt result " << result_id_
                   << " pd " << 100.0 * p_min_pos_;
        else
            loginf << "JoinedPositionMaxDistance: updatesToUseChanges: updt result " << result_id_ << " has no data";
    }

}
