#ifndef ASSOCIATIONTARGET_H
#define ASSOCIATIONTARGET_H

#include <vector>

namespace Association
{
    using namespace std;

    class TargetReport;

    class Target
    {
    public:
        Target(unsigned int utn);

        unsigned int utn_{0};

        vector<TargetReport*> assoc_trs_;
        vector<TargetReport*> potential_trs_;
    };

}

#endif // ASSOCIATIONTARGET_H
