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

#include "evaluationtaskresult.h"
#include "evaluationcalculator.h"
#include "evaluationmanager.h"
#include "evalsectionid.h"
#include "eval/results/base/single.h"
#include "eval/results/base/joined.h"

#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/sectioncontenttable.h"
#include "task/result/report/section.h"

#include "compass.h"
#include "logger.h"

#include <QMenu>

const std::string EvaluationTaskResult::FieldTargets    = "targets";
const std::string EvaluationTaskResult::FieldTargetUTN  = "utn";
const std::string EvaluationTaskResult::FieldTargetInfo = "info";

/**
 */
EvaluationTaskResult::EvaluationTaskResult(unsigned int id, 
                                           TaskManager& task_man)
:   TaskResult(id, task_man)
{
}

/**
 */
EvaluationTaskResult::~EvaluationTaskResult() = default;

/**
 */
void EvaluationTaskResult::setTargets(const TargetMap& targets)
{
    targets_ = targets;
}

/**
 */
Result EvaluationTaskResult::createCalculator()
{
    if (!config_.is_object())
    {
        Result::failed("Config not available");
        return false;
    }

    auto& eval_manager = COMPASS::instance().evaluationManager();

    try
    {
        //create calculator based on stored config
        calculator_.reset(new EvaluationCalculator(eval_manager, config_));
    }
    catch(const std::exception& ex)
    {
        return ResultT<EvaluationCalculator*>::failed("Could not create calculator from stored config: " + std::string(ex.what()));
    }
    catch(...)
    {
        return ResultT<EvaluationCalculator*>::failed("Could not create calculator from stored config: Unknown error");
    }

    auto can_eval = calculator_->canEvaluate();
    if (!can_eval.ok())
        return can_eval;

    //update some stuff
    calculator_->updateSectorROI();

    updateInterestSwitches();

    return Result::succeeded();
}

/**
 */
Result EvaluationTaskResult::finalizeResult_impl()
{
    //create calculator
    auto res = createCalculator();
    if (!res.ok())
        return res;

    return Result::succeeded();
}

/**
 */
Result EvaluationTaskResult::update_impl(UpdateEvent evt)
{
    //@TODO: !support partial update!
    calculator_->evaluate(true, true);

    return Result::succeeded();
}

/**
 */
Result EvaluationTaskResult::canUpdate_impl(UpdateEvent evt) const
{
    //true for all kinds of updates
    if (!calculator_)
        return Result::failed("Calculator not initialized");

    auto r = calculator_->canEvaluate();
    if (!r.ok())
        return r;

    return Result::succeeded();
}

namespace helpers
{
    /**
     */
    std::pair<unsigned int, Evaluation::RequirementResultID> singleResultContentProperties(const ResultReport::SectionContent* content)
    {
        assert(content);

        auto info = EvaluationRequirementResult::Single::singleContentProperties(*content);
        assert(info.has_value());

        return std::make_pair(info->first, info->second);
    }

    /**
     */
    Evaluation::RequirementResultID joinedResultContentProperties(const ResultReport::SectionContent* content)
    {
        assert(content);

        auto info = EvaluationRequirementResult::Joined::joinedContentProperties(*content);
        assert(info.has_value());

        return info.value();
    }

    /**
     */
    EvaluationRequirementResult::Single* obtainSingleResult(const ResultReport::SectionContent* content,
                                                            EvaluationCalculator* calculator)
    {
        auto info = singleResultContentProperties(content);

        loginf << "obtainResult: Obtaining result for" 
               << " utn " << info.first 
               << " layer " << info.second.sec_layer_name
               << " group " << info.second.req_group_name
               << " req " << info.second.req_name;

        //result already present?
        auto r = calculator->singleResult(info.second, info.first);
        if (r)
            return r;

        //otherwise evaluate for specified utn and requirement
        calculator->evaluate(true, false, { info.first }, { info.second });

        //then return result
        return calculator->singleResult(info.second, info.first);
    }

