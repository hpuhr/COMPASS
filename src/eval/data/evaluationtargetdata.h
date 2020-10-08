#ifndef EVALUATIONTARGETDATA_H
#define EVALUATIONTARGETDATA_H

#include "evaluationtargetposition.h"

#include <map>
#include <memory>
#include <vector>

class Buffer;

class EvaluationTargetData
{
public:
    EvaluationTargetData(unsigned int utn);

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

    void finalize ();

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

    bool hasRefDataForTime (float tod, float d_max) const;
    std::pair<float, float> refTimesFor (float tod) const;
    std::pair<EvaluationTargetPosition, bool> interpolatedRefPosForTime (float tod, float d_max) const;
    // bool ok

    bool hasRefPosForTime (float tod) const;
    EvaluationTargetPosition refPosForTime (float tod) const;

    bool hasTstPosForTime (float tod) const;
    EvaluationTargetPosition tstPosForTime (float tod) const;

    // nullptr if none

protected:
    bool use_ {true};

    std::multimap<float, unsigned int> ref_data_; // tod -> index
    std::vector<unsigned int> ref_indexes_;

    std::multimap<float, unsigned int> tst_data_; // tod -> index
    std::vector<unsigned int> tst_indexes_;

    std::shared_ptr<Buffer> ref_buffer;
    std::string ref_latitude_name;
    std::string ref_longitude_name;
    std::string ref_altitude_name;


    std::shared_ptr<Buffer> tst_buffer;
    std::string tst_latitude_name;
    std::string tst_longitude_name;
    std::string tst_altitude_name;

    std::vector<std::string> callsigns_;
    std::vector<unsigned int> target_addresses_;
    std::vector<unsigned int> mode_a_codes_;

    bool has_mode_c_ {false};
    int mode_c_min_ {0};
    int mode_c_max_ {0};

    void updateCallsigns();
    void updateTargetAddresses();
    void updateModeACodes();
    void updateModeCMinMax();
};

#endif // EVALUATIONTARGETDATA_H
