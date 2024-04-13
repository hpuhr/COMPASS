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

#ifndef JOINEDEVALUATIONREQUIREMENTRESULT_H
#define JOINEDEVALUATIONREQUIREMENTRESULT_H

#include "eval/results/base.h"
#include "view/gridview/grid2d.h"
#include "view/gridview/grid2dlayer.h"

namespace EvaluationRequirementResult
{

class Single;

class Joined : public Base
{
public:
    Joined(const std::string& type, 
           const std::string& result_id,
           std::shared_ptr<EvaluationRequirement::Base> requirement,
           const SectorLayer& sector_layer,
           EvaluationManager& eval_man);
    virtual ~Joined();

    virtual BaseType baseType() const override { return BaseType::Joined; }

    void add(std::shared_ptr<Single> other);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override = 0 ;

    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override final;

    std::vector<std::shared_ptr<Single>>& results();

    void updateToChanges();

    unsigned int numResults();
    unsigned int numUsableResults();
    unsigned int numUnusableResults();

protected:
    enum class OverviewMode
    {
        Features = 0,
        Grid,
        GridPlusFeatures,
        GridOrFeatures
    };

    std::vector<std::shared_ptr<Single>> results_;

    std::unique_ptr<Grid2D> grid_;

    void addCommonDetails (EvaluationResultsReport::SectionContentTable& sector_details_table);

    //virtual void join_impl(std::shared_ptr<Single> other) = 0;
    virtual void updateToChanges_impl() = 0;

    std::unique_ptr<nlohmann::json::object_t> createViewable() const;

    virtual OverviewMode overviewMode() const { return OverviewMode::GridOrFeatures; }

    virtual std::unique_ptr<nlohmann::json::object_t> viewableDataImpl(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) { return {}; }

    void addAnnotationsFromSingles(nlohmann::json::object_t& viewable_ref);

    void createGrid(size_t num_cells_x, size_t num_cells_y);
    void createGrid(double cell_size_x, double cell_size_y);
    bool addToGrid(double lon, double lat, double value);
    void addGridToViewData(nlohmann::json::object_t& view_data);
};

}
#endif // JOINEDEVALUATIONREQUIREMENTRESULT_H
