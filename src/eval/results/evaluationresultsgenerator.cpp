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

#include "evaluationresultsgenerator.h"
#include "evaluationmanager.h"
#include "evaluationdata.h"
#include "evaluationstandard.h"
#include "eval/requirement/group.h"
#include "eval/requirement/config.h"
#include "eval/requirement/base.h"
#include "eval/results/single.h"
#include "eval/results/detection/joined.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "logger.h"
#include "stringconv.h"

#include <QProgressDialog>
#include <QApplication>
#include <QThread>
#include <QLabel>
#include <QMessageBox>

#include "boost/date_time/posix_time/posix_time.hpp"

#include <tbb/tbb.h>

using namespace std;
using namespace EvaluationRequirementResult;
using namespace Utils;

EvaluationResultsGenerator::EvaluationResultsGenerator(const std::string& class_id, const std::string& instance_id,
                                                       EvaluationManager& eval_man)
    : Configurable(class_id, instance_id, &eval_man, "eval_results.json"),
      eval_man_(eval_man), results_model_(eval_man_)
{
    registerParameter("skip_no_data_details", &skip_no_data_details_, true);
    registerParameter("split_results_by_mops", &split_results_by_mops_, false);
    registerParameter("show_adsb_info", &show_adsb_info_, false);


    createSubConfigurables();
}

EvaluationResultsGenerator::~EvaluationResultsGenerator()
{
    clear();
}

void EvaluationResultsGenerator::generateSubConfigurable(const std::string& class_id,
                                                         const std::string& instance_id)
{
    assert (false);
}

void EvaluationResultsGenerator::checkSubConfigurables()
{
}

