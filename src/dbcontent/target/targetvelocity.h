#ifndef DBCONTENTTARGETVELOCITY_H
#define DBCONTENTTARGETVELOCITY_H

namespace dbContent {

class TargetVelocity
{
public:
    TargetVelocity() {}

    double track_angle_ {0}; // true north, deg
    double speed_ {0}; // m/s
};

}

#endif // DBCONTENTTARGETVELOCITY_H
