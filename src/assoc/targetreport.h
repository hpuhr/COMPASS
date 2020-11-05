#ifndef ASSOCIATIONTARGETREPORT_H
#define ASSOCIATIONTARGETREPORT_H

#include <string>
#include <vector>

namespace Association
{
    using namespace std;

    class Target;

    class TargetReport
    {
    public:
        TargetReport(string dbo_name, unsigned int rec_num, unsigned int ds_id, float tod);

        string dbo_name_;
        unsigned int rec_num_{0};
        unsigned int ds_id_{0};
        float tod_{0};

        // mode s
        bool has_ta_{false};
        unsigned int ta_{0};

        bool has_ti_{false};
        string ti_;

        // mode 3/a
        bool has_ma_{false};
        unsigned int ma_{0};

        bool has_ma_v_{false};
        bool ma_v_{false};

        bool has_ma_g_{false};
        bool ma_g_{false};

        // mode c
        bool has_mc_{false};
        float mc_{0};

        bool has_mc_v_{false};
        bool mc_v_{false};

        vector<Target*> assoc_targets_;
        vector<Target*> potential_targets_;
    };

}

#endif // ASSOCIATIONTARGETREPORT_H
