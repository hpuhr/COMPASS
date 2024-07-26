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

#include "evaluationdetail.h"
#include "eval/results/evaluationdetail.h"
#include "eval/results/base/result_defs.h"

#include <QVariant>
#include <QRectF>

#include "json.hpp"

#include <memory>
#include <vector>

#include <QColor>

#include <Eigen/Core>

class EvaluationTargetData;
class EvaluationManager;
class SectorLayer;

namespace EvaluationRequirement 
{
    class Base;
}

namespace EvaluationResultsReport 
{
    class Section;
    class RootItem;
    class SectionContentTable;
}

namespace EvaluationRequirementResult
{

class FeatureDefinitions;

template <typename T>
struct ValueSource;

/**
*/
class Base
{
public:
    typedef std::vector<EvaluationDetail> EvaluationDetails;  // details vector
    typedef std::array<int, 2>            DetailIndex;        // index for a nested detail struct

    typedef std::function<bool(const EvaluationDetail&)> DetailSkipFunc;
    typedef std::function<void(const EvaluationDetail&, const EvaluationDetail*, int, int, int, int)> DetailFunc;

    enum class BaseType
    {
        Single = 0, // target results (for specific utn)
        Joined      // sum results (for specific requirement in specific sector)
    };

    /**
     * Info table entry struct.
     */
    struct Info
    {
        Info() {}
        Info(const QString& name,
             const QString& comment,
             const QVariant& value)
        :   info_name   (name   )
        ,   info_comment(comment)
        ,   info_value  (value  )
        {}

        QString  info_name;
        QString  info_comment;
        QVariant info_value;
    };

    Base(const std::string& type, 
         const std::string& result_id,
         std::shared_ptr<EvaluationRequirement::Base> requirement, 
         const SectorLayer& sector_layer,
         EvaluationManager& eval_man);
    virtual ~Base();

    /// returns the base type of the result (either single or joined)
    virtual BaseType baseType() const = 0;

    std::string type() const;
    std::string resultId() const;
    std::string reqGrpId() const;

    std::shared_ptr<EvaluationRequirement::Base> requirement() const;

    bool isSingle() const;
    bool isJoined() const;

    bool use() const;
    void use(bool use);

    const boost::optional<double>& result() const;

    bool resultUsable() const;
    bool hasFailed() const;
    bool hasIssues() const;
    bool isIgnored() const;

    const SectorLayer& sectorLayer() const { return sector_layer_; } 

    QVariant resultValue() const;
    QVariant resultValueOptional(const boost::optional<double>& value) const;
    virtual QVariant resultValue(double value) const;

    /// returns the number of issues detected for this result
    virtual unsigned int numIssues() const = 0;

    /// checks if the result references a specific section of the report
    virtual bool hasReference(const EvaluationResultsReport::SectionContentTable& table, 
                              const QVariant& annotation) const = 0;
    /// returns a report reference link
    virtual std::string reference(const EvaluationResultsReport::SectionContentTable& table, 
                                  const QVariant& annotation) const = 0;

    /// checks if the result can generate viewable data for the given table and annotation index
    virtual bool hasViewableData (const EvaluationResultsReport::SectionContentTable& table, 
                                  const QVariant& annotation) const = 0;
    /// creates suitable viewable data for the given table and annotation index
    virtual std::shared_ptr<nlohmann::json::object_t> viewableData(const EvaluationResultsReport::SectionContentTable& table, 
                                                                   const QVariant& annotation) const = 0;
    /// adds the result to the report root item
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) = 0;

    /// iterate over details
    virtual void iterateDetails(const DetailFunc& func,
                                const DetailSkipFunc& skip_func = DetailSkipFunc(),
                                const EvaluationDetails* details = nullptr) const = 0;

    std::vector<double> getValues(const ValueSource<double>& source) const;
    std::vector<double> getValues(int value_id) const;

