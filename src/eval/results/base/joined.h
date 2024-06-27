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

#pragma once

#include "eval/results/base/base.h"

#include "view/gridview/grid2d_defs.h"

#include <fstream>

class Grid2D;

namespace EvaluationRequirementResult
{

class Single;

/**
*/
class Joined : public Base
{
public:
    typedef Info SectorInfo;

    typedef std::function<void(const std::shared_ptr<Single>&)> SingleResultFunc;

    Joined(const std::string& type, 
           const std::string& result_id,
           std::shared_ptr<EvaluationRequirement::Base> requirement,
           const SectorLayer& sector_layer,
           EvaluationManager& eval_man);
    virtual ~Joined();

    virtual BaseType baseType() const override { return BaseType::Joined; }

    void clearResults();
    void addSingleResult(std::shared_ptr<Single> other);

    void addToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override final;

    bool hasViewableData (const EvaluationResultsReport::SectionContentTable& table, 
                          const QVariant& annotation) const override final;
    std::unique_ptr<nlohmann::json::object_t> viewableData(const EvaluationResultsReport::SectionContentTable& table, 
                                                           const QVariant& annotation) const override final;
    std::unique_ptr<nlohmann::json::object_t> viewableData() const override final;

    bool hasReference (const EvaluationResultsReport::SectionContentTable& table, 
                       const QVariant& annotation) const override final;
    std::string reference(const EvaluationResultsReport::SectionContentTable& table, 
                          const QVariant& annotation) const override final;

    std::vector<std::shared_ptr<Single>>& singleResults();

    void updateToChanges();

    unsigned int numSingleResults() const;
    unsigned int numUsableSingleResults() const;
    unsigned int numUnusableSingleResults() const;

    static const std::string SectorOverviewID;
    static const int         SectorOverviewRenderDelayMSec;

protected:
    enum class OverviewMode
    {
        Annotations = 0,
        Grid,
        GridPlusAnnotations,
        GridOrAnnotations
    };

    //virtual void join_impl(std::shared_ptr<Single> other) = 0;
    virtual void clearResults_impl() = 0;
    virtual void accumulateSingleResult(const std::shared_ptr<Single>& single_result, 
                                        bool first,
                                        bool last) = 0;
    virtual unsigned int numUpdates() const = 0;

    bool exportAsCSV() const;
    virtual bool canExportCSV() const { return false; }
    virtual bool exportAsCSV(std::ofstream& strm) const { return false; }

    std::unique_ptr<nlohmann::json::object_t> createBaseViewable() const override final;
    ViewableInfo createViewableInfo(const AnnotationOptions& options) const override final;
    void createAnnotations(nlohmann::json& annotations, 
                           const AnnotationOptions& options) const override final;

    virtual OverviewMode overviewMode() const { return OverviewMode::GridOrAnnotations; }

    virtual std::vector<SectorInfo> sectorInfosCommon() const;
    std::vector<SectorInfo> sectorConditionInfos() const;

    virtual std::vector<SectorInfo> sectorInfos() const = 0;

    bool resultUsed(const std::shared_ptr<Single>& result) const;
    void iterateSingleResults(const SingleResultFunc& func,
                              const SingleResultFunc& func_used,
                              const SingleResultFunc& func_unused) const;

    std::vector<std::shared_ptr<Single>> results_;

private:
    void addOverview (EvaluationResultsReport::Section& section,
                      const std::string& name = "Sector Overview");

    void addSectorToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addSectorDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

    void createGrid(const grid2d::GridResolution& resolution) const;
    void addOverviewGrid(nlohmann::json& annotations_json) const;
    void addOverviewAnnotations(nlohmann::json& annotations_json) const;

    unsigned int num_targets_        = 0;
    unsigned int num_failed_targets_ = 0;

    mutable std::unique_ptr<Grid2D> grid_;
};

}
