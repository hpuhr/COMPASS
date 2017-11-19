#include "calculategeometryjob.h"

#include "customgeometry.h"
#include "dbobject.h"

CalculateGeometryJob::CalculateGeometryJob(DBObject &object, std::shared_ptr<Buffer> buffer, GeometryObject &geometry_object)
    : Job("CalculateGeometryJob"+object.name()+std::to_string(buffer->size())), object_(object), buffer_(buffer),
      geometry_object_(geometry_object)
{

}

void CalculateGeometryJob::run ()
{
    logdbg << "CalculateGeometryJob: execute: start " << name_;
    started_ = true;

    geometry_object_.processBuffer(object_, buffer_);

    done_=true;

    logdbg << "CalculateGeometryJob: execute: done " << name_;
    return;
}
