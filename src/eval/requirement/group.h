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

#ifndef EVALUATIONREQUIREMENTGROUP_H
#define EVALUATIONREQUIREMENTGROUP_H

#include "configurable.h"
#include "evaluationstandardtreeitem.h"

#include <QObject>
#include <QMenu>

#include <memory>

class EvaluationStandard;
class EvaluationManager;

namespace EvaluationRequirement {
class Config;
}

class Group : public QObject, public Configurable, public EvaluationStandardTreeItem
{
    Q_OBJECT

signals:
    void configsChangedSignal();

public slots:
    void deleteGroupSlot();
    void addRequirementSlot();


public:
    Group(const std::string& class_id, const std::string& instance_id,
                               EvaluationStandard& standard, EvaluationManager& eval_man);
    virtual ~Group();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    std::string name() const;

    bool hasRequirementConfig (const std::string& name);
    void addRequirementConfig (const std::string& class_id, const std::string& name, const std::string& short_name);
    EvaluationRequirement::Config& requirementConfig (const std::string& name);
    void removeRequirementConfig (const std::string& name);

    using EvaluationRequirementConfigIterator =
    typename std::vector<std::unique_ptr<EvaluationRequirement::Config>>::iterator;

    EvaluationRequirementConfigIterator begin() { return configs_.begin(); }
    EvaluationRequirementConfigIterator end() { return configs_.end(); }
    unsigned int size () { return configs_.size(); };

    virtual EvaluationStandardTreeItem *child(int row) override;
    virtual int childCount() const override;
    virtual int columnCount() const override;
    virtual QVariant data(int column) const override;
    virtual int row() const override;

    void showMenu ();

protected:
    EvaluationStandard& standard_;
    EvaluationManager& eval_man_;
    std::string name_;

    std::vector<std::unique_ptr<EvaluationRequirement::Config>> configs_;

    virtual void checkSubConfigurables() override;

    void sortConfigs();
};

#endif // EVALUATIONREQUIREMENTGROUP_H