    size_t totalNumDetails(const EvaluationDetails* details = nullptr) const;
    size_t totalNumPositions(const EvaluationDetails* details = nullptr) const;

    const static std::string req_overview_table_name_;

    static const QColor HistogramColorDefault;

protected:
    /**
     * Used to display a certain result parameter in the report as
     * Name | Description | Value, e.g.
     * PD [%]    Probability of Detection    98.4
     */
    struct ReportParam
    {
        ReportParam() = default;
        ReportParam(const std::string& _name, 
                    const std::string& _descr, 
                    const QVariant& _value) : name(_name), descr(_descr), value(_value) {}

        std::string name;
        std::string descr;
        QVariant    value;
    };

    enum class ViewableType
    {
        Overview = 0,  // viewable represents an overview over the result data
        Highlight      // viewable represents a highlighted result detail
    };

    /**
     * Options for creating result annotations.
    */
    struct AnnotationOptions
    {
        /// configure as result overview
        AnnotationOptions& overview() 
        { 
            viewable_type = ViewableType::Overview; 
            return *this; 
        }
        /// configure as highlighted result detail
        AnnotationOptions& highlight(const DetailIndex& index)
        { 
            viewable_type = ViewableType::Highlight; 
            detail_index  = index;
            return *this; 
        }

        bool valid() const
        {
            if (viewable_type == ViewableType::Highlight)
                return detail_index.has_value();
            return true;
        }

        ViewableType                 viewable_type = ViewableType::Overview;
        boost::optional<DetailIndex> detail_index;
    };

    /**
     * Information about a viewable.
    */
    struct ViewableInfo
    {
        ViewableType             viewable_type; // viewable type
        QRectF                   bounds;        // viewable bounds (e.g. for viewpoint region of interest)
        boost::posix_time::ptime timestamp;     // viewable timestamp (e.g. for single occurrences)
    };

    void setIgnored();

    QString formatValue(double v, int precision = 2) const;

    /// compute result value
    virtual void updateResult();
    virtual boost::optional<double> computeResult() const;
    virtual boost::optional<double> computeResult_impl() const = 0;
    
    std::string conditionResultString() const;

    /// creates overview viewable data
    virtual std::unique_ptr<nlohmann::json::object_t> viewableOverviewData() const = 0;

    std::unique_ptr<nlohmann::json::object_t> createViewable(const AnnotationOptions& options,
                                                             const EvaluationDetails* details = nullptr) const;

    void addCustomAnnotations(nlohmann::json& annotations_json,
                              const EvaluationDetails* details = nullptr) const;

    /// creates a basic viewable
    virtual std::unique_ptr<nlohmann::json::object_t> createBaseViewable() const = 0;
    /// creates additional viewable information (region of interest etc.)
    virtual ViewableInfo createViewableInfo(const AnnotationOptions& options,
                                            const EvaluationDetails* details = nullptr) const = 0;
    /// creates annotations for the given options
    virtual void createAnnotations(nlohmann::json& annotations_json, 
                                   const AnnotationOptions& options,
                                   const EvaluationDetails* details = nullptr) const = 0;

    /// generate definitions for the automatic generation of custom annotations (grids, histograms, etc.)
    virtual FeatureDefinitions getCustomAnnotationDefinitions() const;

    EvaluationResultsReport::SectionContentTable& getReqOverviewTable (
            std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

    /// section and annotation id strings
    virtual std::string getRequirementSectionID() const;
    virtual std::string getRequirementSumSectionID() const;
    virtual std::string getRequirementAnnotationID_impl() const = 0;

    std::string getRequirementAnnotationID() const;
    
    EvaluationResultsReport::Section& getRequirementSection(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

    std::string type_;
    std::string result_id_;
    std::string req_grp_id_;

    bool use_ {true};

    std::shared_ptr<EvaluationRequirement::Base> requirement_;
    const SectorLayer& sector_layer_;

    EvaluationManager& eval_man_;

private:
    boost::optional<double> result_;
    bool                    ignore_ = false;
};

}