    /**
     */
    EvaluationRequirementResult::Joined* obtainJoinedResult(const ResultReport::SectionContent* content,
                                                            EvaluationCalculator* calculator)
    {
        auto info = joinedResultContentProperties(content);

        loginf << "obtainResult: Obtaining result for" 
               << " layer " << info.sec_layer_name
               << " group " << info.req_group_name
               << " req " << info.req_name;

        //result already present?
        auto r = calculator->joinedResult(info);
        if (r)
            return r;

        //otherwise evaluate for specified requirement
        calculator->evaluate(true, false, {}, { info });

        //then return result
        return calculator->joinedResult(info);
    }

    /**
     */
    unsigned int utnFromTable(const ResultReport::SectionContentTable* table, unsigned int row)
    {
        assert(table);
        assert(table->hasColumn("UTN"));
        const auto& d = table->getData(row, "UTN");
        assert (d.is_number_unsigned());

        unsigned int utn = d;

        return utn;
    }
}

/**
 */
bool EvaluationTaskResult::loadOnDemandFigure_impl(ResultReport::SectionContentFigure* figure) const
{
    if (!calculator_)
        return false;

    try
    {
        if (figure->name() == EvaluationRequirementResult::Single::TargetOverviewID)
        {
            //get result for section
            auto result = helpers::obtainSingleResult(figure, calculator_.get());
            if (!result)
            {
                logerr << "EvaluationTaskResult: loadOnDemandFigure_impl: result could not be obtained";
                return false;
            }

            //add overview to figure
            if (!result->addOverviewToFigure(*figure))
            {
                logerr << "EvaluationTaskResult: loadOnDemandFigure_impl: error configuring content";
                return false;
            }

            return true;
        }
    }
    catch(...)
    {
        logerr << "EvaluationTaskResult: loadOnDemandFigure_impl: critical error during load";
    }
    
    return false;
}

/**
 */
bool EvaluationTaskResult::loadOnDemandTable_impl(ResultReport::SectionContentTable* table) const
{
    if (!calculator_)
        return false;

    try
    {
        if (table->name() == EvaluationRequirementResult::Single::TRDetailsTableName)
        {
            //target reports details table in single result section

            //get result for section
            auto result = helpers::obtainSingleResult(table, calculator_.get());
            if (!result)
            {
                logerr << "EvaluationTaskResult: loadOnDemandTable_impl: result could not be obtained";
                return false;
            }

            //add table details
            if (!result->addDetailsToTable(*table))
            {
                logerr << "EvaluationTaskResult: loadOnDemandTable_impl: error configuring content";
                return false;
            }

            return true;
        }
        else if (table->name() == EvaluationData::TargetsTableName)
        {
            //evaluation targets table

            //fill table with target info
            calculator_->data().fillTargetsTable(targets_, *table, 
                [ this ] (const Evaluation::RequirementSumResultID& id) { return this->interestFactorEnabled(id); });

            return true;
        }
    }
    catch(...)
    {
        logerr << "EvaluationTaskResult: loadOnDemandTable_impl: critical error during load";
    }
    
    return false;
}

/**
 */
bool EvaluationTaskResult::loadOnDemandViewable_impl(const ResultReport::SectionContent& content,
                                                     ResultReport::SectionContentViewable& viewable, 
                                                     const QVariant& index,
                                                     unsigned int row) const
{
    if (!calculator_)
        return false;

    if (content.type() == ResultReport::SectionContent::Type::Table)
    {
        if (content.name() == EvaluationRequirementResult::Single::TRDetailsTableName)
        {
            //get result for section
            auto result = helpers::obtainSingleResult(&content, calculator_.get());
            if (!result)
            {
                logerr << "EvaluationTaskResult: loadOnDemandViewable_impl: result could not be obtained";
                return false;
            }

            //configure detail highlight viewable 
            if (!result->addHighlightToViewable(viewable, index))
            {
                logerr << "EvaluationTaskResult: loadOnDemandViewable_impl: error configuring content";
                return false;
            }

            return true;
        }
        else if (content.name() == EvaluationData::TargetsTableName)
        {
            const ResultReport::SectionContentTable* table = dynamic_cast<const ResultReport::SectionContentTable*>(&content);
            assert(table);

            //obtain utn
            auto utn = helpers::utnFromTable(table, row);

            //configure viewable
            auto content = calculator_->getViewableForUTN(utn);
            nlohmann::json j_content = *content;
            viewable.setCallback(j_content);

            return true;
        }
    }

    return false;
}

