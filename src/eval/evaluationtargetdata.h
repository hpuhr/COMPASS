#ifndef EVALUATIONTARGETDATA_H
#define EVALUATIONTARGETDATA_H

#include <map>
#include <memory>

class Buffer;

class EvaluationTargetData
{
public:
    EvaluationTargetData(unsigned int utn);

    bool hasRefBuffer ();
    void setRefBuffer (std::shared_ptr<Buffer> buffer);
    void addRefRecNum (float tod, unsigned int rec_num);

    bool hasTstBuffer ();
    void setTstBuffer (std::shared_ptr<Buffer> buffer);
    void addTstRecNum (float tod, unsigned int rec_num);

    bool hasRefData ();
    bool hasTstData ();

    void finalize ();

protected:
    unsigned int utn_{0};

    std::multimap<float, unsigned int> ref_rec_nums_;
    std::multimap<float, unsigned int> tst_rec_nums_;

    std::shared_ptr<Buffer> ref_buffer;
    std::shared_ptr<Buffer> tst_buffer;
};

#endif // EVALUATIONTARGETDATA_H
