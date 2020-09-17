#ifndef EVALUATIONDATA_H
#define EVALUATIONDATA_H

#include <memory>

class EvaluationManager;
class DBObject;
class Buffer;

class EvaluationData
{
public:
    EvaluationData(EvaluationManager& eval_man);

    void addReferenceData (DBObject& dbo_ref, std::shared_ptr<Buffer> buffer);
    void addTestData (DBObject& dbo_ref, std::shared_ptr<Buffer> buffer);
    void clear();

protected:
    EvaluationManager& eval_man_;
};

#endif // EVALUATIONDATA_H
