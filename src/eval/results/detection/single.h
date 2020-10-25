#ifndef EVALUATIONREQUIREMENTDETECTIONRESULT_H
#define EVALUATIONREQUIREMENTDETECTIONRESULT_H

#include "eval/results/single.h"
#include "eval/requirement/detection/detection.h"
#include "timeperiod.h"

namespace EvaluationRequirementResult
{

class SingleDetection : public Single
{
public:
    SingleDetection(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer, unsigned int utn, const EvaluationTargetData* target,
            EvaluationManager& eval_man,
            int sum_uis, int missed_uis, TimePeriodCollection ref_periods,
            std::vector<EvaluationRequirement::DetectionDetail> details);

    virtual void print() override;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    int sumUIs() const;
    int missedUIs() const;

    std::vector<EvaluationRequirement::DetectionDetail>& details();

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

protected:
    int sum_uis_ {0};
    int missed_uis_ {0};

    TimePeriodCollection ref_periods_;

    std::vector<EvaluationRequirement::DetectionDetail> details_;

    bool has_pd_ {false};
    float pd_{0};

    void updatePD();
    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);
};

}

#endif // EVALUATIONREQUIREMENTDETECTIONRESULT_H
