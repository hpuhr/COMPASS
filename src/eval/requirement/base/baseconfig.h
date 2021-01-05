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

#ifndef EVALUATIONREQUIREMENTBASECONFIG_H
#define EVALUATIONREQUIREMENTBASECONFIG_H

#include <QObject>

#include "configurable.h"
#include "evaluationstandardtreeitem.h"

class Group;
class EvaluationStandard;


class QWidget;
class QFormLayout;
class EvaluationManager;

namespace EvaluationRequirement
{
    class Base;
    class BaseConfigWidget;

    class BaseConfig : public QObject, public Configurable, public EvaluationStandardTreeItem
    {
        Q_OBJECT

    public:
        BaseConfig(const std::string& class_id, const std::string& instance_id,
                                    Group& group, EvaluationStandard& standard,
                                    EvaluationManager& eval_man);
        virtual ~BaseConfig();

        virtual void generateSubConfigurable(const std::string& class_id,
                                             const std::string& instance_id) override;

        std::string name() const;
        void name(const std::string& name);

        bool hasShortName () const;
        std::string shortName() const;
        void shortName(const std::string& short_name);

        virtual EvaluationStandardTreeItem *child(int row) override;
        virtual int childCount() const override;
        virtual int columnCount() const override;
        virtual QVariant data(int column) const override;
        virtual int row() const override;

        BaseConfigWidget* widget();
        virtual std::shared_ptr<Base> createRequirement() = 0;

    protected:
        Group& group_;
        EvaluationStandard& standard_;
        EvaluationManager& eval_man_;

        std::string name_;
        std::string short_name_;

        std::unique_ptr<BaseConfigWidget> widget_ {nullptr};

        virtual void checkSubConfigurables() override;
        virtual void createWidget(); // creates BaseConfigWidget, override to change
    };

}

#endif // EVALUATIONREQUIREMENTBASECONFIG_H
