#ifndef JOINEVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H
#define JOINEVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H

#include "eval/results/joined.h"

namespace EvaluationRequirementResult
{
    class SinglePositionMaxDistance;

    class JoinedPositionMaxDistance : public Joined
    {
    public:
        JoinedPositionMaxDistance(
                const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
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
        int num_pos_ {0};
        int num_no_ref_ {0};
        int num_pos_outside_ {0};
        int num_pos_inside_ {0};
        int num_pos_ok_ {0};
        int num_pos_nok_ {0};

        bool first_ {true};
        double error_min_ {0};
        double error_max_ {0};
        double error_avg_ {0};

        bool has_p_min_pos_ {false};
        float p_min_pos_{0};

        void addToValues (std::shared_ptr<SinglePositionMaxDistance> single_result);
        void updatePMinPos();
    };

}

#endif // JOINEVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H
