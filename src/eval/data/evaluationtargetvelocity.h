#ifndef EVALUATIONTARGETVELOCITY_H
#define EVALUATIONTARGETVELOCITY_H


class EvaluationTargetVelocity
{
public:
    EvaluationTargetVelocity() {}

    double track_angle_ {0}; // true north, deg
    double speed_ {0}; // m/s
};

#endif // EVALUATIONTARGETVELOCITY_H
