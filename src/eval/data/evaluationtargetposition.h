#ifndef EVALUATIONTARGETPOSITION_H
#define EVALUATIONTARGETPOSITION_H

class EvaluationTargetPosition
{
public:
    EvaluationTargetPosition() {}
    EvaluationTargetPosition(float latitude, float longitude, bool has_altitude, float altitude)
        : latitude_(latitude), longitude_(longitude), has_altitude_(has_altitude), altitude_(altitude)
    {}

    float latitude_ {0};
    float longitude_ {0};
    bool has_altitude_ {false};
    float altitude_ {0};
};

#endif // EVALUATIONTARGETPOSITION_H
