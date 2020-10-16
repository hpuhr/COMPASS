#ifndef JOINEVALUATIONREQUIREMENIDENTIFICATIONRESULT_H
#define JOINEVALUATIONREQUIREMENIDENTIFICATIONRESULT_H

#include "eval/results/joined.h"

namespace EvaluationRequirementResult
{
    class SingleIdentification;

    class JoinedIdentification : public Joined
    {
    public:
        JoinedIdentification(const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
                             const SectorLayer& sector_layer, EvaluationManager& eval_man);

        virtual void join(std::shared_ptr<Base> other) override;

        virtual void print() override;
        virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

        virtual void updatesToUseChanges() override;

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
        int num_no_ref_ {0};
        int num_unknown_id_ {0};
        int num_correct_id_ {0};
        int num_false_id_ {0};

        bool has_pid_ {false};
        float pid_{0};

        void addToValues (std::shared_ptr<SingleIdentification> single_result);
        void updatePID();
    };

}

#endif // JOINEVALUATIONREQUIREMENIDENTIFICATIONRESULT_H
