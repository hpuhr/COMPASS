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

#include "eval/requirement/group.h"
#include "evaluationstandard.h"
#include "eval/requirement/detection/detectionconfig.h"
#include "eval/requirement/position/distanceconfig.h"
#include "eval/requirement/position/distancermsconfig.h"
#include "eval/requirement/position/radarrangeconfig.h"
#include "eval/requirement/position/radarazimuthconfig.h"
#include "eval/requirement/position/alongconfig.h"
#include "eval/requirement/position/acrossconfig.h"
#include "eval/requirement/position/latencyconfig.h"
#include "eval/requirement/speed/speedconfig.h"
#include "eval/requirement/trackangle/trackangleconfig.h"
#include "eval/requirement/identification/correctconfig.h"
#include "eval/requirement/identification/falseconfig.h"
#include "eval/requirement/identification/correct_period.h"
#include "eval/requirement/mode_a/presentconfig.h"
#include "eval/requirement/mode_a/falseconfig.h"
#include "eval/requirement/mode_c/falseconfig.h"
#include "eval/requirement/mode_c/presentconfig.h"
#include "eval/requirement/mode_c/correctconfig.h"
#include "eval/requirement/mode_c/correct_period.h"
#include "eval/requirement/extra/dataconfig.h"
#include "eval/requirement/extra/trackconfig.h"
#include "eval/requirement/dubious/dubioustrackconfig.h"
#include "eval/requirement/dubious/dubioustargetconfig.h"
#include "eval/requirement/generic/genericconfig.h"
#include "logger.h"

#include <QInputDialog>
#include <QMessageBox>

#include <algorithm>

using namespace std;

const std::map<std::string, std::string> Group::requirement_type_mapping_
{
    {"EvaluationRequirementExtraDataConfig", "Extra Data"},
    {"EvaluationRequirementExtraTrackConfig", "Extra Track"},
    {"EvaluationRequirementDubiousTargetConfig", "Dubious Target"},
    {"EvaluationRequirementDubiousTrackConfig", "Dubious Track"},
    {"EvaluationRequirementDetectionConfig", "Detection"},
    {"EvaluationRequirementIdentificationCorrectConfig", "Identification Correct"},
    {"EvaluationRequirementIdentificationFalseConfig", "Identification False"},
    {"EvaluationRequirementIdentificationCorrectPeriodConfig", "Identification Correct (Periods)"},
    {"EvaluationRequirementModeAPresentConfig", "Mode 3/A Present"},
    {"EvaluationRequirementModeAFalseConfig", "Mode 3/A False"},
    {"EvaluationRequirementModeCPresentConfig", "Mode C Present"},
    {"EvaluationRequirementModeCCorrectConfig", "Mode C Correct"},
    {"EvaluationRequirementModeCFalseConfig", "Mode C False"},
    {"EvaluationRequirementModeCCorrectPeriodConfig", "Mode C Correct (Periods)"},
    {"EvaluationRequirementPositionDistanceConfig", "Position Distance"},
    {"EvaluationRequirementPositionDistanceRMSConfig", "Position Distance RMS"},
    {"EvaluationRequirementPositionRadarRangeConfig", "Position Radar Range"},
    {"EvaluationRequirementPositionRadarAzimuthConfig", "Position Radar Azimuth"},
    {"EvaluationRequirementPositionAlongConfig", "Position Along"},
    {"EvaluationRequirementPositionAcrossConfig", "Position Across"},
    {"EvaluationRequirementPositionLatencyConfig", "Position Latency"},
    {"EvaluationRequirementSpeedConfig", "Speed"},
    {"EvaluationRequirementTrackAngleConfig", "TrackAngle"},
    {"EvaluationRequirementMoMLongAccConfig", "MoM Longitutidinal Acceleration Correct"},
    {"EvaluationRequirementMoMTransAccConfig", "MoM Transversal Acceleration Correct"},
    {"EvaluationRequirementMoMVertRateConfig", "MoM Vertical Rate Correct"},
    {"EvaluationRequirementROCDCorrectConfig", "ROCD Correct"},
    {"EvaluationRequirementAccelerationCorrectConfig", "Acceleration Correct"}
};


