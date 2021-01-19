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

#include "eval/results/identification/correctsingle.h"
#include "eval/results/identification/correctjoined.h"
#include "eval/requirement/base/base.h"
#include "eval/requirement/identification/correct.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"

#include <cassert>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

    JoinedIdentificationCorrect::JoinedIdentificationCorrect(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer, EvaluationManager& eval_man)
        : Joined("JoinedIdentificationCorrect", result_id, requirement, sector_layer, eval_man)
    {
    }


    void JoinedIdentificationCorrect::join(std::shared_ptr<Base> other)
    {
        Joined::join(other);

        std::shared_ptr<SingleIdentificationCorrect> other_sub =
                std::static_pointer_cast<SingleIdentificationCorrect>(other);
        assert (other_sub);

        addToValues(other_sub);
    }

    void JoinedIdentificationCorrect::addToValues (std::shared_ptr<SingleIdentificationCorrect> single_result)
    {
        assert (single_result);

        if (!single_result->use())
            return;

        num_updates_ += single_result->numUpdates();
        num_no_ref_pos_ += single_result->numNoRefPos();
        num_no_ref_id_ += single_result->numNoRefId();
        num_pos_outside_ += single_result->numPosOutside();
        num_pos_inside_ += single_result->numPosInside();
        num_correct_ += single_result->numCorrect();
        num_not_correct_ += single_result->numNotCorrect();

        updatePID();
    }

    void JoinedIdentificationCorrect::updatePID()
    {
        assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
        assert (num_pos_inside_ == num_no_ref_id_+ num_correct_+num_not_correct_);

        if (num_correct_+num_not_correct_)
        {
            pid_ = (float)num_correct_/(float)(num_correct_+num_not_correct_);
            has_pid_ = true;
        }
        else
        {
            pid_ = 0;
            has_pid_ = false;
        }
    }

//    void JoinedIdentification::print()
//    {
//        std::shared_ptr<EvaluationRequirement::Identification> req =
//                std::static_pointer_cast<EvaluationRequirement::Identification>(requirement_);
//        assert (req);

//        if (has_pid_)
//            loginf << "JoinedIdentification: print: req. name " << req->name()
//                   << " pid " << String::percentToString(100.0 * pid_)
//                   << " passed " << (pid_ >= req->minimumProbability());
//        else
//            loginf << "JoinedIdentification: print: req. name " << req->name()
//                   << " has no data";
//    }

    void JoinedIdentificationCorrect::addToReport (
            std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        logdbg << "JoinedIdentification " <<  requirement_->name() <<": addToReport";

        if (!results_.size()) // some data must exist
        {
            logerr << "JoinedIdentification " <<  requirement_->name() <<": addToReport: no data";
            return;
        }

        logdbg << "JoinedIdentification " <<  requirement_->name() << ": addToReport: adding joined result";

        addToOverviewTable(root_item);
        addDetails(root_item);
    }

    void JoinedIdentificationCorrect::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

        // condition
        std::shared_ptr<EvaluationRequirement::IdentificationCorrect> req =
                std::static_pointer_cast<EvaluationRequirement::IdentificationCorrect>(requirement_);
        assert (req);

        // pd
        QVariant pd_var;

        string result {"Unknown"};

        if (has_pid_)
        {
            pd_var = String::percentToString(pid_ * 100.0).c_str();

            result = req-> getResultConditionStr(pid_);
        }

        // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
        ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                         requirement_->shortname().c_str(),
                         result_id_.c_str(), {num_correct_+num_not_correct_},
                         pd_var, req->getConditionStr().c_str(), result.c_str()}, this, {});
    }

    void JoinedIdentificationCorrect::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

        if (!sector_section.hasTable("sector_details_table"))
            sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

        EvaluationResultsReport::SectionContentTable& sec_det_table =
                sector_section.getTable("sector_details_table");

        addCommonDetails(sec_det_table);

        sec_det_table.addRow({"#Updates", "Total number target reports", num_updates_}, this);
        sec_det_table.addRow({"#NoRef [1]", "Number of updates w/o reference position or identification",
                                num_no_ref_pos_+num_no_ref_id_}, this);
        sec_det_table.addRow({"#NoRefPos [1]", "Number of updates w/o reference position ", num_no_ref_pos_}, this);
        sec_det_table.addRow({"#NoRef [1]", "Number of updates w/o reference identification", num_no_ref_id_}, this);
        sec_det_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
        sec_det_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
        sec_det_table.addRow({"#CID [1]", "Number of updates with correct identification", num_correct_}, this);
        sec_det_table.addRow({"#NCID [1]", "Number of updates with no correct identification", num_not_correct_}, this);

        // condition
        std::shared_ptr<EvaluationRequirement::IdentificationCorrect> req =
                std::static_pointer_cast<EvaluationRequirement::IdentificationCorrect>(requirement_);
        assert (req);

        // pd
        QVariant pd_var;

        string result {"Unknown"};

        if (has_pid_)
        {
            pd_var = String::percentToString(pid_ * 100.0).c_str();

            result = req-> getResultConditionStr(pid_);
        }

        sec_det_table.addRow({"POK [%]", "Probability of correct identification", pd_var}, this);
        sec_det_table.addRow({"Condition", {}, req->getConditionStr().c_str()}, this);
        sec_det_table.addRow({"Condition Fulfilled", {}, result.c_str()}, this);

        // figure
        if (has_pid_ && pid_ != 1.0)
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


    bool JoinedIdentificationCorrect::hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        //loginf << "UGA4 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

        if (table.name() == req_overview_table_name_)
            return true;
        else
            return false;
    }

    std::unique_ptr<nlohmann::json::object_t> JoinedIdentificationCorrect::viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasViewableData(table, annotation));

        return getErrorsViewable();
    }

    std::unique_ptr<nlohmann::json::object_t> JoinedIdentificationCorrect::getErrorsViewable ()
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

    bool JoinedIdentificationCorrect::hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        //loginf << "UGA5 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

        if (table.name() == req_overview_table_name_)
            return true;
        else
            return false;;
    }

    std::string JoinedIdentificationCorrect::reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasReference(table, annotation));
        return "Report:Results:"+getRequirementSectionID();

        return nullptr;
    }

    void JoinedIdentificationCorrect::updatesToUseChanges()
    {
        num_updates_ = 0;
        num_no_ref_pos_ = 0;
        num_no_ref_id_ = 0;
        num_pos_outside_ = 0;
        num_pos_inside_ = 0;
        num_correct_ = 0;
        num_not_correct_ = 0;

        for (auto result_it : results_)
        {
            std::shared_ptr<SingleIdentificationCorrect> result =
                    std::static_pointer_cast<SingleIdentificationCorrect>(result_it);
            assert (result);

            addToValues(result);
        }
    }

}
