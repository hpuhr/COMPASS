#ifndef EVALUATIONDATA_H
#define EVALUATIONDATA_H

#include <memory>
#include <map>

#include "evaluationtargetdata.h"

class EvaluationManager;
class DBObject;
class Buffer;

class EvaluationData
{
public:
    EvaluationData(EvaluationManager& eval_man);

    void addReferenceData (DBObject& object, std::shared_ptr<Buffer> buffer);
    void addTestData (DBObject& object, std::shared_ptr<Buffer> buffer);
    void finalize ();

    void clear();

protected:
    EvaluationManager& eval_man_;

    std::map<unsigned int, EvaluationTargetData> target_data_;

    unsigned int unassociated_ref_cnt_ {0};
    unsigned int associated_ref_cnt_ {0};

    unsigned int unassociated_tst_cnt_ {0};
    unsigned int associated_tst_cnt_ {0};
};

#endif // EVALUATIONDATA_H
