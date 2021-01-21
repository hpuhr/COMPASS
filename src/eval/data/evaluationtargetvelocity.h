#ifndef EVALUATIONTARGETVELOCITY_H
#define EVALUATIONTARGETVELOCITY_H


class EvaluationTargetVelocity
{
public:
    EvaluationTargetVelocity() {}

    double x_ {0}; // m
    double y_ {0}; // m
    double track_angle_ {0}; // math, rad
    double speed_ {0}; // m/s
};

#endif // EVALUATIONTARGETVELOCITY_H
