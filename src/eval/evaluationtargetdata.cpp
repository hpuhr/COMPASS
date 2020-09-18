#include "evaluationtargetdata.h"
#include "logger.h"

#include <cassert>

EvaluationTargetData::EvaluationTargetData(unsigned int utn)
    : utn_(utn)
{

}

bool EvaluationTargetData::hasRefBuffer ()
{
    return ref_buffer != nullptr;
}

void EvaluationTargetData::setRefBuffer (std::shared_ptr<Buffer> buffer)
{
    assert (!ref_buffer);
    ref_buffer = buffer;
}

void EvaluationTargetData::addRefRecNum (float tod, unsigned int rec_num)
{
    ref_rec_nums_.insert({tod, rec_num});
}


bool EvaluationTargetData::hasTstBuffer ()
{
    return tst_buffer != nullptr;
}

void EvaluationTargetData::setTstBuffer (std::shared_ptr<Buffer> buffer)
{
    assert (!tst_buffer);
    tst_buffer = buffer;
}

void EvaluationTargetData::addTstRecNum (float tod, unsigned int rec_num)
{
    tst_rec_nums_.insert({tod, rec_num});
}

bool EvaluationTargetData::hasRefData ()
{
    return ref_rec_nums_.size();
}

bool EvaluationTargetData::hasTstData ()
{
    return tst_rec_nums_.size();
}

void EvaluationTargetData::finalize ()
{
    loginf << "EvaluationTargetData: finalize: utn " << utn_
           << " ref " << hasRefData() << " up " << ref_rec_nums_.size()
           << " tst " << hasTstData() << " up " << tst_rec_nums_.size();
}
