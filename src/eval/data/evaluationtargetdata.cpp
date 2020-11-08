/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "evaluationtargetdata.h"
#include "buffer.h"
#include "logger.h"
#include "stringconv.h"
#include "dbovariable.h"
#include "metadbovariable.h"
#include "compass.h"
#include "dbobjectmanager.h"

#include <ogr_spatialref.h>

#include <cassert>
#include <algorithm>

using namespace std;
using namespace Utils;

bool EvaluationTargetData::in_appimage_ {getenv("APPDIR")};

EvaluationTargetData::EvaluationTargetData(unsigned int utn)
    : utn_(utn)
{
}

bool EvaluationTargetData::hasRefBuffer () const
{
    return ref_buffer_ != nullptr;
}

void EvaluationTargetData::setRefBuffer (std::shared_ptr<Buffer> buffer)
{
    assert (!ref_buffer_);
    ref_buffer_ = buffer;

    DBObjectManager& object_manager = COMPASS::instance().objectManager();

    string dbo_name = ref_buffer_->dboName();

    ref_latitude_name_ = object_manager.metaVariable("pos_lat_deg").getFor(dbo_name).name();
    ref_longitude_name_ = object_manager.metaVariable("pos_long_deg").getFor(dbo_name).name();
    ref_altitude_name_ = object_manager.metaVariable("modec_code_ft").getFor(dbo_name).name();
    ref_callsign_name_ = object_manager.metaVariable("callsign").getFor(dbo_name).name();
}

void EvaluationTargetData::addRefIndex (float tod, unsigned int index)
{
    ref_data_.insert({tod, index});
}


bool EvaluationTargetData::hasTstBuffer () const
{
    return tst_buffer_ != nullptr;
}

