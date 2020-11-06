#ifndef ASSOCIATIONTARGET_H
#define ASSOCIATIONTARGET_H

#include <vector>
#include <string>

namespace Association
{
    using namespace std;

    class TargetReport;

    class Target
    {
    public:
        Target(unsigned int utn, bool tmp);
        ~Target();

        unsigned int utn_{0};
        bool tmp_ {false};

        bool has_ta_ {false};
        unsigned int ta_ {0};

        vector<TargetReport*> assoc_trs_;

        void addAssociated (TargetReport* tr);
        void addAssociated (vector<TargetReport*> trs);

        std::string asStr();
    };

}

#endif // ASSOCIATIONTARGET_H