/**
 */
bool EvaluationTaskResult::customContextMenu_impl(QMenu& menu, 
                                                  ResultReport::SectionContentTable* table, 
                                                  unsigned int row)
{
    logdbg << "EvaluationTaskResult: customContextMenu_impl";

    if (table->name() == EvaluationRequirementResult::Single::TRDetailsTableName)
    {
        //target report details table in single result section
    }
    else if (table->name() == EvaluationRequirementResult::Joined::TargetsTableName)
    {
        //target table in joined result section
        auto info = helpers::joinedResultContentProperties(table);
        auto utn  = helpers::utnFromTable(table, row);
        
        loginf << "EvaluationTaskResult: customContextMenu_impl: Context menu requested for utn " << utn;

        if (calculator_)
        {
            auto action_show_utn = menu.addAction("Show Full UTN");
            QObject::connect (action_show_utn, &QAction::triggered, [ = ] () { this->showFullUTN(utn); });

            auto action_show_data = menu.addAction("Show Surrounding Data");
            QObject::connect (action_show_data, &QAction::triggered, [ = ] () { this->showSurroundingData(utn); });
        }

        //@TODO: jump to requirement

        return true;
    }
    else if (table->name() == EvaluationRequirementResult::Base::RequirementOverviewTableName)
    {
        //requirement table in overview section
    }
    else if (table->name() == EvaluationData::TargetsTableName)
    {
        //evaluation target table
        auto utn  = helpers::utnFromTable(table, row);

        loginf << "EvaluationTaskResult: customContextMenu_impl: Context menu requested for utn " << utn;

        if (calculator_)
        {
            auto action_show_utn = menu.addAction("Show Full UTN");
            QObject::connect (action_show_utn, &QAction::triggered, [ = ] () { this->showFullUTN(utn); });

            auto action_show_data = menu.addAction("Show Surrounding Data");
            QObject::connect (action_show_data, &QAction::triggered, [ = ] () { this->showSurroundingData(utn); });
        }

        createRequirementLinkMenu(utn, menu);

        return true;
    }

    // unsigned int row_index = source_index.row();

    // if (result_ptrs_.at(row_index) && result_ptrs_.at(row_index)->isSingle())
    // {
    //     EvaluationRequirementResult::Single* single_result =
    //             static_cast<EvaluationRequirementResult::Single*>(result_ptrs_.at(row_index));
    //     assert (single_result);

    //     QMenu menu;

    //     unsigned int utn = single_result->utn();
    //     loginf << "SectionContentTable: customContextMenuSlot: utn " << utn;

    //     assert (eval_man_.getData().hasTargetData(utn));

    //     const EvaluationTargetData& target_data = eval_man_.getData().targetData(utn);

    //     if (target_data.use())
    //     {
    //         QAction* action = new QAction("Remove", this);
    //         connect (action, &QAction::triggered, this, &SectionContentTable::removeUTNSlot);
    //         action->setData(utn);

    //         menu.addAction(action);
    //     }
    //     else
    //     {
    //         QAction* action = new QAction("Add", this);
    //         connect (action, &QAction::triggered, this, &SectionContentTable::addUTNSlot);
    //         action->setData(utn);

    //         menu.addAction(action);
    //     }

    //     QAction* action = new QAction("Show Full UTN", this);
    //     connect (action, &QAction::triggered, this, &SectionContentTable::showFullUTNSlot);
    //     action->setData(utn);
    //     menu.addAction(action);

    //     QAction* action2 = new QAction("Show Surrounding Data", this);
    //     connect (action2, &QAction::triggered, this, &SectionContentTable::showSurroundingDataSlot);
    //     action2->setData(utn);
    //     menu.addAction(action2);

    //     menu.exec(table_view_->viewport()->mapToGlobal(p));
    // }
    // else
    //     loginf << "SectionContentTable: customContextMenuSlot: no associated utn";

    return false;
}

/**
 */
bool EvaluationTaskResult::customContextMenu_impl(QMenu& menu, 
                                                  ResultReport::SectionContent* content)
{
    if (!calculator_)
        return false;

    if (content->type() == ResultReport::SectionContent::Type::Table)
    {
        if (content->name() == EvaluationData::TargetsTableName)
        {
            const ResultReport::SectionContentTable* table = dynamic_cast<const ResultReport::SectionContentTable*>(content);
            assert(table);

            createInterestMenu(menu);

            return true;
        }
    }

    return false;
}

