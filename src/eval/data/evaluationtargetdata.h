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

#ifndef EVALUATIONTARGETDATA_H
#define EVALUATIONTARGETDATA_H

#include "evaluationtargetposition.h"

#include <map>
#include <memory>
#include <vector>

class Buffer;
class EvaluationManager;

class TstDataMapping // mapping to respective ref data
{
public:
    float tod_ {0}; // tod of test

    bool has_ref1_ {false};
    float tod_ref1_ {0};

    bool has_ref2_ {false};
    float tod_ref2_ {0};

    bool has_ref_pos_ {false};
    EvaluationTargetPosition pos_ref_;
};

class EvaluationTargetData
{
public:
    EvaluationTargetData(unsigned int utn, EvaluationManager& eval_man);

    bool hasRefBuffer () const;
    void setRefBuffer (std::shared_ptr<Buffer> buffer);
    void addRefIndex (float tod, unsigned int index);
    std::shared_ptr<Buffer> refBuffer() const;

    bool hasTstBuffer () const;
    void setTstBuffer (std::shared_ptr<Buffer> buffer);
    std::shared_ptr<Buffer> tstBuffer() const;
    void addTstIndex (float tod, unsigned int index);

    bool hasData() const;
    bool hasRefData () const;
    bool hasTstData () const;

    void finalize () const;

    const unsigned int utn_{0};

    unsigned int numUpdates () const;
    unsigned int numRefUpdates () const;
    unsigned int numTstUpdates () const;

    float timeBegin() const;
    std::string timeBeginStr() const;
    float timeEnd() const;
    std::string timeEndStr() const;

    std::vector<std::string> callsigns() const;
    std::string callsignsStr() const;

    std::vector<unsigned int> targetAddresses() const;
    std::string targetAddressesStr() const;

    std::vector<unsigned int> modeACodes() const;
    std::string modeACodesStr() const;

    bool hasModeC() const;
    int modeCMin() const;
    std::string modeCMinStr() const;
    int modeCMax() const;
    std::string modeCMaxStr() const;

    bool use() const;
    void use(bool use);

    const std::multimap<float, unsigned int>& refData() const;
    const std::multimap<float, unsigned int>& tstData() const;

    // ref
    bool hasRefDataForTime (float tod, float d_max) const;
    std::pair<float, float> refTimesFor (float tod, float d_max) const; // lower/upper times, -1 if not existing
    std::pair<EvaluationTargetPosition, bool> interpolatedRefPosForTime (float tod, float d_max) const;
    // bool ok

    bool hasRefPosForTime (float tod) const;
    EvaluationTargetPosition refPosForTime (float tod) const;

    bool hasRefCallsignForTime (float tod) const;
    std::string refCallsignForTime (float tod) const;

    // test

    bool hasTstPosForTime (float tod) const;
    EvaluationTargetPosition tstPosForTime (float tod) const;

    bool hasTstCallsignForTime (float tod) const;
    std::string tstCallsignForTime (float tod) const;

    // nullptr if none

    double latitudeMin() const;
    double latitudeMax() const;
    double longitudeMin() const;
    double longitudeMax() const;

    bool hasPos() const;

protected:
    static bool in_appimage_;

    EvaluationManager& eval_man_;

    bool use_ {true};

    std::multimap<float, unsigned int> ref_data_; // tod -> index
    mutable std::vector<unsigned int> ref_indexes_;

    std::multimap<float, unsigned int> tst_data_; // tod -> index
    mutable std::vector<unsigned int> tst_indexes_;

    std::shared_ptr<Buffer> ref_buffer_;
    std::string ref_latitude_name_;
    std::string ref_longitude_name_;
    std::string ref_altitude_name_;
    std::string ref_callsign_name_;


    std::shared_ptr<Buffer> tst_buffer_;
    std::string tst_latitude_name_;
    std::string tst_longitude_name_;
    std::string tst_altitude_name_;
    std::string tst_callsign_name_;

    mutable std::vector<std::string> callsigns_;
    mutable std::vector<unsigned int> target_addresses_;
    mutable std::vector<unsigned int> mode_a_codes_;

    mutable bool has_mode_c_ {false};
    mutable int mode_c_min_ {0};
    mutable int mode_c_max_ {0};

    mutable bool has_pos_ {false};
    mutable double latitude_min_ {0};
    mutable double latitude_max_ {0};

    mutable double longitude_min_ {0};
    mutable double longitude_max_ {0};

    mutable std::map<float, TstDataMapping> test_data_mappings_;

    void updateCallsigns() const;
    void updateTargetAddresses() const;
    void updateModeACodes() const;
    void updateModeCMinMax() const;
    void updatePositionMinMax() const;

    void calculateTestDataMappings() const;
    TstDataMapping calculateTestDataMapping(float tod) const; // test tod
    void addRefPositiosToMapping (TstDataMapping& mapping) const;
    void addRefPositiosToMappingFast (TstDataMapping& mapping) const;
};

#endif // EVALUATIONTARGETDATA_H
