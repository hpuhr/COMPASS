#ifndef CALCULATEGEOMETRYJOB_H
#define CALCULATEGEOMETRYJOB_H

#include "job.h"
#include "buffer.h"

class DBObject;
class GeometryObject;

class CalculateGeometryJob : public Job
{
public:
    CalculateGeometryJob(DBObject &object, std::shared_ptr<Buffer> buffer, GeometryObject &geometry_object);

    virtual void run ();

    GeometryObject &geometryObject() { return geometry_object_; }

protected:
    DBObject &object_;
    std::shared_ptr<Buffer> buffer_;
    GeometryObject &geometry_object_;
};

#endif // CALCULATEGEOMETRYJOB_H
