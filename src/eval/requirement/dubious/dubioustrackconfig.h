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

#ifndef DUBIOUSTRACKCONFIG_H
#define DUBIOUSTRACKCONFIG_H

#include "configurable.h"
#include "eval/requirement/base/baseconfig.h"
#include "eval/requirement/dubious/dubioustrackconfigwidget.h"
#include "eval/requirement/dubious/dubioustrack.h"

class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{

class DubiousTrackConfig : public BaseConfig
{
public:
    DubiousTrackConfig(const std::string& class_id, const std::string& instance_id,
                       Group& group, EvaluationStandard& standard, EvaluationManager& eval_man);
    virtual ~DubiousTrackConfig();

    std::shared_ptr<Base> createRequirement() override;

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    bool markPrimaryOnly() const;
    void markPrimaryOnly(bool mark_primary_only);

    bool useMinUpdates() const;
    void useMinUpdates(bool use_min_updates);

    unsigned int minUpdates() const;
    void minUpdates(unsigned int min_updates);

    bool useMinDuration() const;
    void useMinDuration(bool use_min_duration);

    float minDuration() const;
    void minDuration(float min_duration);

protected:

    bool mark_primary_only_ {true};

    bool use_min_updates_ {true};
    unsigned int min_updates_ {10};

    bool use_min_duration_ {true};
    float min_duration_ {30.0};

    virtual void createWidget() override;
};

}

#endif // DUBIOUSTRACKCONFIG_H
