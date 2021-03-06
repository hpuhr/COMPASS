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

#ifndef EVALUATIONRESULTSREPORTROOTITEM_H
#define EVALUATIONRESULTSREPORTROOTITEM_H

#include "eval/results/report/treeitem.h"

#include <memory>

class EvaluationManager;

namespace EvaluationResultsReport
{

    class Section;

    class RootItem : public TreeItem
    {
    public:
        RootItem(EvaluationManager& eval_man);
        virtual ~RootItem();

        virtual TreeItem *child(int row) override;
        virtual int childCount() const override;
        virtual int columnCount() const override;
        virtual QVariant data(int column) const override;
        virtual int row() const override;

        std::shared_ptr<Section> rootSection();

        Section& getSection (const std::string& id); // bla:bla2

    protected:
        EvaluationManager& eval_man_;

        std::shared_ptr<Section> root_section_;
    };

}

#endif // EVALUATIONRESULTSREPORTROOTITEM_H