void EvaluationResultsGenerator::evaluate (EvaluationData& data, EvaluationStandard& standard)
{
    loginf << "EvaluationResultsGenerator: evaluate";

    boost::posix_time::ptime start_time;
    boost::posix_time::ptime elapsed_time;

    start_time = boost::posix_time::microsec_clock::local_time();

    std::vector<std::shared_ptr<SectorLayer>>& sector_layers = eval_man_.sectorsLayers();

    unsigned int num_req_evals = 0;
    for (auto& sec_it : sector_layers)
    {
        const string& sector_layer_name = sec_it->name();

        for (auto& req_group_it : standard)
        {
            const string& requirement_group_name = req_group_it.first;

            if (!eval_man_.useGroupInSectorLayer(sector_layer_name, requirement_group_name))
                continue; // skip if not used

            num_req_evals += req_group_it.second->size() * data.size(); // num reqs * num target
            num_req_evals += req_group_it.second->size(); // num reqs for sector sum
        }
    }

    QProgressDialog postprocess_dialog ("", "", 0, num_req_evals);
    postprocess_dialog.setWindowTitle("Evaluating");
    postprocess_dialog.setCancelButton(nullptr);
    postprocess_dialog.setWindowModality(Qt::ApplicationModal);

    QLabel* progress_label = new QLabel("", &postprocess_dialog);
    progress_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    postprocess_dialog.setLabel(progress_label);

    postprocess_dialog.show();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    clear();

    vector<unsigned int> utns;

    for (auto& target_data_it : data)
        utns.push_back(target_data_it.utn_);

    unsigned int num_utns = utns.size();

    unsigned int eval_cnt = 0;

    boost::posix_time::time_duration time_diff;
    double elapsed_time_s;
    double time_per_eval, remaining_time_s;

    string remaining_time_str;

    string mops_str;

    for (auto& sec_it : sector_layers)
    {
        const string& sector_layer_name = sec_it->name();

        loginf << "EvaluationResultsGenerator: evaluate: sector layer " << sector_layer_name;

        for (auto& req_group_it : standard)
        {
            const string& requirement_group_name = req_group_it.first;

            if (!eval_man_.useGroupInSectorLayer(sector_layer_name, requirement_group_name))
                continue; // skip if not used

            loginf << "EvaluationResultsGenerator: evaluate: sector layer " << sector_layer_name
                   << " group " << req_group_it.first;

            for (auto& req_cfg_it : *req_group_it.second)
            {
                loginf << "EvaluationResultsGenerator: evaluate: sector layer " << sector_layer_name
                       << " group " << req_group_it.first
                       << " req '" << req_cfg_it->name() << "'";

                std::shared_ptr<EvaluationRequirement::Base> req = req_cfg_it->createRequirement();
                std::shared_ptr<Joined> result_sum;
                map<string, std::shared_ptr<Joined>> mops_sums;

                vector<shared_ptr<Single>> results;
                results.resize(num_utns);

                vector<bool> done_flags;
                done_flags.resize(num_utns, false);
                bool task_done = false;

                // generate results
                EvaluateTask* t = new (tbb::task::allocate_root()) EvaluateTask(
                            results, utns, data, req, *sec_it, done_flags, task_done, false);
                tbb::task::enqueue(*t);

                unsigned int tmp_done_cnt;

                postprocess_dialog.setLabelText(
                            ("Sector Layer "+sector_layer_name
                             +":\n Requirement: "+req_group_it.first+":"+req_cfg_it->name()).c_str());
                postprocess_dialog.setValue(eval_cnt);

                logdbg << "EvaluationResultsGenerator: evaluate: waiting on group " << req_group_it.first
                       << " req '" << req_cfg_it->name() << "'";

                while (!task_done)
                {
                    tmp_done_cnt = 0;

                    for (auto done_it : done_flags)
                    {
                        if (done_it)
                            tmp_done_cnt++;
                    }

                    //assert (eval_cnt+tmp_done_cnt <= num_req_evals);
                    // hack
                    if (eval_cnt+tmp_done_cnt <= num_req_evals && tmp_done_cnt)
                    {

                        elapsed_time = boost::posix_time::microsec_clock::local_time();

                        time_diff = elapsed_time - start_time;
                        elapsed_time_s = time_diff.total_milliseconds() / 1000.0;

                        time_per_eval = elapsed_time_s/(double)(eval_cnt+tmp_done_cnt);
                        remaining_time_s = (double)(num_req_evals-eval_cnt-tmp_done_cnt)*time_per_eval;

                        postprocess_dialog.setLabelText(
                                    ("Sector Layer "+sector_layer_name
                                     +":\n  "+req_group_it.first+": "+req_cfg_it->name()
                                     +"\n\nElapsed: "+String::timeStringFromDouble(elapsed_time_s, false)
                                     +"\nRemaining: "+String::timeStringFromDouble(remaining_time_s, false)
                                     +" (estimated)").c_str());

                        postprocess_dialog.setValue(eval_cnt+tmp_done_cnt);
                    }

                    if (!task_done)
                    {
                        QCoreApplication::processEvents();
                        QThread::msleep(200);
                    }
                }

                postprocess_dialog.setLabelText(("Sector Layer "+sector_layer_name+":\nAggregating results").c_str());

                for (auto& result_it : results)
                {
                    results_[result_it->reqGrpId()][result_it->resultId()] = result_it;
                    results_vec_.push_back(result_it);

                    if (!result_sum)
                        result_sum = result_it->createEmptyJoined("Sum");

                    result_sum->join(result_it);

                    if (split_results_by_mops_)
                    {
                        mops_str = result_it->target()->mopsVersionsStr();

                        if (!mops_str.size())
                            mops_str = "N/A";

                        mops_str = "MOPS "+mops_str;

                        if (!mops_sums.count(mops_str+" Sum"))
                            mops_sums[mops_str+" Sum"] =
                                    result_it->createEmptyJoined(mops_str+" Sum");

                        mops_sums.at(mops_str+" Sum")->join(result_it);
                    }
                }

                if (result_sum)
                {
                    loginf << "EvaluationResultsGenerator: evaluate: adding result '" << result_sum->reqGrpId()
                           << "' id '" << result_sum->resultId() << "'";
                    assert (!results_[result_sum->reqGrpId()].count(result_sum->resultId()));
                    results_[result_sum->reqGrpId()][result_sum->resultId()] = result_sum;
                    results_vec_.push_back(result_sum); // has to be added after all singles
                }

                if (split_results_by_mops_)
                {
                    for (auto& mops_res_it : mops_sums)
                    {
                        loginf << "EvaluationResultsGenerator: evaluate: adding result '"
                               << mops_res_it.second->reqGrpId()
                               << "' id '" << mops_res_it.second->resultId() << "'";

                        assert (!results_[mops_res_it.second->reqGrpId()].count(mops_res_it.second->resultId()));
                        results_[mops_res_it.second->reqGrpId()][mops_res_it.second->resultId()] = mops_res_it.second;
                        results_vec_.push_back(mops_res_it.second); // has to be added after all singles
                    }
                }

                assert (eval_cnt <= num_req_evals);
                eval_cnt = results_vec_.size();
            }
        }
    }

    elapsed_time = boost::posix_time::microsec_clock::local_time();

    time_diff = elapsed_time - start_time;
    elapsed_time_s = time_diff.total_milliseconds() / 1000.0;

    loginf << "EvaluationResultsGenerator: evaluate: data done " << String::timeStringFromDouble(elapsed_time_s, true);

    // 00:06:22.852 with no parallel

    emit eval_man_.resultsChangedSignal();

    postprocess_dialog.close();

    loginf << "EvaluationResultsGenerator: evaluate: generating results";

    // generating results GUI
    generateResultsReportGUI();

    loginf << "EvaluationResultsGenerator: evaluate: done " << String::timeStringFromDouble(elapsed_time_s, true);

    QApplication::restoreOverrideCursor();
}