/**
 */
void EvaluationTaskResult::postprocessTable_impl(ResultReport::SectionContentTable* table)
{
    if (table->name() == EvaluationRequirementResult::Single::TRDetailsTableName)
    {
        //target report details table in single result section
    }
    else if (table->name() == EvaluationRequirementResult::Joined::TargetsTableName)
    {
        //target table in joined result section
    }
    else if (table->name() == EvaluationRequirementResult::Base::RequirementOverviewTableName)
    {
        //requirement table in overview section
    }
    else if (table->name() == EvaluationData::TargetsTableName)
    {
        //createInterestMenu()


        //evaluation target table
        //auto this_unconst = const_cast<EvaluationTaskResult*>(this);

        // table->registerCallBack("Rerun Evaluation", 
        //     [ this_unconst ] () 
        //     { 
        //         this_unconst->informUpdate(UpdateEvent::Complete);
        //         this_unconst->update(true);
        //     });

        // std::string section = table->parentSection()->id();
        // std::string name    = table->name();

        // table->registerCallBack("Update Target Table", 
        //     [ this_unconst, section, name ] () 
        //     { 
        //         this_unconst->updateTargets();
        //         this_unconst->informUpdate(UpdateEvent::Content, std::make_pair(section, name));
        //         this_unconst->update(true);
        //     });
    }
}

/**
 */
void EvaluationTaskResult::updateTargets()
{
    auto& dbcontent_man = COMPASS::instance().dbContentManager();

    for (auto& t : targets_)
        EvaluationTargetData::updateTarget(dbcontent_man, t.second);
}

/**
 */
void EvaluationTaskResult::showUTN(unsigned int utn) const
{
    if (!calculator_)
        return;

    calculator_->showUTN(utn);
}

/**
 */
void EvaluationTaskResult::showFullUTN(unsigned int utn) const
{
    if (!calculator_)
        return;

    calculator_->showFullUTN(utn);
}

/**
 */
void EvaluationTaskResult::showSurroundingData(unsigned int utn) const
{
    if (targets_.count(utn) == 0)
    {
        logerr << "EvaluationTaskResult: showSurroundingData: utn " << utn << " not found in targets";
        return;
    }

    if (!calculator_)
        return;

    calculator_->showSurroundingData(targets_.at(utn));
}

/**
 */
void EvaluationTaskResult::updateInterestSwitches()
{
    interest_factor_enabled_.clear();

    if (!calculator_)
        return;

    auto req_names = calculator_->currentRequirementNames();

    for (const auto& req : req_names)
    {
        interest_factor_enabled_[ req ] = true;
    }
}

/**
 */
const std::map<std::string, bool>& EvaluationTaskResult::interestSwitches() const
{
    return interest_factor_enabled_;
}

/**
 */
bool EvaluationTaskResult::interestFactorEnabled(const Evaluation::RequirementSumResultID& id) const
{
    auto it = interest_factor_enabled_.find(id.req_name);
    if (it == interest_factor_enabled_.end())
        return false;

    return it->second;
}

/**
 */
void EvaluationTaskResult::setInterestFactorEnabled(const Evaluation::RequirementSumResultID& id, bool ok)
{
    assert(calculator_);

    interest_factor_enabled_.at(id.req_name) = ok;

    informUpdate(UpdateEvent::Content, std::make_pair(EvaluationData::SectionID, EvaluationData::TargetsTableName));
    update(true);
}

/**
 */
void EvaluationTaskResult::setInterestFactorEnabled(const std::string& req_name, bool ok)
{
    assert(calculator_);

    interest_factor_enabled_.at(req_name) = ok;

    informUpdate(UpdateEvent::Content, std::make_pair(EvaluationData::SectionID, EvaluationData::TargetsTableName));
    update(true);
}

/**
 */
void EvaluationTaskResult::setInterestFactorsEnabled(bool ok)
{
    for (auto& it : interest_factor_enabled_)
        it.second = ok;

    informUpdate(UpdateEvent::Content, std::make_pair(EvaluationData::SectionID, EvaluationData::TargetsTableName));
    update(true);
}

