#include "evaluationtargetdata.h"
#include "buffer.h"
#include "logger.h"
#include "stringconv.h"
#include "dbovariable.h"
#include "metadbovariable.h"
#include "atsdb.h"
#include "dbobjectmanager.h"

#include <cassert>
#include <algorithm>

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

    updateCallsigns();
    updateTargetAddresses();
    updateModeACodes();
    updateModeCMinMax();
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

bool EvaluationTargetData::hasModeC() const
{
    return has_mode_c_;
}

int EvaluationTargetData::modeCMin() const
{
    assert (has_mode_c_);
    return mode_c_min_;
}

int EvaluationTargetData::modeCMax() const
{
    assert (has_mode_c_);
    return mode_c_max_;
}

bool EvaluationTargetData::use() const
{
    return use_;
}

void EvaluationTargetData::use(bool use)
{
    use_ = use;
}

const std::multimap<float, unsigned int>& EvaluationTargetData::refData() const
{
    return ref_data_;
}


const std::multimap<float, unsigned int>& EvaluationTargetData::tstData() const
{
    return tst_data_;
}

std::shared_ptr<Buffer> EvaluationTargetData::refBuffer() const
{
    return ref_buffer;
}

std::shared_ptr<Buffer> EvaluationTargetData::tstBuffer() const
{
    return tst_buffer;
}

std::vector<string> EvaluationTargetData::callsigns() const
{
    return callsigns_;
}

std::vector<unsigned int> EvaluationTargetData::targetAddresses() const
{
    return target_addresses_;
}

void EvaluationTargetData::updateCallsigns()
{
    callsigns_.clear();

    if (ref_data_.size())
    {
        NullableVector<string>& value_vec = ref_buffer->get<string>("callsign");
        map<string, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(ref_indexes_);

        for (auto& val_it : distinct_values)
        {
            if (find(callsigns_.begin(), callsigns_.end(), val_it.first) == callsigns_.end())
                callsigns_.push_back(val_it.first);
        }
    }

    if (tst_data_.size())
    {
        NullableVector<string>& value_vec = tst_buffer->get<string>("callsign");
        map<string, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(tst_indexes_);

        for (auto& val_it : distinct_values)
        {
            if (find(callsigns_.begin(), callsigns_.end(), val_it.first) == callsigns_.end())
                callsigns_.push_back(val_it.first);
        }
    }
}

void EvaluationTargetData::updateTargetAddresses()
{
    target_addresses_.clear();

    if (ref_data_.size())
    {
        NullableVector<int>& value_vec = ref_buffer->get<int>("target_addr");
        map<int, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(ref_indexes_);

        for (auto& val_it : distinct_values)
        {
            if (find(target_addresses_.begin(), target_addresses_.end(), val_it.first) == target_addresses_.end())
                target_addresses_.push_back(val_it.first);
        }
    }

    if (tst_data_.size())
    {
        NullableVector<int>& value_vec = tst_buffer->get<int>("target_addr");
        map<int, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(tst_indexes_);

        for (auto& val_it : distinct_values)
        {
            if (find(target_addresses_.begin(), target_addresses_.end(), val_it.first) == target_addresses_.end())
                target_addresses_.push_back(val_it.first);
        }
    }
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

void EvaluationTargetData::updateModeCMinMax()
{
    logdbg << "EvaluationTargetData: updateModeC: utn " << utn_;

    // TODO garbled, valid flags

    has_mode_c_ = false;

    DBObjectManager& object_man = ATSDB::instance().objectManager();

    if (ref_data_.size())
    {
        string modec_name = object_man.metaVariable("modec_code_ft").getFor(ref_buffer->dboName()).name();

        assert (ref_buffer->has<int>(modec_name));
        NullableVector<int>& modec_codes_ft = ref_buffer->get<int>(modec_name);

        for (auto ind_it : ref_indexes_)
        {
            assert (ind_it < modec_codes_ft.size());

            if (!modec_codes_ft.isNull(ind_it))
            {
                if (!has_mode_c_)
                {
                    has_mode_c_ = true;
                    mode_c_min_ = modec_codes_ft.get(ind_it);
                    mode_c_max_ = modec_codes_ft.get(ind_it);
                }
                else
                {
                    mode_c_min_ = min(mode_c_min_, modec_codes_ft.get(ind_it));
                    mode_c_max_ = max(mode_c_max_, modec_codes_ft.get(ind_it));
                }
            }
        }
    }

    if (tst_data_.size())
    {
        string modec_name = object_man.metaVariable("modec_code_ft").getFor(tst_buffer->dboName()).name();

        assert (tst_buffer->has<int>(modec_name));
        NullableVector<int>& modec_codes_ft = tst_buffer->get<int>(modec_name);

        for (auto ind_it : tst_indexes_)
        {
            assert (ind_it < modec_codes_ft.size());

            if (!modec_codes_ft.isNull(ind_it))
            {
                if (!has_mode_c_)
                {
                    has_mode_c_ = true;
                    mode_c_min_ = modec_codes_ft.get(ind_it);
                    mode_c_max_ = modec_codes_ft.get(ind_it);
                }
                else
                {
                    mode_c_min_ = min(mode_c_min_, modec_codes_ft.get(ind_it));
                    mode_c_max_ = max(mode_c_max_, modec_codes_ft.get(ind_it));
                }
            }
        }
    }
}
