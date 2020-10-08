#ifndef EVALUATIONTARGETPOSITION_H
#define EVALUATIONTARGETPOSITION_H

class EvaluationTargetPosition
{
public:
    EvaluationTargetPosition() {}
    EvaluationTargetPosition(double latitude, double longitude, bool has_altitude, float altitude)
        : latitude_(latitude), longitude_(longitude), has_altitude_(has_altitude), altitude_(altitude)
    {}

    double latitude_ {0};
    double longitude_ {0};
    bool has_altitude_ {false};
    float altitude_ {0};
};

#endif // EVALUATIONTARGETPOSITION_H
