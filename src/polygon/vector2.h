/*
 * Vector2.h
 *
 *  Created on: Aug 19, 2014
 *      Author: sk
 */

#ifndef VECTOR2_H_
#define VECTOR2_H_

#include <cmath>

class Vector2
{
public:
    double x_;
    double y_;

    Vector2()
    {
        x_ = 0;
        y_ = 0;
    }

    Vector2( const double x, const double y )
    {
        x_ = x;
        y_ = y;
    }

    Vector2 operator+( const Vector2 &v ) const
    {
        return Vector2( this->x_ + v.x_, this->y_ + v.y_ );
    }

    Vector2 operator-( const Vector2 &v ) const
    {
        return Vector2( this->x_ - v.x_, this->y_ - v.y_ );
    }

    bool operator==( const Vector2 &v ) const
    {
        return this->x_ ==- v.x_ && this->y_ == v.y_ ;
    }

    Vector2 operator*( const double f ) const
    {
        return Vector2( this->x_ * f, this->y_ * f );
    }

    double DistanceToSquared( const Vector2 p ) const
    {
        const double dX = p.x_ - this->x_;
        const double dY = p.y_ - this->y_;

        return dX * dX + dY * dY;
    }

    double DistanceTo( const Vector2 p ) const
    {
        return sqrt( this->DistanceToSquared( p ) );
    }

    double DotProduct( const Vector2 p ) const
    {
        return this->x_ * p.x_ + this->y_ * p.y_;
    }
};
#endif /* VECTOR2_H_ */