void EvaluationResultsGenerator::clear()
{
    // clear everything
    results_model_.beginReset();
    results_model_.clear();
    results_.clear();
    results_vec_.clear();
    results_model_.endReset();
}

void EvaluationResultsGenerator::generateResultsReportGUI()
{
    loginf << "EvaluationResultsGenerator: generateResultsReportGUI";

    boost::posix_time::ptime loading_start_time;
    boost::posix_time::ptime loading_stop_time;

    loading_start_time = boost::posix_time::microsec_clock::local_time();

    QMessageBox msg_box; // QApplication::topLevelWidgets().first()
    msg_box.setWindowTitle("Updating Results");
    msg_box.setText( "Please wait...");
    msg_box.setStandardButtons(QMessageBox::NoButton);
    msg_box.setWindowModality(Qt::ApplicationModal);
    msg_box.show();

    // prepare for new data
    results_model_.beginReset();

    std::shared_ptr<EvaluationResultsReport::RootItem> root_item = results_model_.rootItem();

    // generate results

    // first add all joined
    for (auto& result_it : results_vec_)
    {
        if (result_it->isJoined())
        {
            QCoreApplication::processEvents();
            result_it->addToReport(root_item);
        }
    }

    // then all singles

    unsigned int cnt = 0;
    for (auto& result_it : results_vec_)
    {
        if (result_it->isSingle())
        {
            if (cnt % 100 == 0)
                QCoreApplication::processEvents();

            result_it->addToReport(root_item);

            ++cnt;
        }
    }

    results_model_.endReset();

    loading_stop_time = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = loading_stop_time - loading_start_time;
    load_time = diff.total_milliseconds() / 1000.0;

    msg_box.close();

    loginf << "EvaluationResultsGenerator: generateResultsReportGUI: done "
           << String::timeStringFromDouble(load_time, true);
}

EvaluationResultsReport::TreeModel& EvaluationResultsGenerator::resultsModel()
{
    return results_model_;
}

void EvaluationResultsGenerator::updateToChanges ()
{
    loginf << "EvaluationResultsGenerator: updateToChanges";

    // clear everything
    results_model_.beginReset();
    results_model_.clear();
    results_model_.endReset();

    for (auto& result_it : results_vec_)
    {
        if (result_it->isSingle()) // single result
        {
            // s starts with prefix

            shared_ptr<EvaluationRequirementResult::Single> result =
                    static_pointer_cast<EvaluationRequirementResult::Single>(result_it);

            assert (result);
            result->updateUseFromTarget();
        }
        else
        {
            assert (result_it->isJoined()); // joined result

            shared_ptr<EvaluationRequirementResult::Joined> result =
                    static_pointer_cast<EvaluationRequirementResult::Joined>(result_it);

            assert (result);
            result->updatesToUseChanges();
        }
    }

    generateResultsReportGUI();
}

bool EvaluationResultsGenerator::skipNoDataDetails() const
{
    return skip_no_data_details_;
}

void EvaluationResultsGenerator::skipNoDataDetails(bool value)
{
    skip_no_data_details_ = value;
}

bool EvaluationResultsGenerator::splitResultsByMOPS() const
{
    return split_results_by_mops_;
}

void EvaluationResultsGenerator::splitResultsByMOPS(bool value)
{
    split_results_by_mops_ = value;
}

bool EvaluationResultsGenerator::showAdsbInfo() const
{
    return show_adsb_info_;
}

void EvaluationResultsGenerator::showAdsbInfo(bool value)
{
    show_adsb_info_ = value;
}

