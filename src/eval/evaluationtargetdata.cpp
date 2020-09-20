#include "evaluationtargetdata.h"
#include "buffer.h"
#include "logger.h"
#include "stringconv.h"

#include <cassert>

using namespace std;
using namespace Utils;

EvaluationTargetData::EvaluationTargetData(unsigned int utn)
    : utn_(utn)
{
}

bool EvaluationTargetData::hasRefBuffer () const
{
    return ref_buffer != nullptr;
}

void EvaluationTargetData::setRefBuffer (std::shared_ptr<Buffer> buffer)
{
    assert (!ref_buffer);
    ref_buffer = buffer;
}

void EvaluationTargetData::addRefIndex (float tod, unsigned int index)
{
    ref_data_.insert({tod, index});
}


bool EvaluationTargetData::hasTstBuffer () const
{
    return tst_buffer != nullptr;
}

void EvaluationTargetData::setTstBuffer (std::shared_ptr<Buffer> buffer)
{
    assert (!tst_buffer);
    tst_buffer = buffer;
}

void EvaluationTargetData::addTstIndex (float tod, unsigned int index)
{
    tst_data_.insert({tod, index});
}

bool EvaluationTargetData::hasData() const
{
    return ref_data_.size() || tst_data_.size();
}

bool EvaluationTargetData::hasRefData () const
{
    return ref_data_.size();
}

bool EvaluationTargetData::hasTstData () const
{
    return tst_data_.size();
}

void EvaluationTargetData::finalize ()
{
//    loginf << "EvaluationTargetData: finalize: utn " << utn_
//           << " ref " << hasRefData() << " up " << ref_rec_nums_.size()
//           << " tst " << hasTstData() << " up " << tst_rec_nums_.size();

    for (auto& ref_it : ref_data_)
        ref_indexes_.push_back(ref_it.second);

    for (auto& tst_it : tst_data_)
        tst_indexes_.push_back(tst_it.second);

    updateModeACodes();
}

unsigned int EvaluationTargetData::numUpdates () const
{
    return ref_data_.size() + tst_data_.size();
}

unsigned int EvaluationTargetData::numRefUpdates () const
{
    return ref_data_.size();
}
unsigned int EvaluationTargetData::numTstUpdates () const
{
    return tst_data_.size();
}

float EvaluationTargetData::timeBegin() const
{
    if (ref_data_.size() && tst_data_.size())
        return min(ref_data_.begin()->first, tst_data_.begin()->first);
    else if (ref_data_.size())
        return ref_data_.begin()->first;
    else if (tst_data_.size())
        return tst_data_.begin()->first;
    else
        throw std::runtime_error("EvaluationTargetData: timeBegin: no data");
}

float EvaluationTargetData::timeEnd() const
{
    if (ref_data_.size() && tst_data_.size())
        return max(ref_data_.rbegin()->first, tst_data_.rbegin()->first);
    else if (ref_data_.size())
        return ref_data_.rbegin()->first;
    else if (tst_data_.size())
        return tst_data_.rbegin()->first;
    else
        throw std::runtime_error("EvaluationTargetData: timeEnd: no data");
}

std::vector<unsigned int> EvaluationTargetData::modeACodes() const
{
    logdbg << "EvaluationTargetData: modeACodes: utn " << utn_ << " num codes " << mode_a_codes_.size();
    return mode_a_codes_;
}

void EvaluationTargetData::updateModeACodes()
{
    logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_;

    mode_a_codes_.clear();

    if (ref_data_.size())
    {
        NullableVector<int>& mode_a_codes = ref_buffer->get<int>("mode3a_code");
        map<int, vector<unsigned int>> distinct_codes = mode_a_codes.distinctValuesWithIndexes(ref_indexes_);
        //unsigned int null_cnt = mode_a_codes.nullValueIndexes(ref_rec_nums_).size();

        for (auto& ma_it : distinct_codes)
        {
            if (find(mode_a_codes_.begin(), mode_a_codes_.end(), ma_it.first) == mode_a_codes_.end())
            {
                logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_ << " new ref m3a "
                       << String::octStringFromInt(ma_it.first, 4, '0');
                mode_a_codes_.push_back(ma_it.first);
            }
        }
    }

    if (tst_data_.size())
    {
        NullableVector<int>& mode_a_codes = tst_buffer->get<int>("mode3a_code");
        map<int, vector<unsigned int>> distinct_codes = mode_a_codes.distinctValuesWithIndexes(tst_indexes_);
        //unsigned int null_cnt = mode_a_codes.nullValueIndexes(tst_rec_nums_).size();

        for (auto& ma_it : distinct_codes)
        {
            if (find(mode_a_codes_.begin(), mode_a_codes_.end(), ma_it.first) == mode_a_codes_.end())
            {
                logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_ << " new tst m3a "
                       << String::octStringFromInt(ma_it.first, 4, '0');
                mode_a_codes_.push_back(ma_it.first);
            }
        }
    }

    logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_ << " num codes " << mode_a_codes_.size();
}