/**
 */
EvaluationTarget::InterestMap EvaluationTaskResult::activeInterestFactors(unsigned int utn) const
{
    EvaluationTarget::InterestMap interest_factors;

    assert(targets_.count(utn));

    const auto& target = targets_.at(utn);

    auto ifactors = target.interestFactors();

    for (const auto& ifactor : ifactors)
    {
        const auto& id = ifactor.first;
        if (!interestFactorEnabled(id))
            continue;

        interest_factors[ id ] = ifactor.second;
    }

    return interest_factors;
}

/**
 */
void EvaluationTaskResult::createInterestMenu(QMenu& menu)
{
    if (interest_factor_enabled_.empty())
        return;

    auto interest_menu = menu.addMenu("Edit Shown Interest Factors");

    auto action_all  = interest_menu->addAction("Show All");
    auto action_none = interest_menu->addAction("Show None");

    QObject::connect(action_all , &QAction::triggered, [ this ] () { this->setInterestFactorsEnabled(true);  });
    QObject::connect(action_none, &QAction::triggered, [ this ] () { this->setInterestFactorsEnabled(false); });

    for (const auto& ife : interest_factor_enabled_)
    {
        std::string req_name = ife.first;

        auto action = interest_menu->addAction(QString::fromStdString(ife.first));
        action->setCheckable(true);
        action->setChecked(ife.second);

        auto clickCB = [ this, req_name ] (bool ok) 
        { 
            this->setInterestFactorEnabled(req_name, ok);
        };

        QObject::connect(action, &QAction::triggered, clickCB);
    }
}

/**
 */
void EvaluationTaskResult::createRequirementLinkMenu(unsigned int utn, QMenu& menu)
{
    auto ifactors = activeInterestFactors(utn);

    if (!ifactors.empty())
    {
        //menu.addSeparator();

        auto req_menu = menu.addMenu("Jump to Requirement");

        for (const auto& ifactor : ifactors)
        {
            const auto& id = ifactor.first;

            QAction* action = EvaluationTargetData::interestFactorAction(id, ifactor.second);

            req_menu->addAction(action);

            QObject::connect(action, &QAction::triggered, [ this, id, utn ] () { this->jumpToRequirement(id, utn, true); });
        }
    }
}

/**
 */
void EvaluationTaskResult::jumpToRequirement(const Evaluation::RequirementSumResultID& id, 
                                             unsigned int utn, 
                                             bool show_image)
{
    if (!calculator_ || !report_)
        return;

    std::string sum_id = EvalSectionID::requirementResultSumID(id);

    loginf << "EvaluationTaskResult: jumpToRequirement: sum id: " << sum_id;

    std::string utn_id = EvalSectionID::createForTargetResult(utn, id);

    loginf << "EvaluationTaskResult: jumpToRequirement: utn id: " << utn_id;

    report_->setCurrentSection(utn_id, show_image);
}

/**
 */
void EvaluationTaskResult::toJSON_impl(nlohmann::json& root_node) const
{
    //write evaluation targets
    auto j_targets = nlohmann::json::array();

    for (const auto& target_it : targets_)
    {
        nlohmann::json j_target;
        j_target[ FieldTargetUTN  ] = target_it.first;
        j_target[ FieldTargetInfo ] = target_it.second.info();

        j_targets.push_back(j_target);
    }

    root_node[ FieldTargets ] = j_targets;
}

/**
 */
bool EvaluationTaskResult::fromJSON_impl(const nlohmann::json& j)
{
    if (!j.contains(FieldTargets))
        return false;

    // read evaluation targets
    const auto& j_targets = j[ FieldTargets ];
    if (!j_targets.is_array())
        return false;

    for (const auto& j_target : j_targets)
    {
        if (!j_target.is_object()              ||
            !j_target.contains(FieldTargetUTN) ||
            !j_target.contains(FieldTargetInfo))
            return false;

        unsigned int utn  = j_target[ FieldTargetUTN  ];
        const auto&  info = j_target[ FieldTargetInfo ];

        if(!info.is_object())
            return false;

        targets_.emplace(utn, EvaluationTarget(utn, info));
    }

    return true;
}
