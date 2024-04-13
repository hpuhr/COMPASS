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

#include "eval/results/single.h"
#include "eval/results/joined.h"
#include "timeperiod.h"

#include <boost/optional.hpp>

#include <QVariant>

namespace EvaluationRequirementResult
{

/**
*/
class SingleIntervalBase : public Single
{
public:
    SingleIntervalBase(const std::string& result_type, 
                       const std::string& result_id, 
                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                       const SectorLayer& sector_layer, 
                       unsigned int utn, 
                       const EvaluationTargetData* target,
                       EvaluationManager& eval_man,
                       const EvaluationDetails& details,
                       int sum_uis, 
                       int missed_uis, 
                       TimePeriodCollection ref_periods,
                       const std::vector<dbContent::TargetPosition>& ref_updates);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    int sumUIs() const;
    int missedUIs() const;

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    enum DetailKey
    {
        MissOccurred,        //bool
        DiffTOD,             //float
        RefExists,           //bool
        MissedUIs,           //unsigned int
        RefUpdateStartIndex, //unsigned int
        RefUpdateEndIndex    //unsigned int
    };

    void addAnnotations(nlohmann::json::object_t& viewable, bool overview, bool add_ok) override;

    virtual std::map<std::string, std::vector<LayerDefinition>> gridLayers() const override;
    virtual void addValuesToGrid(Grid2D& grid, const std::string& layer) const override;

    bool hasFailed() const;

protected:
    virtual std::vector<std::string> targetTableColumns() const;
    virtual std::vector<QVariant> targetTableValues() const;
    virtual std::vector<ReportParam> detailsOverviewDescriptions() const;
    virtual std::vector<std::string> detailsTableColumns() const;
    virtual std::vector<QVariant> detailsTableValues(const EvaluationDetail& detail) const;

    virtual std::string probabilityName() const = 0;
    virtual std::string probabilityDescription() const = 0;

    virtual unsigned int sortColumn() const;

    void updateProbability();

    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::Section& section, const std::string& table_name);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    void addAnnotations(nlohmann::json::object_t& viewable, const EvaluationDetail& detail);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable (const EvaluationDetail* detail = nullptr);

    QVariant probabilityVar() const;

    int sum_uis_    {0};
    int missed_uis_ {0};

    TimePeriodCollection                   ref_periods_;
    std::vector<dbContent::TargetPosition> ref_updates_;

    boost::optional<float> probability_;
};

/**
*/
class JoinedIntervalBase : public Joined
{
public:
    JoinedIntervalBase(const std::string& result_type, 
                       const std::string& result_id, 
                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                       const SectorLayer& sector_layer, 
                       EvaluationManager& eval_man);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

protected:
    void addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

    std::unique_ptr<nlohmann::json::object_t> getErrorsViewable ();

    virtual void updateToChanges_impl() override;

    virtual std::unique_ptr<nlohmann::json::object_t> viewableDataImpl(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual std::vector<ReportParam> detailsOverviewDescriptions() const;

    virtual std::string probabilityName() const = 0;
    virtual std::string probabilityDescription() const = 0;

    unsigned int sum_uis_    {0};
    unsigned int missed_uis_ {0};

    unsigned int num_single_targets_ {0};
    unsigned int num_failed_single_targets_ {0};

    boost::optional<float> probability_;
};

} // EvaluationRequirementResult
