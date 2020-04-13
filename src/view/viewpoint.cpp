#include "viewpoint.h"

#include "viewmanager.h"

ViewPoint::ViewPoint(unsigned int id, ViewManager& view_manager)
    : id_(id), view_manager_(view_manager)
{
}

unsigned int ViewPoint::id() const { return id_; }

nlohmann::json& ViewPoint::data() { return data_; }
