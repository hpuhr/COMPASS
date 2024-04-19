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
#include "eval/requirement/base/baseconfig.h"
#include "eval/requirement/base/base.h"
#include "eval/results/single.h"
//#include "eval/results/detection/joined.h"
#include "eval/results/joined.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
//#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"

#include "compass.h"
//#include "dbinterface.h"
//#include "sqliteconnection.h"

#include "logger.h"
#include "stringconv.h"
#include "global.h"
#include "sectorlayer.h"

#include <QProgressDialog>
#include <QApplication>
#include <QThread>
#include <QLabel>
#include <QMessageBox>

#include "util/tbbhack.h"

#include "boost/date_time/posix_time/posix_time.hpp"

#include <future>


using namespace std;
using namespace EvaluationRequirementResult;
using namespace EvaluationResultsReport;
using namespace Utils;

EvaluationResultsGenerator::EvaluationResultsGenerator(
        EvaluationManager& eval_man, EvaluationManagerSettings& eval_settings)
    : eval_man_(eval_man), eval_settings_(eval_settings), results_model_(eval_man_)
{
}

EvaluationResultsGenerator::~EvaluationResultsGenerator()
{
    clear();
}

void EvaluationResultsGenerator::evaluate (EvaluationData& data, EvaluationStandard& standard)
{
    loginf << "EvaluationResultsGenerator: evaluate: skip_no_data_details "
           << eval_settings_.report_skip_no_data_details_
           << " split_results_by_mops " << eval_settings_.report_split_results_by_mops_
           << " report_split_results_by_aconly_ms " << eval_settings_.report_split_results_by_aconly_ms_
           << " show_adsb_info " << eval_settings_.report_show_adsb_info_;

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
            if (!req_group_it->used())
                continue;

            const string& requirement_group_name = req_group_it->name();

            if (!eval_man_.useGroupInSectorLayer(sector_layer_name, requirement_group_name))
                continue; // skip if not used

            num_req_evals += req_group_it->numUsedRequirements() * data.size(); // num reqs * num target
            num_req_evals += req_group_it->numUsedRequirements(); // num reqs for sector sum
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
    {
        //if (target_data_it.use())
            utns.push_back(target_data_it.utn_);
    }

    unsigned int num_utns = utns.size();

    unsigned int eval_cnt = 0;

    boost::posix_time::time_duration time_diff;
    double elapsed_time_s;
    double time_per_eval, remaining_time_s;

    string remaining_time_str;

    string subresult_str;

    for (auto& sec_it : sector_layers)
    {
        const string& sector_layer_name = sec_it->name();

        loginf << "EvaluationResultsGenerator: evaluate: sector layer " << sector_layer_name;

        for (auto& req_group_it : standard)
        {
            if (!req_group_it->used())
                continue;

            const string& requirement_group_name = req_group_it->name();

            if (!eval_man_.useGroupInSectorLayer(sector_layer_name, requirement_group_name))
                continue; // skip if not used

            loginf << "EvaluationResultsGenerator: evaluate: sector layer " << sector_layer_name
                   << " group " << requirement_group_name;

            for (auto& req_cfg_it : *req_group_it)
            {
                if (!req_cfg_it->used())
                    continue;

                loginf << "EvaluationResultsGenerator: evaluate: sector layer " << sector_layer_name
                       << " group " << requirement_group_name
                       << " req '" << req_cfg_it->name() << "'";

                std::shared_ptr<EvaluationRequirement::Base> req = req_cfg_it->createRequirement();
                std::shared_ptr<Joined> result_sum;
                map<string, std::shared_ptr<Joined>> extra_results_sums;

                vector<shared_ptr<Single>> results;
                results.resize(num_utns);

                vector<bool> done_flags;
                done_flags.resize(num_utns, false);
                bool task_done = false;

                // generate results
//                EvaluateTask* t = new (tbb::task::allocate_root()) EvaluateTask(
//                            results, utns, data, req, *sec_it, done_flags, task_done, false);
//                tbb::task::enqueue(*t);

                const SectorLayer& sector_layer = *sec_it;
                bool single_thread = false;

//                int num_threads = oneapi::tbb::info::default_concurrency();
//                loginf << "EvaluateTask: execute: starting, num_threads " << num_threads;

                std::future<void> pending_future = std::async(std::launch::async, [&] {

                    unsigned int num_utns = utns.size();
                    assert (done_flags.size() == num_utns);

                    if (single_thread)
                    {
                        for(unsigned int utn_cnt=0; utn_cnt < num_utns; ++utn_cnt)
                        {
                            results[utn_cnt] = req->evaluate(data.targetData(utns.at(utn_cnt)), req, sector_layer);
                            done_flags[utn_cnt] = true;
                        }
                    }
                    else
                    {
                        tbb::parallel_for(uint(0), num_utns, [&](unsigned int utn_cnt)
                        {
                            //assert(num_threads == oneapi::tbb::this_task_arena::max_concurrency());
                            results[utn_cnt] = req->evaluate(data.targetData(utns.at(utn_cnt)), req, sector_layer);
                            done_flags[utn_cnt] = true;
                        });
                    }

                    for(unsigned int utn_cnt=0; utn_cnt < num_utns; ++utn_cnt)
                        assert (results[utn_cnt]);

                    task_done = true;
                });

//                tbb::task_group g;

//                g.run([&] {
//                    loginf << "EvaluateTask: execute: starting";

//                    unsigned int num_utns = utns.size();
//                    assert (done_flags.size() == num_utns);

//                    if (single_thread)
//                    {
//                        for(unsigned int utn_cnt=0; utn_cnt < num_utns; ++utn_cnt)
//                        {
//                            results[utn_cnt] = req->evaluate(data.targetData(utns.at(utn_cnt)), req, sector_layer);
//                            done_flags[utn_cnt] = true;
//                        }
//                    }
//                    else
//                    {
//                        tbb::parallel_for(uint(0), num_utns, [&](unsigned int utn_cnt)
//                        {
//                            results[utn_cnt] = req->evaluate(data.targetData(utns.at(utn_cnt)), req, sector_layer);
//                            done_flags[utn_cnt] = true;
//                        });
//                    }

//                    for(unsigned int utn_cnt=0; utn_cnt < num_utns; ++utn_cnt)
//                        assert (results[utn_cnt]);
//                });

                unsigned int tmp_done_cnt;

                postprocess_dialog.setLabelText(
                            ("Sector Layer "+sector_layer_name
                             +":\n Requirement: "+req_group_it->name()+":\n    "+req_cfg_it->name()+"\n\n\n").c_str());
                postprocess_dialog.setValue(eval_cnt);

                logdbg << "EvaluationResultsGenerator: evaluate: waiting on group " << req_group_it->name()
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
                                     +":\n  "+req_group_it->name()+":\n    "+req_cfg_it->name()
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

                    result_sum->add(result_it);

                    if (eval_man_.settings().report_split_results_by_mops_)
                    {
                        subresult_str = result_it->target()->mopsVersionStr();

                        if (subresult_str == "?")
                            subresult_str = "Unknown";

                        subresult_str = "MOPS "+subresult_str;

                        if (!extra_results_sums.count(subresult_str+" Sum"))
                            extra_results_sums[subresult_str+" Sum"] =
                                    result_it->createEmptyJoined(subresult_str+" Sum");

                        extra_results_sums.at(subresult_str+" Sum")->add(result_it);
                    }

                    if (eval_man_.settings().report_split_results_by_aconly_ms_)
                    {
                        subresult_str = "Primary";

                        if (result_it->target()->isModeS())
                            subresult_str = "Mode S";
                        else if (result_it->target()->isModeACOnly())
                            subresult_str = "Mode A/C";
                        else
                            assert (result_it->target()->isPrimaryOnly());

                        if (!extra_results_sums.count(subresult_str+" Sum"))
                            extra_results_sums[subresult_str+" Sum"] =
                                    result_it->createEmptyJoined(subresult_str+" Sum");

                        extra_results_sums.at(subresult_str+" Sum")->add(result_it);
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

                for (auto& mops_res_it : extra_results_sums) // add extra results generated by splits
                {
                    loginf << "EvaluationResultsGenerator: evaluate: adding extra result '"
                           << mops_res_it.second->reqGrpId()
                           << "' id '" << mops_res_it.second->resultId() << "'";

                    assert (!results_[mops_res_it.second->reqGrpId()].count(mops_res_it.second->resultId()));
                    results_[mops_res_it.second->reqGrpId()][mops_res_it.second->resultId()] = mops_res_it.second;
                    results_vec_.push_back(mops_res_it.second); // has to be added after all singles
                }

                assert (eval_cnt <= num_req_evals);
                eval_cnt = results_vec_.size();
            }
        }
    }

    updateToChanges();

    elapsed_time = boost::posix_time::microsec_clock::local_time();

    time_diff = elapsed_time - start_time;
    elapsed_time_s = time_diff.total_milliseconds() / 1000.0;

    loginf << "EvaluationResultsGenerator: evaluate: data done " << String::timeStringFromDouble(elapsed_time_s, true);

    // 00:06:22.852 with no parallel

    emit eval_man_.resultsChangedSignal();

    postprocess_dialog.close();

    loginf << "EvaluationResultsGenerator: evaluate: generating results";

    // generating results GUI
    //generateResultsReportGUI();

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

    // add dataset stuff

    Section& gen_sec = root_item->getSection("Overview:General");

    gen_sec.addText("This section contains information about the used application, database and dat sources.");

    gen_sec.addTable("gen_overview_table", 3, {"Name", "Comment", "Value"}, false);

    EvaluationResultsReport::SectionContentTable& gen_table = gen_sec.getTable("gen_overview_table");

    gen_table.addRow({"Application", "Application Filename", APP_FILENAME.c_str()}, nullptr);
    gen_table.addRow({"Application Version", "Application Version", VERSION.c_str()}, nullptr);
    gen_table.addRow({"DB", "Database Name", COMPASS::instance().lastDbFilename().c_str()}, nullptr);

    assert (eval_man_.hasCurrentStandard());
    gen_table.addRow({"Standard", "Standard name", eval_man_.currentStandardName().c_str()}, nullptr);

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
        if (result_it->isSingle() && result_it->use())
        {
            if (cnt % 100 == 0)
                QCoreApplication::processEvents();

            result_it->addToReport(root_item);

            ++cnt;
        }
    }

    // generate non-result details
    addNonResultsContent (root_item);

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

    // first check all single results if should be used
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
    }

    // then do the joined ones
    for (auto& result_it : results_vec_)
    {
        if (result_it->isJoined()) // single result
        {
            shared_ptr<EvaluationRequirementResult::Joined> result =
                static_pointer_cast<EvaluationRequirementResult::Joined>(result_it);

            assert (result);
            result->updateToChanges();
        }
    }

    generateResultsReportGUI();
}

EvaluationResultsGeneratorWidget& EvaluationResultsGenerator::widget()
{
    if (!widget_)
        widget_.reset(new EvaluationResultsGeneratorWidget(*this, eval_man_, eval_settings_));

    return *widget_.get();
}


void EvaluationResultsGenerator::addNonResultsContent (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    // standard
    assert (eval_man_.hasCurrentStandard());
    eval_man_.currentStandard().addToReport(root_item);
}