void EvaluationTargetData::setTstBuffer (std::shared_ptr<Buffer> buffer)
{
    assert (!tst_buffer_);
    tst_buffer_ = buffer;

    DBObjectManager& object_manager = COMPASS::instance().objectManager();

    string dbo_name = tst_buffer_->dboName();

    tst_latitude_name_ = object_manager.metaVariable("pos_lat_deg").getFor(dbo_name).name();
    tst_longitude_name_ = object_manager.metaVariable("pos_long_deg").getFor(dbo_name).name();
    tst_altitude_name_ = object_manager.metaVariable("modec_code_ft").getFor(dbo_name).name();
    tst_callsign_name_ = object_manager.metaVariable("callsign").getFor(dbo_name).name();
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

void EvaluationTargetData::finalize () const
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
    updatePositionMinMax();

    calculateTestDataMappings();
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

std::string EvaluationTargetData::timeBeginStr() const
{
    if (hasData())
        return String::timeStringFromDouble(timeBegin()).c_str();
    else
        return "";
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

std::string EvaluationTargetData::timeEndStr() const
{
    if (hasData())
        return String::timeStringFromDouble(timeEnd()).c_str();
    else
        return "";
}

std::vector<unsigned int> EvaluationTargetData::modeACodes() const
{
    logdbg << "EvaluationTargetData: modeACodes: utn " << utn_ << " num codes " << mode_a_codes_.size();
    return mode_a_codes_;
}

std::string EvaluationTargetData::modeACodesStr() const
{
    std::ostringstream out;

    for (unsigned int cnt=0; cnt < mode_a_codes_.size(); ++cnt)
    {
        if (cnt != 0)
            out << ",";
        out << String::octStringFromInt(mode_a_codes_.at(cnt), 4, '0');
    }

    return out.str();
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

std::string EvaluationTargetData::modeCMinStr() const
{
    if (has_mode_c_)
        return to_string(mode_c_min_);
    else
        return "";
}

int EvaluationTargetData::modeCMax() const
{
    assert (has_mode_c_);
    return mode_c_max_;
}

std::string EvaluationTargetData::modeCMaxStr() const
{
    if (has_mode_c_)
        return to_string(mode_c_max_);
    else
        return "";
}

bool EvaluationTargetData::use() const
{
    return use_;
}

void EvaluationTargetData::use(bool use)
{
    loginf << "EvaluationTargetData: use: utn " << utn_ << " use " << use;

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

bool EvaluationTargetData::hasRefDataForTime (float tod, float d_max) const
{
    //    if (ref_data_.count(tod))
    //        return true; // contains exact value

    //    //    Return iterator to lower bound
    //    //    Returns an iterator pointing to the first element in the container whose key is not considered to go
    //    //    before k (i.e., either it is equivalent or goes after).

    //    auto lb_it = ref_data_.lower_bound(tod);

    //    if (lb_it == ref_data_.end())
    //        return false;

    //    assert (lb_it->first >= tod);

    //    if (lb_it->first - tod > d_max)
    //        return false; // too much time difference

    //    // save value
    //    float upper = lb_it->first;

    //    lb_it--;

    //    if (lb_it == ref_data_.end())
    //        return false;

    //    assert (tod >= lb_it->first);

    //    if (tod - lb_it->first > d_max)
    //        return false; // too much time difference

    //    float lower = lb_it->first;

    //    logdbg << "EvaluationTargetData: hasRefDataForTime: found " << String::timeStringFromDouble(lower)
    //           << " <= " << String::timeStringFromDouble(tod)
    //           << " <= " << String::timeStringFromDouble(upper);

    //    return true;
    assert (test_data_mappings_.count(tod));
    TstDataMapping& mapping = test_data_mappings_.at(tod);

    if (!mapping.has_ref1_ && !mapping.has_ref2_) // no ref data
        return false;

    if (mapping.has_ref1_ && !mapping.has_ref2_) // exact time
    {
        assert (mapping.tod_ref1_ == tod);
        return true;
    }

    if (mapping.has_ref1_ && mapping.has_ref2_) // interpolated
    {
        assert (mapping.tod_ref1_ <= tod);
        assert (mapping.tod_ref2_ >= tod);

        if (tod - mapping.tod_ref1_ > d_max) // lower to far
            return false;

        if (mapping.tod_ref2_ - tod > d_max) // upper to far
            return false;

        return true;
    }

    return false;
}

std::pair<float, float> EvaluationTargetData::refTimesFor (float tod, float d_max)  const
{
    //    if (ref_data_.count(tod))
    //        return {tod, -1}; // contains exact value

    //    //    Return iterator to lower bound
    //    //    Returns an iterator pointing to the first element in the container whose key is not considered to go
    //    //    before k (i.e., either it is equivalent or goes after).

    //    auto lb_it = ref_data_.lower_bound(tod);

    //    if (lb_it == ref_data_.end())
    //        return {-1, -1};

    //    assert (lb_it->first >= tod);

    //    if (lb_it->first - tod > d_max)
    //        return {-1, -1}; // too much time difference

    //    // save value
    //    float upper = lb_it->first;

    //    lb_it--;

    //    if (lb_it == ref_data_.end())
    //        return {-1, upper};

    //    assert (tod >= lb_it->first);

    //    if (tod - lb_it->first > d_max)
    //        return {-1, upper}; // too much time difference

    //    float lower = lb_it->first;

    //    return {lower, upper};

    assert (test_data_mappings_.count(tod));
    TstDataMapping& mapping = test_data_mappings_.at(tod);

    if (!mapping.has_ref1_ && !mapping.has_ref2_) // no ref data
        return {-1, -1};

    if (mapping.has_ref1_ && !mapping.has_ref2_) // exact time
    {
        assert (mapping.tod_ref1_ == tod);
        return {tod, -1};
    }

    if (mapping.has_ref1_ && mapping.has_ref2_) // interpolated
    {
        assert (mapping.tod_ref1_ <= tod);
        assert (mapping.tod_ref2_ >= tod);

        if (tod - mapping.tod_ref1_ > d_max) // lower to far
            return {-1, -1};

        if (mapping.tod_ref2_ - tod > d_max) // upper to far
            return {-1, -1};

        return {mapping.tod_ref1_, mapping.tod_ref2_};
    }

    return {-1, -1};
}

std::pair<EvaluationTargetPosition, bool>  EvaluationTargetData::interpolatedRefPosForTime (
        float tod, float d_max) const
{
    //    if (ref_data_.count(tod)) // contains exact value
    //        return {refPosForTime(tod), true};

    //    //    Return iterator to lower bound
    //    //    Returns an iterator pointing to the first element in the container whose key is not considered to go
    //    //    before k (i.e., either it is equivalent or goes after).

    //    auto lb_it = ref_data_.lower_bound(tod);

    //    assert (lb_it != ref_data_.end());
    //    assert (lb_it->first >= tod); // too much time difference

    //    assert (lb_it->first - tod <= d_max); // too much time difference

    //    // save value
    //    float upper = lb_it->first;

    //    lb_it--;

    //    assert (lb_it != ref_data_.end());

    //    assert (tod >= lb_it->first);

    //    assert (tod - lb_it->first <= d_max); // too much time difference

    //    float lower = lb_it->first;

    //    EvaluationTargetPosition pos1 = refPosForTime(lower);
    //    EvaluationTargetPosition pos2 = refPosForTime(upper);
    //    float d_t = upper - lower;

    //    logdbg << "EvaluationTargetData: interpolatedRefPosForTime: d_t " << d_t;

    //    assert (d_t >= 0);

    //    if (pos1.latitude_ == pos2.latitude_
    //            && pos1.longitude_ == pos2.longitude_) // same pos
    //        return {pos1, true};

    //    if (lower == upper) // same time
    //    {
    //        logwrn << "EvaluationTargetData: interpolatedRefPosForTime: ref has same time twice";
    //        return {{}, false};
    //    }

    //    OGRSpatialReference wgs84;
    //    wgs84.SetWellKnownGeogCS("WGS84");
    //    OGRSpatialReference local;
    //    local.SetStereographic(pos1.latitude_, pos1.longitude_, 1.0, 0.0, 0.0);

    //    logdbg << "EvaluationTargetData: interpolatedRefPosForTime: pos1 " << pos1.latitude_ << ", " << pos1.longitude_;
    //    logdbg << "EvaluationTargetData: interpolatedRefPosForTime: pos2 " << pos2.latitude_ << ", " << pos2.longitude_;

    //    std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart {OGRCreateCoordinateTransformation(&wgs84, &local)};
    //    assert (ogr_geo2cart);
    //    std::unique_ptr<OGRCoordinateTransformation> ogr_cart2geo {OGRCreateCoordinateTransformation(&local, &wgs84)};
    //    assert (ogr_cart2geo);

    //    double x_pos, y_pos;

    //    if (in_appimage_) // inside appimage
    //    {
    //        x_pos = pos2.longitude_;
    //        y_pos = pos2.latitude_;
    //    }
    //    else
    //    {
    //        x_pos = pos2.latitude_;
    //        y_pos = pos2.longitude_;
    //    }

    //    logdbg << "EvaluationTargetData: interpolatedRefPosForTime: geo2cart";
    //    bool ret = ogr_geo2cart->Transform(1, &x_pos, &y_pos); // wgs84 to cartesian offsets
    //    if (!ret)
    //    {
    //        logerr << "EvaluationTargetData: interpolatedRefPosForTime: error with latitude " << pos2.latitude_
    //               << " longitude " << pos2.longitude_;
    //        return {{}, false};
    //    }

    //    logdbg << "EvaluationTargetData: interpolatedRefPosForTime: offsets x " << fixed << x_pos
    //           << " y " << fixed << y_pos << " dist " << fixed << sqrt(pow(x_pos,2)+pow(y_pos,2));

    //    double v_x = x_pos/d_t;
    //    double v_y = y_pos/d_t;
    //    logdbg << "EvaluationTargetData: interpolatedRefPosForTime: v_x " << v_x << " v_y " << v_y;

    //    float d_t2 = tod - lower;
    //    logdbg << "EvaluationTargetData: interpolatedRefPosForTime: d_t2 " << d_t2;

    //    assert (d_t2 >= 0);

    //    x_pos = v_x * d_t2;
    //    y_pos = v_y * d_t2;

    //    logdbg << "EvaluationTargetData: interpolatedRefPosForTime: interpolated offsets x " << x_pos << " y " << y_pos;

    //    ret = ogr_cart2geo->Transform(1, &x_pos, &y_pos);

    //    // x_pos long, y_pos lat

    //    logdbg << "EvaluationTargetData: interpolatedRefPosForTime: interpolated lat " << y_pos << " long " << x_pos;

    //    // calculate altitude
    //    bool has_altitude = false;
    //    float altitude = 0.0;

    //    if (pos1.has_altitude_ && !pos2.has_altitude_)
    //    {
    //        has_altitude = true;
    //        altitude = pos1.altitude_;
    //    }
    //    else if (!pos1.has_altitude_ && pos2.has_altitude_)
    //    {
    //        has_altitude = true;
    //        altitude = pos2.altitude_;
    //    }
    //    else if (pos1.has_altitude_ && pos2.has_altitude_)
    //    {
    //        float v_alt = (pos2.altitude_ - pos1.altitude_)/d_t;
    //        has_altitude = true;
    //        altitude = pos1.altitude_ + v_alt*d_t2;
    //    }

    //    logdbg << "EvaluationTargetData: interpolatedRefPosForTime: pos1 has alt "
    //           << pos1.has_altitude_ << " alt " << pos1.altitude_
    //           << " pos2 has alt " << pos2.has_altitude_ << " alt " << pos2.altitude_
    //           << " interpolated has alt " << has_altitude << " alt " << altitude;

    //    if (in_appimage_) // inside appimage
    //        return {{y_pos, x_pos, has_altitude, altitude}, true};
    //    else
    //        return {{x_pos, y_pos, has_altitude, altitude}, true};

    assert (test_data_mappings_.count(tod));
    TstDataMapping& mapping = test_data_mappings_.at(tod);

    if (!mapping.has_ref1_ && !mapping.has_ref2_) // no ref data
        return {{}, false};

    if (mapping.has_ref1_ && !mapping.has_ref2_) // exact time
    {
        assert (mapping.tod_ref1_ == tod);
        return {refPosForTime(tod), true};
    }

    if (mapping.has_ref1_ && mapping.has_ref2_) // interpolated
    {
        assert (mapping.tod_ref1_ <= tod);
        assert (mapping.tod_ref2_ >= tod);

        if (tod - mapping.tod_ref1_ > d_max) // lower to far
            return {{}, false};

        if (mapping.tod_ref2_ - tod > d_max) // upper to far
            return {{}, false};

        if (!mapping.has_ref_pos_)
            return {{}, false};

        return {mapping.pos_ref_, true};
    }

    return {{}, false};
}

bool EvaluationTargetData::hasRefPosForTime (float tod) const
{
    return ref_data_.count(tod);
}

EvaluationTargetPosition EvaluationTargetData::refPosForTime (float tod) const
{
    assert (hasRefPosForTime(tod));

    auto it_pair = ref_data_.equal_range(tod);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    EvaluationTargetPosition pos;

    NullableVector<double>& latitude_vec = ref_buffer_->get<double>(ref_latitude_name_);
    NullableVector<double>& longitude_vec = ref_buffer_->get<double>(ref_longitude_name_);
    NullableVector<int>& altitude_vec = ref_buffer_->get<int>(ref_altitude_name_);

    assert (!latitude_vec.isNull(index));
    assert (!longitude_vec.isNull(index));

    pos.latitude_ = latitude_vec.get(index);
    pos.longitude_ = longitude_vec.get(index);

    if (!altitude_vec.isNull(index))
    {
        pos.has_altitude_ = true;
        pos.altitude_ = altitude_vec.get(index);
    }

    return pos;
}

bool EvaluationTargetData::hasRefCallsignForTime (float tod) const
{
    if (!ref_data_.count(tod))
        return false;

    auto it_pair = ref_data_.equal_range(tod);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    NullableVector<string>& callsign_vec = ref_buffer_->get<string>(ref_callsign_name_);

    return !callsign_vec.isNull(index);
}

std::string EvaluationTargetData::refCallsignForTime (float tod) const
{
    assert (hasRefPosForTime(tod));

    auto it_pair = ref_data_.equal_range(tod);

    assert (it_pair.first != ref_data_.end());

    unsigned int index = it_pair.first->second;

    NullableVector<string>& callsign_vec = ref_buffer_->get<string>(ref_callsign_name_);
    assert (!callsign_vec.isNull(index));

    return callsign_vec.get(index);
}

bool EvaluationTargetData::hasTstPosForTime (float tod) const
{
    return tst_data_.count(tod);
}

EvaluationTargetPosition EvaluationTargetData::tstPosForTime (float tod) const
{
    assert (hasTstPosForTime(tod));

    auto it_pair = tst_data_.equal_range(tod);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    EvaluationTargetPosition pos;

    NullableVector<double>& latitude_vec = tst_buffer_->get<double>(tst_latitude_name_);
    NullableVector<double>& longitude_vec = tst_buffer_->get<double>(tst_longitude_name_);
    NullableVector<int>& altitude_vec = tst_buffer_->get<int>(tst_altitude_name_);

    assert (!latitude_vec.isNull(index));
    assert (!longitude_vec.isNull(index));

    pos.latitude_ = latitude_vec.get(index);
    pos.longitude_ = longitude_vec.get(index);

    if (!altitude_vec.isNull(index))
        pos.altitude_ = altitude_vec.get(index);

    return pos;
}

bool EvaluationTargetData::hasTstCallsignForTime (float tod) const
{
    if (!tst_data_.count(tod))
        return false;

    auto it_pair = tst_data_.equal_range(tod);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    NullableVector<string>& callsign_vec = tst_buffer_->get<string>(tst_callsign_name_);
    return !callsign_vec.isNull(index);
}

std::string EvaluationTargetData::tstCallsignForTime (float tod) const
{
    assert (hasTstPosForTime(tod));

    auto it_pair = tst_data_.equal_range(tod);

    assert (it_pair.first != tst_data_.end());

    unsigned int index = it_pair.first->second;

    NullableVector<string>& callsign_vec = tst_buffer_->get<string>(tst_callsign_name_);
    assert (!callsign_vec.isNull(index));

    return callsign_vec.get(index);
}

double EvaluationTargetData::latitudeMin() const
{
    assert (has_pos_);
    return latitude_min_;
}

double EvaluationTargetData::latitudeMax() const
{
    assert (has_pos_);
    return latitude_max_;
}

double EvaluationTargetData::longitudeMin() const
{
    assert (has_pos_);
    return longitude_min_;
}

double EvaluationTargetData::longitudeMax() const
{
    assert (has_pos_);
    return longitude_max_;
}

bool EvaluationTargetData::hasPos() const
{
    return has_pos_;
}

std::shared_ptr<Buffer> EvaluationTargetData::refBuffer() const
{
    return ref_buffer_;
}

std::shared_ptr<Buffer> EvaluationTargetData::tstBuffer() const
{
    return tst_buffer_;
}

std::vector<string> EvaluationTargetData::callsigns() const
{
    return callsigns_;
}

string EvaluationTargetData::callsignsStr() const
{
    std::ostringstream out;

    for (unsigned int cnt=0; cnt < callsigns_.size(); ++cnt)
    {
        if (cnt != 0)
            out << ",";
        out << callsigns_.at(cnt);
    }

    return out.str().c_str();
}

std::vector<unsigned int> EvaluationTargetData::targetAddresses() const
{
    return target_addresses_;
}

std::string EvaluationTargetData::targetAddressesStr() const
{
    std::ostringstream out;

    for (unsigned int cnt=0; cnt < target_addresses_.size(); ++cnt)
    {
        if (cnt != 0)
            out << ",";
        out << String::hexStringFromInt(target_addresses_.at(cnt), 6, '0');
    }

    return out.str().c_str();
}

void EvaluationTargetData::updateCallsigns() const
{
    callsigns_.clear();

    if (ref_data_.size())
    {
        NullableVector<string>& value_vec = ref_buffer_->get<string>("callsign");
        map<string, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(ref_indexes_);

        for (auto& val_it : distinct_values)
        {
            if (find(callsigns_.begin(), callsigns_.end(), val_it.first) == callsigns_.end())
                callsigns_.push_back(val_it.first);
        }
    }

    if (tst_data_.size())
    {
        NullableVector<string>& value_vec = tst_buffer_->get<string>("callsign");
        map<string, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(tst_indexes_);

        for (auto& val_it : distinct_values)
        {
            if (find(callsigns_.begin(), callsigns_.end(), val_it.first) == callsigns_.end())
                callsigns_.push_back(val_it.first);
        }
    }
}

void EvaluationTargetData::updateTargetAddresses() const
{
    target_addresses_.clear();

    if (ref_data_.size())
    {
        NullableVector<int>& value_vec = ref_buffer_->get<int>("target_addr");
        map<int, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(ref_indexes_);

        for (auto& val_it : distinct_values)
        {
            if (find(target_addresses_.begin(), target_addresses_.end(), val_it.first) == target_addresses_.end())
                target_addresses_.push_back(val_it.first);
        }
    }

    if (tst_data_.size())
    {
        NullableVector<int>& value_vec = tst_buffer_->get<int>("target_addr");
        map<int, vector<unsigned int>> distinct_values = value_vec.distinctValuesWithIndexes(tst_indexes_);

        for (auto& val_it : distinct_values)
        {
            if (find(target_addresses_.begin(), target_addresses_.end(), val_it.first) == target_addresses_.end())
                target_addresses_.push_back(val_it.first);
        }
    }
}

void EvaluationTargetData::updateModeACodes() const
{
    logdbg << "EvaluationTargetData: updateModeACodes: utn " << utn_;

    mode_a_codes_.clear();

    if (ref_data_.size())
    {
        NullableVector<int>& mode_a_codes = ref_buffer_->get<int>("mode3a_code");
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
        NullableVector<int>& mode_a_codes = tst_buffer_->get<int>("mode3a_code");
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

void EvaluationTargetData::updateModeCMinMax() const
{
    logdbg << "EvaluationTargetData: updateModeC: utn " << utn_;

    // TODO garbled, valid flags

    has_mode_c_ = false;

    DBObjectManager& object_man = COMPASS::instance().objectManager();

    if (ref_data_.size())
    {
        string modec_name = object_man.metaVariable("modec_code_ft").getFor(ref_buffer_->dboName()).name();

        assert (ref_buffer_->has<int>(modec_name));
        NullableVector<int>& modec_codes_ft = ref_buffer_->get<int>(modec_name);

        for (auto ind_it : ref_indexes_)
        {
            if (!modec_codes_ft.isNull(ind_it))
            {
                assert (ind_it < modec_codes_ft.size());

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
        string modec_name = object_man.metaVariable("modec_code_ft").getFor(tst_buffer_->dboName()).name();

        assert (tst_buffer_->has<int>(modec_name));
        NullableVector<int>& modec_codes_ft = tst_buffer_->get<int>(modec_name);

        for (auto ind_it : tst_indexes_)
        {
            if (!modec_codes_ft.isNull(ind_it))
            {
                assert (ind_it < modec_codes_ft.size());

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

void EvaluationTargetData::updatePositionMinMax() const
{
    DBObjectManager& object_man = COMPASS::instance().objectManager();

    has_pos_ = false;

    if (ref_data_.size())
    {
        string lat_name = object_man.metaVariable("pos_lat_deg").getFor(ref_buffer_->dboName()).name();
        string long_name = object_man.metaVariable("pos_long_deg").getFor(ref_buffer_->dboName()).name();

        assert (ref_buffer_->has<double>(lat_name));
        assert (ref_buffer_->has<double>(long_name));

        NullableVector<double>& lats = ref_buffer_->get<double>(lat_name);
        NullableVector<double>& longs = ref_buffer_->get<double>(long_name);

        for (auto ind_it : ref_indexes_)
        {
            assert (!lats.isNull(ind_it));
            assert (!longs.isNull(ind_it));

            if (!has_pos_)
            {
                has_pos_ = true;
                latitude_min_ = lats.get(ind_it);
                latitude_max_ = lats.get(ind_it);
                longitude_min_ = longs.get(ind_it);
                longitude_max_ = longs.get(ind_it);
            }
            else
            {
                latitude_min_ = min(latitude_min_, lats.get(ind_it));
                latitude_max_ = max(latitude_max_, lats.get(ind_it));
                longitude_min_ = min(longitude_min_, longs.get(ind_it));
                longitude_max_ = max(longitude_max_, longs.get(ind_it));
            }
        }
    }

    if (tst_data_.size())
    {
        string lat_name = object_man.metaVariable("pos_lat_deg").getFor(tst_buffer_->dboName()).name();
        string long_name = object_man.metaVariable("pos_long_deg").getFor(tst_buffer_->dboName()).name();

        assert (tst_buffer_->has<double>(lat_name));
        assert (tst_buffer_->has<double>(long_name));

        NullableVector<double>& lats = tst_buffer_->get<double>(lat_name);
        NullableVector<double>& longs = tst_buffer_->get<double>(long_name);

        for (auto ind_it : tst_indexes_)
        {
            assert (!lats.isNull(ind_it));
            assert (!longs.isNull(ind_it));

            if (!has_pos_)
            {
                has_pos_ = true;
                latitude_min_ = lats.get(ind_it);
                latitude_max_ = lats.get(ind_it);
                longitude_min_ = longs.get(ind_it);
                longitude_max_ = longs.get(ind_it);
            }
            else
            {
                latitude_min_ = min(latitude_min_, lats.get(ind_it));
                latitude_max_ = max(latitude_max_, lats.get(ind_it));
                longitude_min_ = min(longitude_min_, longs.get(ind_it));
                longitude_max_ = max(longitude_max_, longs.get(ind_it));
            }
        }
    }
}

void EvaluationTargetData::calculateTestDataMappings() const
{
    loginf << "EvaluationTargetData: calculateTestDataMappings: utn " << utn_;

    assert (!test_data_mappings_.size());

    for (auto& tst_it : tst_data_)
        test_data_mappings_[tst_it.first] = calculateTestDataMapping(tst_it.first);

    logdbg << "EvaluationTargetData: calculateTestDataMappings: utn " << utn_ << " done";
}

TstDataMapping EvaluationTargetData::calculateTestDataMapping(float tod) const
{
    TstDataMapping ret;

    ret.tod_ = tod;

    if (ref_data_.count(tod)) // contains exact value
    {
        ret.has_ref1_ = true;
        ret.tod_ref1_ = tod;
    }
    else
    {
        //    Return iterator to lower bound
        //    Returns an iterator pointing to the first element in the container whose key is not considered to go
        //    before k (i.e., either it is equivalent or goes after).

        auto lb_it = ref_data_.lower_bound(tod);

        if (lb_it != ref_data_.end()) // upper tod found
        {
            assert (lb_it->first >= tod);

            // save upper value
            ret.has_ref2_ = true;
            ret.tod_ref2_ = lb_it->first;

            // search lower values by decrementing iterator
            while (lb_it != ref_data_.end() && tod < lb_it->first)
            {
                if (lb_it == ref_data_.begin()) // exit condition on first value
                {
                    if (tod < lb_it->first) // set as not found
                        lb_it = ref_data_.end();

                    break;
                }

                lb_it--;
            }

            if (lb_it != ref_data_.end()) // lower tod found
            {
                assert (tod >= lb_it->first);

                // add lower value
                ret.has_ref1_ = true;
                ret.tod_ref1_ = lb_it->first;
            }
            else // not found, clear previous
            {
                ret.has_ref2_ = false;
                ret.tod_ref2_ = 0.0;
            }
        }
    }

    addRefPositiosToMappingFast(ret);

    return ret;
}

void EvaluationTargetData::addRefPositiosToMapping (TstDataMapping& mapping) const
{
    if (mapping.has_ref1_ && !mapping.has_ref2_) // exact first time
    {
        mapping.has_ref_pos_ = hasRefPosForTime(mapping.tod_ref1_);

        if (mapping.has_ref_pos_)
            mapping.pos_ref_ = refPosForTime(mapping.tod_ref1_);
    }
    else if (mapping.has_ref1_  && hasRefPosForTime(mapping.tod_ref1_)
             && mapping.has_ref2_ && hasRefPosForTime(mapping.tod_ref2_)) // two positions which can be interpolated
    {
        float lower = mapping.tod_ref1_;
        float upper = mapping.tod_ref2_;

        EvaluationTargetPosition pos1 = refPosForTime(lower);
        EvaluationTargetPosition pos2 = refPosForTime(upper);
        float d_t = upper - lower;

        logdbg << "EvaluationTargetData: addRefPositiosToMapping: d_t " << d_t;

        assert (d_t >= 0);

        if (pos1.latitude_ == pos2.latitude_ && pos1.longitude_ == pos2.longitude_) // same pos
        {
            mapping.has_ref_pos_ = true;
            mapping.pos_ref_ = pos1;
        }
        else
        {

            if (lower == upper) // same time
            {
                logwrn << "EvaluationTargetData: addRefPositiosToMapping: ref has same time twice";
            }
            else
            {
                OGRSpatialReference wgs84;
                wgs84.SetWellKnownGeogCS("WGS84");
                OGRSpatialReference local;
                local.SetStereographic(pos1.latitude_, pos1.longitude_, 1.0, 0.0, 0.0);

                logdbg << "EvaluationTargetData: addRefPositiosToMapping: pos1 "
                       << pos1.latitude_ << ", " << pos1.longitude_;
                logdbg << "EvaluationTargetData: addRefPositiosToMapping: pos2 "
                       << pos2.latitude_ << ", " << pos2.longitude_;

                std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart {OGRCreateCoordinateTransformation(&wgs84, &local)};
                assert (ogr_geo2cart);
                std::unique_ptr<OGRCoordinateTransformation> ogr_cart2geo {OGRCreateCoordinateTransformation(&local, &wgs84)};
                assert (ogr_cart2geo);

                double x_pos, y_pos;

                if (in_appimage_) // inside appimage
                {
                    x_pos = pos2.longitude_;
                    y_pos = pos2.latitude_;
                }
                else
                {
                    x_pos = pos2.latitude_;
                    y_pos = pos2.longitude_;
                }

                logdbg << "EvaluationTargetData: addRefPositiosToMapping: geo2cart";
                bool ret = ogr_geo2cart->Transform(1, &x_pos, &y_pos); // wgs84 to cartesian offsets
                if (!ret)
                {
                    logerr << "EvaluationTargetData: addRefPositiosToMapping: error with latitude " << pos2.latitude_
                           << " longitude " << pos2.longitude_;
                }
                else
                {

                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: offsets x " << fixed << x_pos
                           << " y " << fixed << y_pos << " dist " << fixed << sqrt(pow(x_pos,2)+pow(y_pos,2));

                    double v_x = x_pos/d_t;
                    double v_y = y_pos/d_t;
                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: v_x " << v_x << " v_y " << v_y;

                    float d_t2 = mapping.tod_ - lower;
                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: d_t2 " << d_t2;

                    assert (d_t2 >= 0);

                    x_pos = v_x * d_t2;
                    y_pos = v_y * d_t2;

                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: interpolated offsets x "
                           << x_pos << " y " << y_pos;

                    ret = ogr_cart2geo->Transform(1, &x_pos, &y_pos);

                    // x_pos long, y_pos lat

                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: interpolated lat "
                           << y_pos << " long " << x_pos;

                    // calculate altitude
                    bool has_altitude = false;
                    float altitude = 0.0;

                    if (pos1.has_altitude_ && !pos2.has_altitude_)
                    {
                        has_altitude = true;
                        altitude = pos1.altitude_;
                    }
                    else if (!pos1.has_altitude_ && pos2.has_altitude_)
                    {
                        has_altitude = true;
                        altitude = pos2.altitude_;
                    }
                    else if (pos1.has_altitude_ && pos2.has_altitude_)
                    {
                        float v_alt = (pos2.altitude_ - pos1.altitude_)/d_t;
                        has_altitude = true;
                        altitude = pos1.altitude_ + v_alt*d_t2;
                    }

                    logdbg << "EvaluationTargetData: addRefPositiosToMapping: pos1 has alt "
                           << pos1.has_altitude_ << " alt " << pos1.altitude_
                           << " pos2 has alt " << pos2.has_altitude_ << " alt " << pos2.altitude_
                           << " interpolated has alt " << has_altitude << " alt " << altitude;

                    mapping.has_ref_pos_ = true;

                    if (in_appimage_) // inside appimage
                        mapping.pos_ref_ = EvaluationTargetPosition(y_pos, x_pos, has_altitude, altitude);
                    else
                        mapping.pos_ref_ = EvaluationTargetPosition(x_pos, y_pos, has_altitude, altitude);
                }
            }
        }
    }
    // else do nothing
}

void EvaluationTargetData::addRefPositiosToMappingFast (TstDataMapping& mapping) const
{
    if (mapping.has_ref1_ && !mapping.has_ref2_) // exact first time
    {
        mapping.has_ref_pos_ = hasRefPosForTime(mapping.tod_ref1_);

        if (mapping.has_ref_pos_)
            mapping.pos_ref_ = refPosForTime(mapping.tod_ref1_);
    }
    else if (mapping.has_ref1_  && hasRefPosForTime(mapping.tod_ref1_)
             && mapping.has_ref2_ && hasRefPosForTime(mapping.tod_ref2_)) // two positions which can be interpolated
    {
        float lower = mapping.tod_ref1_;
        float upper = mapping.tod_ref2_;

        EvaluationTargetPosition pos1 = refPosForTime(lower);
        EvaluationTargetPosition pos2 = refPosForTime(upper);
        float d_t = upper - lower;

        logdbg << "EvaluationTargetData: addRefPositiosToMappingFast: d_t " << d_t;

        assert (d_t >= 0);

        if (pos1.latitude_ == pos2.latitude_ && pos1.longitude_ == pos2.longitude_) // same pos
        {
            mapping.has_ref_pos_ = true;
            mapping.pos_ref_ = pos1;
        }
        else
        {

            if (lower == upper) // same time
            {
                logwrn << "EvaluationTargetData: addRefPositiosToMappingFast: ref has same time twice";
            }
            else
            {
                double v_lat = (pos2.latitude_ - pos1.latitude_)/d_t;
                double v_long = (pos2.longitude_ - pos1.longitude_)/d_t;
                logdbg << "EvaluationTargetData: interpolatedPosForTimeFast: v_x " << v_lat << " v_y " << v_long;

                float d_t2 = mapping.tod_  - lower;
                logdbg << "EvaluationTargetData: interpolatedPosForTimeFast: d_t2 " << d_t2;

                assert (d_t2 >= 0);

                double int_lat = pos1.latitude_ + v_lat * d_t2;
                double int_long = pos1.longitude_ + v_long * d_t2;

                logdbg << "EvaluationTargetData: interpolatedPosForTimeFast: interpolated lat " << int_lat
                       << " long " << int_long;

                // calculate altitude
                bool has_altitude = false;
                float altitude = 0.0;

                if (pos1.has_altitude_ && !pos2.has_altitude_)
                {
                    has_altitude = true;
                    altitude = pos1.altitude_;
                }
                else if (!pos1.has_altitude_ && pos2.has_altitude_)
                {
                    has_altitude = true;
                    altitude = pos2.altitude_;
                }
                else if (pos1.has_altitude_ && pos2.has_altitude_)
                {
                    float v_alt = (pos2.altitude_ - pos1.altitude_)/d_t;
                    has_altitude = true;
                    altitude = pos1.altitude_ + v_alt*d_t2;
                }

                logdbg << "EvaluationTargetData: interpolatedPosForTimeFast: pos1 has alt "
                       << pos1.has_altitude_ << " alt " << pos1.altitude_
                       << " pos2 has alt " << pos2.has_altitude_ << " alt " << pos2.altitude_
                       << " interpolated has alt " << has_altitude << " alt " << altitude;

                mapping.has_ref_pos_ = true;

                mapping.pos_ref_ = EvaluationTargetPosition(int_lat, int_long, has_altitude, altitude);
            }
        }
    }
    // else do nothing
}
