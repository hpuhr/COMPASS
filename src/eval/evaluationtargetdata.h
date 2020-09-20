#ifndef EVALUATIONTARGETDATA_H
#define EVALUATIONTARGETDATA_H

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

    bool hasTstBuffer () const;
    void setTstBuffer (std::shared_ptr<Buffer> buffer);
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
    float timeEnd() const;

    std::vector<unsigned int> modeACodes() const;

protected:

    std::multimap<float, unsigned int> ref_data_; // tod -> index
    std::vector<unsigned int> ref_indexes_;

    std::multimap<float, unsigned int> tst_data_; // tod -> index
    std::vector<unsigned int> tst_indexes_;

    std::shared_ptr<Buffer> ref_buffer;
    std::shared_ptr<Buffer> tst_buffer;

    std::vector<unsigned int> mode_a_codes_;

    void updateModeACodes();
};

#endif // EVALUATIONTARGETDATA_H
