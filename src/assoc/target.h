#ifndef ASSOCIATIONTARGET_H
#define ASSOCIATIONTARGET_H

#include "evaluationtargetposition.h"

#include <vector>
#include <string>
#include <map>
#include <set>

namespace Association
{
    using namespace std;

    class TargetReport;

    class Target
    {
    public:
        Target(unsigned int utn, bool tmp);
        ~Target();

        static bool in_appimage_;
        static double max_time_diff_;
        static double max_altitude_diff_;

        unsigned int utn_{0};
        bool tmp_ {false};

        //bool has_ta_ {false};
        std::set<unsigned int> tas_;

        bool has_tod_ {false};
        float tod_min_ {0};
        float tod_max_ {0};

        vector<TargetReport*> assoc_trs_;
        std::map<float, unsigned int> timed_indexes_;
        std::set <unsigned int> ds_ids_;
        std::set <std::pair<unsigned int, unsigned int>> track_nums_; // ds_it, tn

        void addAssociated (TargetReport* tr);
        void addAssociated (vector<TargetReport*> trs);

        bool hasTA () const;
        bool hasTA (unsigned int ta)  const;
        bool hasAllOfTAs (std::set<unsigned int> tas) const;
        bool hasAnyOfTAs (std::set<unsigned int> tas) const;

        std::string asStr();

        bool hasDataForTime (float tod, float d_max) const;
        std::pair<float, float> timesFor (float tod, float d_max) const; // lower/upper times, -1 if not existing
        std::pair<EvaluationTargetPosition, bool> interpolatedPosForTime (float tod, float d_max) const;

        bool hasDataForExactTime (float tod) const;
        TargetReport& dataForExactTime (float tod) const;
        EvaluationTargetPosition posForExactTime (float tod) const;

        float duration () const;
        bool timeOverlaps (Target& other) const;
        float probTimeOverlaps (Target& other) const; // ratio of overlap, measured by shortest target
        std::tuple<std::vector<float>, std::vector<float>, std::vector<float>> compareModeACodes (Target& other) const;
        std::tuple<std::vector<float>, std::vector<float>, std::vector<float>> compareModeCCodes (
                Target& other, const std::vector<float>& timestamps) const;
        // unknown, same, different timestamps from this
    };

}

#endif // ASSOCIATIONTARGET_H
