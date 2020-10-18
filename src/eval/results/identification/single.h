#ifndef EVALUATIONREQUIREMENIDENTIFICATIONRESULT_H
#define EVALUATIONREQUIREMENIDENTIFICATIONRESULT_H

#include "eval/results/single.h"
#include "eval/requirement/identification/identification.h"

namespace EvaluationRequirementResult
{

class SingleIdentification : public Single
{
public:
    SingleIdentification(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            int num_updates, int num_no_ref_pos, int num_no_ref_id, int num_pos_outside, int num_pos_inside,
            int num_unknown_id, int num_correct_id, int num_false_id,
            std::vector<EvaluationRequirement::IdentificationDetail> details);

    virtual void print() override;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    int numUpdates() const;
    int numNoRefPos() const;
    int numNoRefId() const;
    int numPosOutside() const;
    int numPosInside() const;
    int numUnknownId() const;
    int numCorrectId() const;
    int numFalseId() const;

    std::vector<EvaluationRequirement::IdentificationDetail>& details();

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;


protected:
    int num_updates_ {0};
    int num_no_ref_pos_ {0};
    int num_no_ref_id_ {0};
    int num_pos_outside_ {0};
    int num_pos_inside_ {0};
    int num_unknown_id_ {0};
    int num_correct_id_ {0};
    int num_false_id_ {0};

    bool has_pid_ {false};
    float pid_{0};

    std::vector<EvaluationRequirement::IdentificationDetail> details_;

    void updatePID();

};

}

#endif // EVALUATIONREQUIREMENIDENTIFICATIONRESULT_H