Group::Group(const std::string& class_id, const std::string& instance_id,
                                                       EvaluationStandard& standard, EvaluationManager& eval_man)
    : Configurable(class_id, instance_id, &standard), EvaluationStandardTreeItem(&standard), standard_(standard),
      eval_man_(eval_man)
{
    registerParameter("name", &name_, std::string());

    assert (name_.size());

    createSubConfigurables();
}

Group::~Group()
{
}

void Group::generateSubConfigurable(const std::string& class_id,
                                                         const std::string& instance_id)
{
    if (class_id == "EvaluationRequirementExtraDataConfig")
    {
        EvaluationRequirement::ExtraDataConfig* config =
                new EvaluationRequirement::ExtraDataConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementExtraTrackConfig")
    {
        EvaluationRequirement::ExtraTrackConfig* config =
                new EvaluationRequirement::ExtraTrackConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementDubiousTrackConfig")
    {
        EvaluationRequirement::DubiousTrackConfig* config =
                new EvaluationRequirement::DubiousTrackConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementDubiousTargetConfig")
    {
        EvaluationRequirement::DubiousTargetConfig* config =
                new EvaluationRequirement::DubiousTargetConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementDetectionConfig")
    {
        EvaluationRequirement::DetectionConfig* config =
                new EvaluationRequirement::DetectionConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementPositionDistanceConfig")
    {
        EvaluationRequirement::PositionDistanceConfig* config =
                new EvaluationRequirement::PositionDistanceConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementPositionDistanceRMSConfig")
    {
        EvaluationRequirement::PositionDistanceRMSConfig* config =
                new EvaluationRequirement::PositionDistanceRMSConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementPositionRadarRangeConfig")
    {
        EvaluationRequirement::PositionRadarRangeConfig* config =
                new EvaluationRequirement::PositionRadarRangeConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementPositionRadarAzimuthConfig")
    {
        EvaluationRequirement::PositionRadarAzimuthConfig* config =
                new EvaluationRequirement::PositionRadarAzimuthConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementPositionAlongConfig")
    {
        EvaluationRequirement::PositionAlongConfig* config =
                new EvaluationRequirement::PositionAlongConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementPositionAcrossConfig")
    {
        EvaluationRequirement::PositionAcrossConfig* config =
                new EvaluationRequirement::PositionAcrossConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementPositionLatencyConfig")
    {
        EvaluationRequirement::PositionLatencyConfig* config =
                new EvaluationRequirement::PositionLatencyConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementSpeedConfig")
    {
        EvaluationRequirement::SpeedConfig* config =
                new EvaluationRequirement::SpeedConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementTrackAngleConfig")
    {
        EvaluationRequirement::TrackAngleConfig* config =
                new EvaluationRequirement::TrackAngleConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementIdentificationCorrectConfig")
    {
        EvaluationRequirement::IdentificationCorrectConfig* config =
                new EvaluationRequirement::IdentificationCorrectConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementIdentificationFalseConfig")
    {
        EvaluationRequirement::IdentificationFalseConfig* config =
                new EvaluationRequirement::IdentificationFalseConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementIdentificationCorrectPeriodConfig")
    {
        EvaluationRequirement::IdentificationCorrectPeriodConfig* config =
                new EvaluationRequirement::IdentificationCorrectPeriodConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementModeAPresentConfig")
    {
        EvaluationRequirement::ModeAPresentConfig* config =
                new EvaluationRequirement::ModeAPresentConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementModeAFalseConfig")
    {
        EvaluationRequirement::ModeAFalseConfig* config =
                new EvaluationRequirement::ModeAFalseConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementModeCPresentConfig")
    {
        EvaluationRequirement::ModeCPresentConfig* config =
                new EvaluationRequirement::ModeCPresentConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementModeCCorrectConfig")
    {
        EvaluationRequirement::ModeCCorrectConfig* config =
                new EvaluationRequirement::ModeCCorrectConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementModeCFalseConfig")
    {
        EvaluationRequirement::ModeCFalseConfig* config =
                new EvaluationRequirement::ModeCFalseConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementModeCCorrectPeriodConfig")
    {
        EvaluationRequirement::ModeCCorrectPeriodConfig* config =
                new EvaluationRequirement::ModeCCorrectPeriodConfig(
                    class_id, instance_id, *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementMoMLongAccConfig")
    {
        EvaluationRequirement::GenericIntegerConfig* config = new EvaluationRequirement::GenericIntegerConfig(
                class_id, instance_id, "MomLongAccCorrect", *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementMoMTransAccConfig")
    {
        EvaluationRequirement::GenericIntegerConfig* config = new EvaluationRequirement::GenericIntegerConfig(
            class_id, instance_id, "MomTransAccCorrect", *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementMoMVertRateConfig")
    {
        EvaluationRequirement::GenericIntegerConfig* config = new EvaluationRequirement::GenericIntegerConfig(
            class_id, instance_id, "MomVertRateCorrect", *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementROCDCorrectConfig")
    {
        EvaluationRequirement::GenericDoubleConfig* config = new EvaluationRequirement::GenericDoubleConfig(
            class_id, instance_id, "ROCDCorrect", *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else if (class_id == "EvaluationRequirementAccelerationCorrectConfig")
    {
        EvaluationRequirement::GenericDoubleConfig* config = new EvaluationRequirement::GenericDoubleConfig(
            class_id, instance_id, "AccelerationCorrect", *this, standard_, eval_man_);
        logdbg << "EvaluationRequirementGroup: generateSubConfigurable: adding config " << config->name();

        assert(!hasRequirementConfig(config->name()));
        configs_.push_back(std::unique_ptr<EvaluationRequirement::BaseConfig>(config));
    }
    else
        throw std::runtime_error("EvaluationRequirementGroup: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

std::string Group::name() const
{
    return name_;
}


bool Group::hasRequirementConfig (const std::string& name)
{
    auto iter = std::find_if(configs_.begin(), configs_.end(),
       [&name](const unique_ptr<EvaluationRequirement::BaseConfig>& x) { return x->name() == name;});

    return iter != configs_.end();
}

void Group::addRequirementConfig (const std::string& class_id, const std::string& name, const std::string& short_name)
{
    loginf << "EvaluationRequirementGroup: addRequirementConfig: class_id " << class_id << " name " << name
           << " short_name " << short_name;

    assert (!hasRequirementConfig(name));

    standard_.beginModelReset();

    std::string instance = class_id + name + "0";

    auto config = Configuration::create(class_id, instance);
    config->addParameter<std::string>("name", name);
    config->addParameter<std::string>("short_name", short_name);

    generateSubConfigurableFromConfig(std::move(config));

    sortConfigs();

    standard_.endModelReset();

    assert (hasRequirementConfig(name));

    emit configsChangedSignal();
}

EvaluationRequirement::BaseConfig& Group::requirementConfig (const std::string& name)
{
    assert (hasRequirementConfig(name));

    auto iter = std::find_if(configs_.begin(), configs_.end(),
                             [&name](const unique_ptr<EvaluationRequirement::BaseConfig>& x) { return x->name() == name;});

    return **iter;
}

void Group::removeRequirementConfig (const std::string& name)
{
    assert (hasRequirementConfig(name));

    auto iter = std::find_if(configs_.begin(), configs_.end(),
                             [&name](const unique_ptr<EvaluationRequirement::BaseConfig>& x) { return x->name() == name;});

    assert (iter != configs_.end());

    standard_.beginModelReset();

    configs_.erase(iter);

    standard_.endModelReset();

    emit configsChangedSignal();
}

void Group::checkSubConfigurables()
{

}

EvaluationStandardTreeItem* Group::child(int row)
{
    if (row < 0 || row >= configs_.size())
        return nullptr;

    return configs_.at(row).get();
}

int Group::childCount() const
{
    return configs_.size();
}

int Group::columnCount() const
{
    return 1;
}

QVariant Group::data(int column) const
{
    assert (column == 0);

    return name_.c_str();
}

int Group::row() const
{
    return 0;
}

void Group::showMenu ()
{
    QMenu menu;

    // menu creation
    {
        QAction* del_action = menu.addAction("Delete Group");
        connect(del_action, &QAction::triggered, this, &Group::deleteGroupSlot);

        // requirements
        QMenu* req_menu = menu.addMenu("Add Requirement");

        for (auto& req_it : requirement_type_mapping_)
        {
            QAction* action = req_menu->addAction(req_it.second.c_str());
            action->setData(req_it.first.c_str());
            connect(action, &QAction::triggered, this, &Group::addRequirementSlot);
        }

//        { // extra data
//            QAction* add_det_action = req_menu->addAction("Extra Data");
//            add_det_action->setData("EvaluationRequirementExtraDataConfig");
//            connect(add_det_action, &QAction::triggered, this, &Group::addRequirementSlot);
//        }

//        { // extra track
//            QAction* add_det_action = req_menu->addAction("Extra Track");
//            add_det_action->setData("EvaluationRequirementExtraTrackConfig");
//            connect(add_det_action, &QAction::triggered, this, &Group::addRequirementSlot);
//        }

//        { // detection
//            QAction* add_det_action = req_menu->addAction("Detection");
//            add_det_action->setData("EvaluationRequirementDetectionConfig");
//            connect(add_det_action, &QAction::triggered, this, &Group::addRequirementSlot);
//        }

//        { // identification
//            QAction* correct_action = req_menu->addAction("Identification Correct");
//            correct_action->setData("EvaluationRequirementIdentificationCorrectConfig");
//            connect(correct_action, &QAction::triggered, this, &Group::addRequirementSlot);

//            QAction* false_action = req_menu->addAction("Identification False");
//            false_action->setData("EvaluationRequirementIdentificationFalseConfig");
//            connect(false_action, &QAction::triggered, this, &Group::addRequirementSlot);
//        }

//        { // mode 3/a
//            QAction* present_action = req_menu->addAction("Mode 3/A Present");
//            present_action->setData("EvaluationRequirementModeAPresentConfig");
//            connect(present_action, &QAction::triggered, this, &Group::addRequirementSlot);

//            QAction* false_action = req_menu->addAction("Mode 3/A False");
//            false_action->setData("EvaluationRequirementModeAFalseConfig");
//            connect(false_action, &QAction::triggered, this, &Group::addRequirementSlot);
//        }

//        { // mode c
//            QAction* present_action = req_menu->addAction("Mode C Present");
//            present_action->setData("EvaluationRequirementModeCPresentConfig");
//            connect(present_action, &QAction::triggered, this, &Group::addRequirementSlot);

//            QAction* false_action = req_menu->addAction("Mode C False");
//            false_action->setData("EvaluationRequirementModeCFalseConfig");
//            connect(false_action, &QAction::triggered, this, &Group::addRequirementSlot);
//        }

//        { // position
//            QAction* md_action = req_menu->addAction("Position Distance");
//            md_action->setData("EvaluationRequirementPositionDistanceConfig");
//            connect(md_action, &QAction::triggered, this, &Group::addRequirementSlot);

//            QAction* along_action = req_menu->addAction("Position Along");
//            along_action->setData("EvaluationRequirementPositionAlongConfig");
//            connect(along_action, &QAction::triggered, this, &Group::addRequirementSlot);

//            QAction* across_action = req_menu->addAction("Position Across");
//            across_action->setData("EvaluationRequirementPositionAcrossConfig");
//            connect(across_action, &QAction::triggered, this, &Group::addRequirementSlot);

//            QAction* latency_action = req_menu->addAction("Position Latency");
//            latency_action->setData("EvaluationRequirementPositionLatencyConfig");
//            connect(latency_action, &QAction::triggered, this, &Group::addRequirementSlot);

//        }

//        { // spd
//            QAction* md_action = req_menu->addAction("Speed");
//            md_action->setData("EvaluationRequirementSpeedConfig");
//            connect(md_action, &QAction::triggered, this, &Group::addRequirementSlot);
//        }

        {
            QMenu* del_menu = menu.addMenu("Delete Requirement");

            for (auto& cfg_it : configs_)
            {
                QAction* action = del_menu->addAction(cfg_it->name().c_str());
                action->setData(cfg_it->name().c_str());
                connect(action, &QAction::triggered, this, &Group::deleteRequirementSlot);
            }
        }

    }

    menu.exec(QCursor::pos());
}

void Group::deleteGroupSlot()
{
    loginf << "EvaluationRequirementGroup " << name_ << ": deleteGroupSlot";

    standard_.removeGroup (name_);
}

void Group::addRequirementSlot()
{
    loginf << "EvaluationRequirementGroup " << name_ << ": addRequirementSlot";

    QAction* action = dynamic_cast<QAction*>(QObject::sender());
    assert (action);

    QVariant data = action->data();
    assert (data.isValid());

    string class_id = data.toString().toStdString();

    loginf << "EvaluationRequirementGroup " << name_ << ": addRequirementSlot: class_id " << class_id;

    bool ok;
    QString text =
            QInputDialog::getText(nullptr, tr("Requirement Name"),
                                  tr("Specify a (unique) requirement name:"), QLineEdit::Normal,
                                  "", &ok);

    if (!ok)
        return;

    std::string req_name;

    if (!text.isEmpty())
    {
        req_name = text.toStdString();
        if (!req_name.size())
        {
            QMessageBox m_warning(QMessageBox::Warning, "Adding Requirement Failed",
                                  "Requirement has to have a non-empty name.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        if (hasRequirementConfig(req_name))
        {
            QMessageBox m_warning(QMessageBox::Warning, "Adding Requirement Failed",
                                  "Requirement with this name already exists.", QMessageBox::Ok);
            m_warning.exec();
            return;
        }
    }

    std::string req_short_name;

    text =  QInputDialog::getText(nullptr, tr("Requirement Short Name"),
                                  tr("Specify a requirement short name:"), QLineEdit::Normal,
                                  "", &ok);

    if (!ok)
        return;

    if (!text.isEmpty())
        req_short_name = text.toStdString();

    loginf << "EvaluationRequirementGroup " << name_ << ": addRequirementSlot: class_id " << class_id
           << " req_name '" << req_name << "' req_short_name '" << req_short_name << "'";

    if (req_name.size() && req_short_name.size())
        addRequirementConfig(class_id, req_name, req_short_name);
    else
    {
        QMessageBox m_warning(QMessageBox::Warning, "Adding Requirement Failed",
                              "Requirement has to have a non-empty name and short name.", QMessageBox::Ok);
        m_warning.exec();
        return;
    }
}

void Group::deleteRequirementSlot()
{
    loginf << "EvaluationRequirementGroup " << name_ << ": deleteRequirementSlot";

    QAction* action = dynamic_cast<QAction*>(QObject::sender());
    assert (action);

    QVariant data = action->data();
    assert (data.isValid());

    string name = data.toString().toStdString();

    QMessageBox::StandardButton reply;
      reply = QMessageBox::question(nullptr, "Delete Requirement", ("Confirm to delete requirement '"+name+"'").c_str(),
                                    QMessageBox::Yes|QMessageBox::No);
      if (reply == QMessageBox::Yes)
      {
        removeRequirementConfig(name);
      }
}


void Group::sortConfigs()
{
    sort(configs_.begin(), configs_.end(),
         [](const unique_ptr<EvaluationRequirement::BaseConfig>&a, const unique_ptr<EvaluationRequirement::BaseConfig>& b) -> bool
    {
        return a->name() > b->name();
    });
}
