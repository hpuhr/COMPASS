#ifndef EVALUATIONREQUIREMENTDETECTIONRESULT_H
#define EVALUATIONREQUIREMENTDETECTIONRESULT_H

#include "eval/results/single.h"
#include "eval/requirement/detection/detection.h"

namespace EvaluationRequirementResult
{

class SingleDetection : public Single
{
public:
    SingleDetection(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            float sum_uis, float missed_uis, float max_gap_uis, float no_ref_uis,
            std::vector<EvaluationRequirement::DetectionDetail> details);

    virtual void print() override;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    float sumUIs() const;
    float missedUIs() const;
    float maxGapUIs() const;
    float noRefUIs() const;

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
    float sum_uis_ {0};
    float missed_uis_ {0};
    float max_gap_uis_ {0};
    float no_ref_uis_ {0};

    std::vector<EvaluationRequirement::DetectionDetail> details_;

    bool has_pd_ {false};
    float pd_{0};

    void updatePD();
};

}

#endif // EVALUATIONREQUIREMENTDETECTIONRESULT_H
