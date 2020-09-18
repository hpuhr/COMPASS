#include "evaluationdata.h"
#include "dbobject.h"
#include "buffer.h"

EvaluationData::EvaluationData(EvaluationManager& eval_man)
    : eval_man_(eval_man)
{

}

void EvaluationData::addReferenceData (DBObject& object, std::shared_ptr<Buffer> buffer)
{
    loginf << "EvaluationData: addReferenceData: dbo " << object.name() << " size " << buffer->size();

    if (!object.hasAssociations())
    {
        logwrn << "EvaluationData: addReferenceData: object " << object.name() << " has no associations";
        unassociated_ref_cnt_ = buffer->size();

        return;
    }

    const DBOAssociationCollection& associations = object.associations();

    unsigned int buffer_size = buffer->size();
    NullableVector<int>& rec_nums = buffer->get<int>("rec_num");
    NullableVector<float>& tods = buffer->get<float>("tod");

    unsigned int rec_num;
    float tod;

    loginf << "EvaluationData: addReferenceData: adding target data";

    for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
    {
        assert (!rec_nums.isNull(cnt));
        assert (!tods.isNull(cnt));

        rec_num = rec_nums.get(cnt);
        tod = tods.get(cnt);

        std::vector<unsigned int> utn_vec = associations.getUTNsFor(rec_num);

        if (!utn_vec.size())
        {
            ++unassociated_ref_cnt_;
            continue;
        }

        for (auto utn_it : utn_vec)
        {
            if (!target_data_.count(utn_it))
            {
                target_data_.emplace(std::piecewise_construct,
                                          std::forward_as_tuple(utn_it),  // args for key
                                          std::forward_as_tuple(utn_it));
            }

            EvaluationTargetData& target_data = target_data_.at(utn_it);

            if (!target_data.hasRefBuffer())
                target_data.setRefBuffer(buffer);

            target_data.addRefRecNum(tod, rec_num);
            ++associated_ref_cnt_;
        }
    }

    loginf << "EvaluationData: addReferenceData: num targets " << target_data_.size()
           << " ref associated cnt " << associated_ref_cnt_ << " unassoc " << unassociated_ref_cnt_;
}

void EvaluationData::addTestData (DBObject& object, std::shared_ptr<Buffer> buffer)
{
    loginf << "EvaluationData: addTestData: dbo " << object.name() << " size " << buffer->size();

    if (!object.hasAssociations())
    {
        logwrn << "EvaluationData: addTestData: object " << object.name() << " has no associations";
        unassociated_tst_cnt_ = buffer->size();

        return;
    }

    const DBOAssociationCollection& associations = object.associations();

    unsigned int buffer_size = buffer->size();
    NullableVector<int>& rec_nums = buffer->get<int>("rec_num");
    NullableVector<float>& tods = buffer->get<float>("tod");

    unsigned int rec_num;
    float tod;

    loginf << "EvaluationData: addTestData: adding target data";

    for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
    {
        assert (!rec_nums.isNull(cnt));
        assert (!tods.isNull(cnt));

        rec_num = rec_nums.get(cnt);
        tod = tods.get(cnt);

        std::vector<unsigned int> utn_vec = associations.getUTNsFor(rec_num);

        if (!utn_vec.size())
        {
            ++unassociated_tst_cnt_;
            continue;
        }

        for (auto utn_it : utn_vec)
        {
            if (!target_data_.count(utn_it))
            {
                target_data_.emplace(std::piecewise_construct,
                                          std::forward_as_tuple(utn_it),  // args for key
                                          std::forward_as_tuple(utn_it));
            }

            EvaluationTargetData& target_data = target_data_.at(utn_it);

            if (!target_data.hasTstBuffer())
                target_data.setTstBuffer(buffer);

            target_data.addTstRecNum(tod, rec_num);
            ++associated_tst_cnt_;
        }
    }

    loginf << "EvaluationData: addTestData: num targets " << target_data_.size()
           << " tst associated cnt " << associated_tst_cnt_ << " unassoc " << unassociated_tst_cnt_;
}

void EvaluationData::finalize ()
{
    loginf << "EvaluationData: finalize";

    for (auto& target_it : target_data_)
        target_it.second.finalize();
}

void EvaluationData::clear()
{
    target_data_.clear();

    unassociated_ref_cnt_ = 0;
    associated_ref_cnt_ = 0;

    unassociated_tst_cnt_ = 0;
    associated_tst_cnt_ = 0;
}
